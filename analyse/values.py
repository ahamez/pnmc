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

  with open(conf.csv, 'w') as f:
    writer = csv.writer(f, delimiter=';')
    data_dir = os.path.join(conf.directory, 'data')
    for filename in os.listdir(data_dir):
      with open(os.path.join(data_dir, filename), 'r', encoding='utf8') as f:
        result = json.load(f)
        values = result['libsdd']['values']
        writer.writerow( [filename, values['#'], values['# peak'], values['# accesses']
                       , values['# hits'], values['# misses'], values['# rehash']
                       , values['load factor']])

########################################################################################
if __name__ == "__main__":

  parser = argparse.ArgumentParser()
  parser.add_argument('directory')
  parser.add_argument('csv')
  conf, _ = parser.parse_known_args()
  conf.directory = os.path.abspath(conf.directory)
  main(conf)
  sys.exit(0)

