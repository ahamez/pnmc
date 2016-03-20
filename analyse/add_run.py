#! /usr/bin/env python3.4

########################################################################################
import argparse
import datetime
import json
import os
import os.path
import sys

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

from data import *

########################################################################################
def main(conf):

  engine = create_engine('sqlite:///' + conf.database, echo=False)
  Base.metadata.create_all(engine)
  Session = sessionmaker(bind=engine)
  session = Session()

  # First, create the run
  run = Run()
  with open(os.path.join(conf.directory, 'config.json'), 'r', encoding='utf8') as f:

    config = json.load(f)

    start = datetime.datetime.fromtimestamp(int(config['epoch start']))
    if session.query(Run).filter_by(start=start).first():
      print('Run started at', start, 'already exists')
      return

    run.name            = config['name']
    run.start           = start
    run.end             = datetime.datetime.fromtimestamp(int(config['epoch end']))
    run.host            = config['host']
    run.pnmc_version    = config['pnmc --version']
    run.time_limit      = config['time limit']

    # Read comments, if any
    try:
      with open(os.path.join(conf.directory, 'comments'), 'r', encoding='utf8') as comments:
        run.comments = comments.read()
    except OSError:
      # no comments file to read
      pass

    # Add options
    for opt in config['options']:
      instance = session.query(Option).filter_by(option=opt).first()
      if not instance:
        # Create option if it doesn't already exist
        instance = Option(option=opt)
      run.options.append(instance)

  # Load result file one by one
  data_dir = os.path.join(conf.directory, 'data')
  for dirname in os.listdir(data_dir):

    print(dirname)

    if not os.path.isdir(os.path.join(data_dir, dirname)):
      continue


    stats_file = os.path.join(data_dir, dirname, 'stats.json')
    results_file = os.path.join(data_dir, dirname, 'results.json')

    if not os.path.exists(stats_file):
      # Model failed
      continue

    r_file = None
    if os.path.exists(results_file):
      r_file = open(results_file, 'r', encoding='utf8')
    
    
    with open(stats_file, 'r', encoding='utf8') as s_file:

      try:
        stats = json.load(s_file)
        results = None
        if r_file:
          results = json.load(r_file)
      except:
        print("Cannot parse ", stats_file)
        continue

      mrun = ModelRun()
      mrun.interrupted          = stats['pnmc']['interrupted']
      if results:
        mrun.states             = float(results['pnmc']['states'])
      else:
        mrun.states             = 0
      mrun.relation_time        = stats['pnmc']['relation time']
      mrun.rewrite_time         = stats['pnmc']['rewrite time']
      mrun.state_space_time     = stats['pnmc']['state space time']
      if mrun.space_space_time == 0:
        mrun.space_space_time = run.time_limit

      if 'FORCE time' in stats['pnmc']:
        mrun.force_time         = stats['pnmc']['FORCE time']
      if 'dead states relation time' in stats['pnmc']:
        mrun.dead_relation_time = stats['pnmc']['dead states relation time']
      if 'dead states rewrite time' in stats['pnmc']:
        mrun.dead_rewrite_time  = stats['pnmc']['dead states rewrite time']
      if 'dead states time' in stats['pnmc']:
        mrun.dead_time          = stats['pnmc']['dead states time']
      
      if results:
        mrun.final_bytes          = stats['pnmc']['state space']['bytes']
        mrun.final_flat_nodes     = stats['pnmc']['state space']['flat nodes']
        mrun.final_hier_nodes     = stats['pnmc']['state space']['hierarchical nodes']
        mrun.final_flat_arcs      = stats['pnmc']['state space']['flat arcs']
        mrun.final_hier_arcs      = stats['pnmc']['state space']['hierarchical arcs']

        # SDD unique table
        sdd_ut = SDDUniqueTable()
        sdd_ut.nb          = stats['pnmc']['libsdd']['SDD unique table']['#']
        sdd_ut.nb          = stats['pnmc']['libsdd']['SDD unique table']['# peak']
        sdd_ut.peak        = stats['pnmc']['libsdd']['SDD unique table']['# accesses']
        sdd_ut.accesses    = stats['pnmc']['libsdd']['SDD unique table']['# hits']
        sdd_ut.misses      = stats['pnmc']['libsdd']['SDD unique table']['# misses']
        sdd_ut.load_factor = stats['pnmc']['libsdd']['SDD unique table']['load factor']
        mrun.sdd_ut = sdd_ut
        
        # Hom unique table
        hom_ut = HomUniqueTable()
        hom_ut.nb          = stats['pnmc']['libsdd']['hom unique table']['#']
        hom_ut.nb          = stats['pnmc']['libsdd']['hom unique table']['# peak']
        hom_ut.peak        = stats['pnmc']['libsdd']['hom unique table']['# accesses']
        hom_ut.accesses    = stats['pnmc']['libsdd']['hom unique table']['# hits']
        hom_ut.misses      = stats['pnmc']['libsdd']['hom unique table']['# misses']
        hom_ut.load_factor = stats['pnmc']['libsdd']['hom unique table']['load factor']
        mrun.hom_ut = hom_ut
        
        # SDD difference cache
        sdd_diff_cache = SddDiffCache()
        sdd_diff_cache.hits      = stats['pnmc']['libsdd']['SDD differences cache']['# hits']
        sdd_diff_cache.misses    = stats['pnmc']['libsdd']['SDD differences cache']['# misses']
        sdd_diff_cache.filtered  = stats['pnmc']['libsdd']['SDD differences cache']['# filtered']
        sdd_diff_cache.discarded = stats['pnmc']['libsdd']['SDD differences cache']['# discarded']
        mrun.sdd_diff_cache = sdd_diff_cache

        # SDD intersection cache
        sdd_inter_cache = SddInterCache()
        sdd_inter_cache.hits      = stats['pnmc']['libsdd']['SDD intersections cache']['# hits']
        sdd_inter_cache.misses    = stats['pnmc']['libsdd']['SDD intersections cache']['# misses']
        sdd_inter_cache.filtered  = stats['pnmc']['libsdd']['SDD intersections cache']['# filtered']
        sdd_inter_cache.discarded = stats['pnmc']['libsdd']['SDD intersections cache']['# discarded']
        mrun.sdd_inter_cache = sdd_inter_cache
        
        # SDD sum cache
        sdd_sum_cache = SddSumCache()
        sdd_sum_cache.hits      = stats['pnmc']['libsdd']['SDD sums cache']['# hits']
        sdd_sum_cache.misses    = stats['pnmc']['libsdd']['SDD sums cache']['# misses']
        sdd_sum_cache.filtered  = stats['pnmc']['libsdd']['SDD sums cache']['# filtered']
        sdd_sum_cache.discarded = stats['pnmc']['libsdd']['SDD sums cache']['# discarded']
        mrun.sdd_sum_cache = sdd_sum_cache
        
        # Hom cache
        hom_cache = HomCache()
        hom_cache.hits      = stats['pnmc']['libsdd']['hom cache']['# hits']
        hom_cache.misses    = stats['pnmc']['libsdd']['hom cache']['# misses']
        hom_cache.filtered  = stats['pnmc']['libsdd']['hom cache']['# filtered']
        hom_cache.discarded = stats['pnmc']['libsdd']['hom cache']['# discarded']
        mrun.hom_cache = hom_cache

      # Link to the Run
      run.modelruns.append(mrun)
      
      # Link to the model
      # modelname, ext =  os.path.splitext(dirname) # Use the name of the JSON file
      modelname = dirname
      model = session.query(Model).filter_by(name=modelname).first()
      if not model:
        print("Model " + modelname + " is not in the database")
        continue
      model.modelruns.append(mrun)

  session.add(run)
  session.commit()

########################################################################################
if __name__ == "__main__":

  parser = argparse.ArgumentParser()
  parser.add_argument('database')
  parser.add_argument('directory')
  conf, _ = parser.parse_known_args()
  conf.database = os.path.abspath(conf.database)
  conf.directory = os.path.abspath(conf.directory)
  main(conf)
  sys.exit(0)
