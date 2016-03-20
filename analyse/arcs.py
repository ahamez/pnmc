#! /usr/bin/env python3.3

########################################################################################
import argparse
import csv
import datetime
import json
import os
import os.path
import sys

########################################################################################
def main(conf):

  total_frequency = {}

  data_dir = os.path.join(conf.directory, 'data')
  for filename in os.listdir(data_dir):
    with open(os.path.join(data_dir, filename), 'r', encoding='utf8') as f:
      result = json.load(f)
      for freq in stats['final sdd']['arcs frequency']:
        key = freq['key']
        value = freq['value']['first']
        if key in total_frequency:
          total_frequency[key] += value
        else:
          total_frequency[key] = value

  print(total_frequency)

########################################################################################
if __name__ == "__main__":

  parser = argparse.ArgumentParser()
  parser.add_argument('directory')
  conf, _ = parser.parse_known_args()
  conf.directory = os.path.abspath(conf.directory)
  main(conf)
  sys.exit(0)
