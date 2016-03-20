from sqlalchemy import Column, ForeignKey, Table
from sqlalchemy import Boolean, DateTime, Float, Integer, LargeBinary, String, Text
from sqlalchemy.ext.declarative import declarative_base, declared_attr
from sqlalchemy.orm import relationship

########################################################################################

Base = declarative_base()

#######################################################################################
run_options = Table( 'run_options', Base.metadata
                   , Column('run_id', Integer, ForeignKey('runs.id'))
                   , Column('option_id', Integer, ForeignKey('options.id')))

########################################################################################
class Run(Base):

  """A Run is a benchmark campaign for a specific set of options of pnmc."""

  __tablename__ = 'runs'

  id              = Column(Integer, primary_key=True)
  name            = Column(String, nullable=False)
  start           = Column(DateTime, nullable=False)
  end             = Column(DateTime, nullable=False)
  host            = Column(String, nullable=False)
  pnmc_version    = Column(String, nullable=False)
  comments        = Column(Text)
  time_limit      = Column(Float, nullable=False)

  options         = relationship('Option', secondary=run_options, backref='runs')
  modelruns       = relationship("ModelRun", backref='run')

########################################################################################
class Option(Base):
  
  __tablename__ = 'options'
  
  id     = Column(Integer, primary_key=True)
  option = Column(String, nullable=False, unique=True)

########################################################################################
class ModelRun(Base):

  """A run for a Model in a Run."""

  __tablename__ = 'modelruns'

  id                 = Column(Integer, primary_key=True)
  interrupted        = Column(Boolean, nullable=False)
  states             = Column(Float, nullable=False)
  relation_time      = Column(Float, nullable=False)
  rewrite_time       = Column(Float, nullable=False)
  state_space_time   = Column(Float, nullable=False)
  force_time         = Column(Float)
  dead_relation_time = Column(Float)
  dead_rewrite_time  = Column(Float)
  dead_time          = Column(Float)

  final_bytes        = Column(Integer, nullable=True)
  final_flat_nodes   = Column(Integer, nullable=True)
  final_hier_nodes   = Column(Integer, nullable=True)
  final_flat_arcs    = Column(Integer, nullable=True)
  final_hier_arcs    = Column(Integer, nullable=True)

  run_id             = Column(Integer, ForeignKey('runs.id'))
  model_id           = Column(Integer, ForeignKey('models.id'))

  arc_frequency      = relationship("ArcFrequency", backref='model_run')
  # uselist=False : one-to-one relationship
  sdd_ut             = relationship("SDDUniqueTable", uselist=False, backref='model_run')
  hom_ut             = relationship("HomUniqueTable", uselist=False, backref='model_run')
  sdd_diff_cache     = relationship("SddDiffCache", uselist=False, backref='model_run')
  sdd_inter_cache    = relationship("SddInterCache", uselist=False, backref='model_run')
  sdd_sum_cache      = relationship("SddSumCache", uselist=False, backref='model_run')
  hom_cache          = relationship("HomCache", uselist=False, backref='model_run')

########################################################################################
class Model(Base):

  """Describes a Petri net model used in benchmarks."""

  __tablename__ = 'models'

  id        = Column(Integer, primary_key=True)
  name      = Column(String, nullable=False)
  format    = Column(String, nullable=False) # bpn, prod, tina, xml ; use an enumaration?
  comments  = Column(Text)
  pn        = Column(LargeBinary)

  modelruns = relationship("ModelRun", backref='model')

########################################################################################
class UniqueTable(object):

  id           = Column(Integer, primary_key=True)
  nb           = Column(Integer, nullable=True)
  peak         = Column(Integer, nullable=True)
  accesses     = Column(Integer, nullable=True)
  misses       = Column(Integer, nullable=True)
  load_factor  = Column(Float, nullable=True)

  # For inheritance
  @declared_attr
  def model_run_id(cls):
    return Column(Integer, ForeignKey('modelruns.id'))

########################################################################################
class SDDUniqueTable(Base, UniqueTable):

  __tablename__ = 'sdd_unique_tables'

########################################################################################
class HomUniqueTable(Base, UniqueTable):

  __tablename__ = 'hom_unique_tables'

########################################################################################
class ArcFrequency(Base):

  __tablename__ = 'arc_frequencies'

  id           = Column(Integer, primary_key=True)
  arcs         = Column(Integer, nullable=True)
  flat         = Column(Integer, nullable=True)
  hier         = Column(Integer, nullable=True)
  
  model_run_id = Column(Integer, ForeignKey('modelruns.id'))

########################################################################################
class Cache(object):

  id           = Column(Integer, primary_key=True)
  hits         = Column(Integer, nullable=True)
  misses       = Column(Integer, nullable=True)
  filtered     = Column(Integer, nullable=True)
  discarded    = Column(Integer, nullable=True)

  # For inheritance
  @declared_attr
  def model_run_id(cls):
    return Column(Integer, ForeignKey('modelruns.id'))

########################################################################################
class SddDiffCache(Base, Cache):

  __tablename__ = 'sdd_diff_caches'

########################################################################################
class SddInterCache(Base, Cache):

  __tablename__ = 'sdd_inter_caches'

########################################################################################
class SddSumCache(Base, Cache):

  __tablename__ = 'sdd_sum_caches'

########################################################################################
class HomCache(Base, Cache):

  __tablename__ = 'hom_caches'
