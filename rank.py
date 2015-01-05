# #! /usr/bin/env python3

import csv
import fileinput
import locale

# Paths to MCC 2014 result files
KNOWN_FILE = '/path/to/known.csv'
SCRAMBLED_FILE = '/path/to/scrambled.csv'
SURPRISE_FILE = '/path/to/surprise.csv'

# Which files to process
FILES = (SURPRISE_FILE)
# FILES = (KNOWN_FILE, SCRAMBLED_FILE, SURPRISE_FILE)

# Tools for which we want results
TOOLS = ['greatspn', 'marcie', 'pnmc', 'pnxdd', 'stratagem', 'tapaal']

# Function to output data to CSV
def output_data(out, kind, data):
  fieldnames = [kind, 1, 2, 3, 4, 5, 6]
  writer = csv.DictWriter(out, fieldnames, delimiter=';')
  writer.writeheader()
  for tool, rank in data.items():
    out_row = {r : v for r, v in enumerate(rank, start=1)}
    out_row[kind] = tool
    writer.writerow(out_row)

with fileinput.input(files=FILES) as file, open('/Users/hal/Desktop/out.csv', 'w') as output:

  # Generate columuns for a given tool
  subfieldnames = ['result', 'techniques', 'mem', 'cpu', 'time', 'i/o', 'status', 'id']
  fn = lambda tool_name : [tool_name + '_{}'.format(i) for i in subfieldnames]

  # All columns found in a result file
  fieldnames = ['model', 'type', 'param'] \
             + fn('greatspn')             \
             + fn('helena')               \
             + fn('marcie')               \
             + fn('pnmc')                 \
             + fn('pnxdd')                \
             + fn('stratagem')            \
             + fn('tapaal')

  reader = csv.DictReader(file, fieldnames)

  data_cpu = {t : [0, 0, 0, 0, 0, 0] for t in TOOLS}
  data_mem = {t : [0, 0, 0, 0, 0, 0] for t in TOOLS}

  for row in reader:

    # Ignore headers
    if row['model'][0] == '#':
      continue

    # If we don't want to process a model
    # if 'PolyORB' in row['model']:
    #   continue
    # if 'Echo' in row['model']:
    #   continue

    # We are not interested in Petri nets othere than place/transition.
    if row['type'] != 'PT':
      continue

    # Helena works only with colored Petri nets
    for key in fn('helena'):
      row.pop(key)

    cpu = []
    mem = []
    for tool in TOOLS:
      result = row[tool + '_result']
      if result != 'DNF' and result != 'MOVF' and result != 'CC':
        cpu.append((tool, float(row[tool + '_cpu']) / 1000))
        mem.append((tool, float(row[tool + '_mem'])))
      else:
        cpu.append((tool, float('inf')))
        mem.append((tool, float('inf')))

    # Test if all tools failed.
    if (all(x == float('inf') for (_, x) in cpu)):
      continue

    # "fair" mode
    # filter models where only one tool succeeded
    tmp = set(x for (_,x) in cpu)
    if float('inf') in tmp:
      tmp.remove(float('inf'))
    if len(tmp) == 1: #only one tool, unfair
      print("Only one tool for ", row['model'])
      continue

    cpu.sort(key=lambda x : x[1])
    for rank, (tool, x) in enumerate(cpu):
      if x != float('inf'):
        data_cpu[tool][rank] += 1

    mem.sort(key=lambda x : x[1])
    for i, (t, x) in enumerate(mem):
      if x != float('inf'):
        data_mem[t][i] += 1

  output_data(output, 'cpu', data_cpu)
  output_data(output, 'mem', data_mem)
