#! /usr/bin/env python3.4

########################################################################################
import argparse
import collections
import csv
import json
import locale
import os
import sys

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

from data import *

########################################################################################
class Result(object):

  def __init__(self, model, size):
    super(Result, self).__init__()
    self.model = model
    self.mruns = [None] * size
    self.podium = None

  def row(self):
    r = [self.model]
    for mrun in self.mruns:
      if mrun:
        r.append(locale.format('%.2f', mrun.state_space_time))
      else:
         r.append(None)
    r.extend(self.podium)
    return r

########################################################################################
def main(conf):

  locale.setlocale(locale.LC_ALL, 'fr_FR.UTF-8')

  engine = create_engine('sqlite:///' + conf.database, echo=False)
  Base.metadata.create_all(engine)
  Session = sessionmaker(bind=engine)
  session = Session()

  runs = []
  for r in conf.runs:
    query = session.query(Run).filter_by(id=r).first()
    if not query:
      print('Run', r, 'not found')
      sys.exit(1)
    runs.append(query)

  nb_runs = len(runs)

  print('Comparing', ' and '.join(map(lambda r : "'" + r.name + "'", runs)))

  if not all(r.time_limit == runs[0].time_limit for r in runs):
    print('Warning, some runs have different time limits')
    for r in runs:
      print(r.name, '->', r.time_limit)

  # Map runs id to an id in a Result
  runid_to_id = {}
  id_to_run = {}
  counter = 0
  for run in runs:
    runid_to_id[run.id] = counter
    id_to_run[counter] = run
    counter += 1

  # Keep the the results for each model
  result = {}

  # Query times for all models
  for run in runs:
    for mrun in run.modelruns:
      model = session.query(Model).filter_by(id=mrun.model_id).first()
      if not model.name in result:
        result[model.name] = Result(model.name, nb_runs)
      r = result[model.name]
      r.mruns[runid_to_id[run.id]] = mrun

  for model, res in result.items():

    # Find first available number of states
    nb_states = None
    for mrun in res.mruns:
      if mrun and not mrun.interrupted:
        nb_states = mrun.states
        break

    # Check if the numbers of states are equal
    if nb_states:
      for mrun in res.mruns:
        if mrun and not mrun.interrupted and not mrun.states == nb_states:
          print('Warning, runs have a different number of states for model', model)
          break

    times = []
    for i, mrun in enumerate(res.mruns):
      if mrun and not mrun.interrupted:
        times.append((mrun.state_space_time, id_to_run[i]))
    times = sorted(times, key=lambda x: x[0])
    res.podium = [run.name for _, run in times]

  # Write CSV file
  with open(conf.csv, 'w') as f:
    writer = csv.writer(f, delimiter=';')
    writer.writerow(['model'] + list(map(lambda run: run.name, runs)) + list(range(1, len(runs)+1)))
    for res in result.values():
      # Print only significants rows (>1s for all runs)
      # if all((True if mrun and mrun.state_space_time > 1 else False for mrun in res.mruns)):
      writer.writerow(res.row())

########################################################################################
if __name__ == "__main__":

  parser = argparse.ArgumentParser()
  parser.add_argument('database')
  parser.add_argument('run', nargs='+')
  parser.add_argument('csv')
  conf, _ = parser.parse_known_args()
  conf.database = os.path.abspath(conf.database)
  conf.runs = list(collections.OrderedDict.fromkeys(map(lambda r : int(r), conf.run)))
  main(conf)
  sys.exit(0)
