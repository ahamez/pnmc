#! /usr/bin/env python3.4

########################################################################################
import argparse
import bz2
import os
import sys

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

from data import Base, Model

########################################################################################
def main(conf):
  engine = create_engine('sqlite:///' + conf.database, echo=False)
  Base.metadata.create_all(engine)

  Session = sessionmaker(bind=engine)
  session = Session()

  for dirpath, _, files in os.walk(conf.directory):
    for name in files:
      if name[0] == ".":
        print("Ignoring", os.path.join(dirpath, name))
        continue
      
      nb = session.query(Model).filter_by(name=name, format=conf.format).count()
      if nb == 1:
        print(name, "with format", conf.format, "already exists")
      elif nb > 1:
        print("Error, several instances of the same model", name, " with format", format)
      else:
        m = Model(name=name, format=conf.format)
        with open(os.path.join(dirpath, name), 'r', encoding='utf8') as f:
          m.pn = bz2.compress(bytes(f.read(), 'UTF-8'))
          print('Adding', name)
          session.add(m)


  session.commit()

########################################################################################
if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument('database')
  parser.add_argument('format', choices=['bpn','pnml','tina'])
  parser.add_argument('directory')
  conf, _ = parser.parse_known_args()
  conf.database = os.path.abspath(conf.database)
  conf.directory = os.path.abspath(conf.directory)
  main(conf)
  sys.exit(0)
