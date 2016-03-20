#! /usr/bin/env python3.4

import argparse
import bz2
import concurrent.futures
import json
import os
import os.path
import shutil
import sqlalchemy
import sqlalchemy.orm
import subprocess
import sys
import tempfile
import time

from data import Base, Model

####################################################################################################
def worker(conf, model):

  with tempfile.NamedTemporaryFile() as f:
    f.write(bz2.decompress(model.pn))
    f.flush()

    params = [conf.pnmc] + conf.pnmc_options
    if conf.time_limit != 0:
      params.append('--time-limit=' + str(conf.time_limit))
    if conf.order_dir:
      order_file = os.path.join(conf.order_dir, model.name)
      json_order_file = os.path.abspath(order_file + '.json')
      if os.path.isfile(json_order_file):
        params.append('--order-load=' + json_order_file)
        print('--order-load=' + json_order_file)
    params.append('--input=' + model.format)
    params.append('--json=stats')
    params.append('--stats=final-sdd')
    params.append('--output-dir=' + os.path.join(conf.outputdir, 'data', model.name))
    params.append(f.name)
    outfile_path = os.path.join(conf.outputdir, 'data', model.name + '.out')
    errfile_path = os.path.join(conf.outputdir, 'data', model.name + '.err')
    with open(outfile_path, 'w') as outfile, open(errfile_path, 'w') as errfile:
      try:
        if conf.time_limit != 0:
          hard_timeout = int(conf.time_limit + 10 + (conf.time_limit)/10)
          subprocess.check_call(params, stdout=outfile, stderr=errfile, timeout=hard_timeout)
        else:
          subprocess.check_call(params, stdout=outfile, stderr=errfile)
        if os.path.getsize(errfile_path) == 0:
          os.remove(errfile_path)
      except subprocess.TimeoutExpired as e:
        print('Error: hard timeout after ', e.timeout, 's', file=errfile)
      except subprocess.CalledProcessError as e:
        print('Error: pnmc returned code ', e.returncode, file=errfile)

########################################################################################
def main(conf):

  engine = sqlalchemy.create_engine('sqlite:///' + conf.database, echo=False)
  Base.metadata.create_all(engine)

  Session = sqlalchemy.orm.sessionmaker(bind=engine)
  session = Session()

  # Get pnmc version
  try:
    pnmc_version = subprocess.check_output([conf.pnmc, '--version']).decode('utf8')
  except subprocess.CalledProcessError as e:
    print('Unable to get pnmc version: ')
    print(e)
    sys.exit(1)

  # Create output directory, if needed
  os.makedirs(os.path.join(conf.outputdir, 'data'), exist_ok=True)

  with tempfile.TemporaryDirectory() as tmpdir:

    # Copy pnmc to make sure we use the same during the whole benchmark
    shutil.copy2(conf.pnmc, tmpdir)
    conf.pnmc = os.path.join(tmpdir, os.path.basename(conf.pnmc))

    # Launch pnmc
    start = time.time()
    with concurrent.futures.ThreadPoolExecutor(max_workers=conf.workers) as executor:
      future_to_model = { executor.submit(lambda m : worker(conf, m), model)
                        : model for model in session.query(Model).all()}
      for future in concurrent.futures.as_completed(future_to_model):
        model = future_to_model[future]
        try:
          data = future.result()
        except Exception as e:
          with open(os.path.join(conf.outputdir, 'errors.txt'), 'w') as errfile:
            print('Problem with model', model.name, file=errfile)
            print(str(e), file=errfile)
    end = time.time()

    # Dump configuration file
    with open(os.path.join(conf.outputdir, 'config.json'), 'w', encoding='utf8') as f:
      config = { "config version": "2"
               , "name" : conf.name
               , "epoch end": int(end)
               , "epoch start": int(start)
               , "host": os.uname().nodename
               , "options": conf.pnmc_options
               , "pnmc --version": pnmc_version
               , "time limit" : int(conf.time_limit)}
      json.dump(config, f, sort_keys=True, indent=2)

    print('Total time', int(end - start), 's')

####################################################################################################
if __name__ == "__main__":

  parser = argparse.ArgumentParser()
  parser.add_argument('database')
  parser.add_argument('name')
  parser.add_argument('pnmc')
  parser.add_argument('outputdir')
  parser.add_argument('--time-limit', action='store', default=600)
  parser.add_argument('--workers', action='store', default=4)
  parser.add_argument('--order-dir', action='store', default=None)
  conf, others = parser.parse_known_args()
  conf.pnmc_options = others
  conf.workers = int(conf.workers)
  conf.pnmc = os.path.abspath(conf.pnmc)
  conf.database = os.path.abspath(conf.database)
  conf.outputdir = os.path.abspath(conf.outputdir)
  conf.time_limit = int(conf.time_limit)
  main(conf)
  sys.exit(0)
