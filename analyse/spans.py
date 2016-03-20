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
    writer.writerow(['model'] + list(range(1, 102)))
    data_dir = os.path.join(conf.directory, 'data')
    for filename in os.listdir(data_dir):
      with open(os.path.join(data_dir, filename), 'r', encoding='utf8') as f:
        result = json.load(f)
        writer.writerow([filename] + stats['pnmc']['FORCE spans'])

  with open(conf.csv_diff, 'w') as f:
    writer = csv.writer(f, delimiter=';')
    writer.writerow(['model'] + list(range(1, 102)))
    data_dir = os.path.join(conf.directory, 'data')
    for filename in os.listdir(data_dir):
      with open(os.path.join(data_dir, filename), 'r', encoding='utf8') as f:
        result = json.load(f)
        spans = stats['pnmc']['FORCE spans']
        writer.writerow([filename] + [x - spans[0] for x in spans])

  with open(conf.csv_diff_cumul, 'w') as f:
    writer = csv.writer(f, delimiter=';')
    writer.writerow(['model'] + list(range(1, 102)))
    data_dir = os.path.join(conf.directory, 'data')
    for filename in os.listdir(data_dir):
      with open(os.path.join(data_dir, filename), 'r', encoding='utf8') as f:
        result = json.load(f)
        spans = stats['pnmc']['FORCE spans']
        diffs = []
        for i in range(len(spans) - 1):
          diffs.append(spans[i+1] - spans[i])
        writer.writerow([filename] + diffs)



########################################################################################
if __name__ == "__main__":

  parser = argparse.ArgumentParser()
  parser.add_argument('directory')
  parser.add_argument('csv')
  parser.add_argument('csv_diff')
  parser.add_argument('csv_diff_cumul')
  conf, _ = parser.parse_known_args()
  conf.directory = os.path.abspath(conf.directory)
  main(conf)
  sys.exit(0)
