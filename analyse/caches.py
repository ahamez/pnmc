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

  max_diff_hits = 0
  max_inter_hits = 0
  max_sum_hits = 0
  data_dir = os.path.join(conf.directory, 'data')
  for filename in os.listdir(data_dir):
    with open(os.path.join(data_dir, filename), 'r', encoding='utf8') as f:
      try:
        result = json.load(f)
        diff_cache = stats['libsdd']['SDD differences cache']
        max_diff_hits = max(max_diff_hits, diff_cache['# hits'])
        inter_cache = stats['libsdd']['SDD intersections cache']
        max_inter_hits = max(max_inter_hits, inter_cache['# hits'])
        sum_cache = stats['libsdd']['SDD sums cache']
        max_sum_hits = max(max_sum_hits, sum_cache['# hits'])
      except:
        print(filename, 'is not a json file')
  print("max_diff_hits", max_diff_hits)
  print("max_inter_hits", max_inter_hits)
  print("max_sum_hits", max_sum_hits)

########################################################################################
if __name__ == "__main__":

  parser = argparse.ArgumentParser()
  parser.add_argument('directory')
  # parser.add_argument('csv')
  conf, _ = parser.parse_known_args()
  conf.directory = os.path.abspath(conf.directory)
  main(conf)
  sys.exit(0)
