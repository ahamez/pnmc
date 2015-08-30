####################################################################################################
#! /usr/bin/env python3.3

####################################################################################################
import argparse
import concurrent.futures
import json
import os
import os.path
import subprocess
import sys

####################################################################################################
BASE_DIR = os.path.dirname(__file__)
MODELS_DIR = os.path.join(BASE_DIR, 'models')

####################################################################################################
class Fail(Exception):

    def __init__(self, nb_states):
        self.nb_states = nb_states

####################################################################################################
def worker(conf, params):

  model_type, model_dir, model, models_conf, full_args = params
  nb_states = models_conf['models'][model]['states']
  model_path = os.path.join(model_dir, model)

  args = full_args.strip().split()
  # create a valid file name containing the arguments of pnmc
  flat_args = "".join(["".join([x if x.isalnum() else "_" for x in a]) for a in args])
  if not flat_args:
    flat_args = "__"

  outputdir = os.path.join(conf.outputdir, os.path.basename(model_dir), model, flat_args)
  # Create output directory for this model's logs, if needed
  os.makedirs(outputdir, exist_ok=True)

  outfile_path = os.path.join(outputdir, "out.txt")
  errfile_path = os.path.join(outputdir, "err.txt")
  json_path = os.path.join(outputdir, "results.json")

  cli = [conf.pnmc, '--format=' + model_type, '--output-dir=' + outputdir]
  if args:
    cli.extend(args)
  cli.append(model_path)

  with open(outfile_path, 'w') as outfile, open(errfile_path, 'w') as errfile:

    # Launch pnmc
    subprocess.check_call(cli, stdout=outfile, stderr=errfile)

    # Check if the number of states is correct
    with open(json_path, 'r') as pnmc_json_file:

      j = json.load(pnmc_json_file)
      run_states = int(j['pnmc']['states'])

      if run_states != nb_states:
        raise Fail(run_states)


####################################################################################################
def main(conf):

  error_encountered = False
  logfile_path = os.path.join(conf.outputdir, "log.txt")

  # First, load all configurations
  models_conf = {}
  for dirname, _, files in os.walk(MODELS_DIR):
    if 'conf.json' in files:
      with open(os.path.join(dirname, 'conf.json'), 'r') as json_file:
        models_conf[dirname] = json.load(json_file)

  # Create output directory for logs, if needed
  os.makedirs(conf.outputdir, exist_ok=True)

  # Launch pnmc
  with concurrent.futures.ThreadPoolExecutor(max_workers=conf.workers) as executor\
     , open(logfile_path, 'w') as logfile:

    futures = {}

    # To format output
    c0, c1, c2 = 0, 0, 0

    # Prepare and launch tasks
    for dirname, _, files in os.walk(MODELS_DIR):
      if 'conf.json' in files:
        local_models_conf = models_conf[dirname]
        model_type = local_models_conf['type']
        for model in local_models_conf['models']:
          if model != 'conf.json' and model[0] != '.':
            arguments = list(local_models_conf['arguments']) # copy arguments
            if 'arguments' in local_models_conf['models'][model]:
              arguments += local_models_conf['models'][model]['arguments']
            for args in arguments:
              c0, c1, c2 = max(c0, len(model_type)), max(c1, len(args)), max(c2, len(model))
              params = (model_type, dirname, model, local_models_conf, args.strip())
              futures[executor.submit(lambda p : worker(conf, p), params)] = params

    # Verify results of tasks
    for future in concurrent.futures.as_completed(futures):

      model_type, _, model, local_conf, args = futures[future]
      line = "| {:{}} | {:{}} | {:{}} | ".format(model_type, c0, args, c1, model, c2)
      try:
        future.result()
      except subprocess.CalledProcessError as c:
        error_encountered = True
        if c.returncode == 64:
          line += "INVALID ARGUMENT(S)"
        else:
          line += "OTHER ERROR: " + str(c)
      except Fail as f:
        error_encountered = True
        line += "FAIL: " + str(f.nb_states) + " states (should be "
        line += str(local_conf['models'][model]['states']) + ")"
      else:
        line += "OK"
      print(line)
      print(line, file=logfile)

  return error_encountered

####################################################################################################
if __name__ == "__main__":

  parser = argparse.ArgumentParser()
  parser.add_argument('pnmc')
  parser.add_argument('outputdir')
  parser.add_argument('--workers', action='store', default=4)
  conf, others = parser.parse_known_args()
  conf.pnmc = os.path.abspath(conf.pnmc)
  conf.outputdir = os.path.abspath(conf.outputdir)
  conf.workers = int(conf.workers)
  if main(conf):
    sys.exit(1)
  else:
    sys.exit(0)
