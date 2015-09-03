/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <iostream>

#include <sdd/tools/dot/force_hypergraph.hh>
#include <sdd/tools/dot/homomorphism.hh>
#include <sdd/tools/dot/sdd.hh>
#include <sdd/tools/js.hh>
#include <sdd/tools/order.hh>
#include <sdd/tools/sdd_statistics.hh>
#include <sdd/tools/serialization.hh>

#include <boost/filesystem/fstream.hpp>

#include <cereal/archives/json.hpp>

#include "conf/configuration.hh"
#include "mc/shared/results.hh"
#include "mc/shared/results_serialize.hh"
#include "mc/shared/statistics.hh"
#include "mc/shared/statistics_serialize.hh"
#include "support/pn/net.hh"
#include "support/pn/statistics.hh"
#include "support/pn/statistics_serialize.hh"
#include "support/util/paths.hh"

namespace pnmc { namespace mc { namespace shared {

/*------------------------------------------------------------------------------------------------*/

template <typename T>
void
exporter_helper(const conf::configuration& conf, const std::string& name, T&& fn)
{
  const auto path = conf.output_dir / boost::filesystem::path{name};
  boost::filesystem::ofstream stream{path};
  if   (stream.is_open()) {fn(stream);}
  else                    {std::cerr << "Can't write to " << path.string() << '\n';}
}

/*------------------------------------------------------------------------------------------------*/

template <typename... Xs>
void
dot_exporter_helper(const conf::configuration& conf, const std::string& name, Xs&&... args)
{
  exporter_helper(conf, name, [&](auto& file){file << sdd::tools::dot(std::forward<Xs>(args)...);});
}

/*------------------------------------------------------------------------------------------------*/

template <typename T>
struct dot_exporter;

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct dot_sdd
{
  const sdd::SDD<C>& sdd;
  const sdd::order<C>& order;
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct dot_exporter<dot_sdd<C>>
{
  const conf::configuration& conf;
  const std::string& name;

  void
  operator()(const dot_sdd<C>& x)
  const
  {
    if (conf.dot_conf.count(dot_export::sdd)) {dot_exporter_helper(conf, name, x.sdd, x.order);}
  }
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct dot_exporter<sdd::homomorphism<C>>
{
  const conf::configuration& conf;
  const std::string& name;

  void
  operator()(const sdd::homomorphism<C>& h)
  const
  {
    if (conf.dot_conf.count(dot_export::hom)) {dot_exporter_helper(conf, name, h);}
  }
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct dot_exporter<sdd::force::hypergraph<C>>
{
  const conf::configuration& conf;
  const std::string& name;

  void
  operator()(const sdd::force::hypergraph<C>& h)
  const
  {
    if (conf.dot_conf.count(dot_export::force)) {dot_exporter_helper(conf, name, h);}
  }
};

/*------------------------------------------------------------------------------------------------*/

inline
void
export_dot(const conf::configuration&)
{}

/*------------------------------------------------------------------------------------------------*/

template <typename X, typename... Xs>
void
export_dot(const conf::configuration& conf, conf::filename name, X&& x, Xs&&... xs)
{
  dot_exporter<std::decay_t<X>>{conf, conf::default_filenames.at(name)}(std::forward<X>(x));
  export_dot(conf, std::forward<Xs>(xs)...);
}

/*------------------------------------------------------------------------------------------------*/

template <typename T>
struct json_exporter;

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct json_exporter<sdd::order<C>>
{
  const conf::configuration& conf;
  const std::string& name;

  void
  operator()(const sdd::order<C>& o)
  const
  {
    if (conf.json_conf.count(json_export::order))
    {
      exporter_helper(conf, name, [&](auto& file) {sdd::tools::dump_order(o, file);});
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct json_exporter<sdd::homomorphism<C>>
{
  const conf::configuration& conf;
  const std::string& name;

  void
  operator()(const sdd::homomorphism<C>& h)
  const
  {
    if (conf.json_conf.count(json_export::hom))
    {
      exporter_helper(conf, name, [&](auto& file) {file << sdd::tools::js(h);});
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct json_exporter<results<C>>
{
  const conf::configuration& conf;
  const std::string& name;

  void
  operator()(const results<C>& r)
  const
  {
    exporter_helper( conf, name
                   , [&](auto& file)
                        {
                          cereal::JSONOutputArchive archive(file);
                          archive(cereal::make_nvp("pnmc", r));
                        });
  }
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct json_exporter<statistics<C>>
{
  const conf::configuration& conf;
  const std::string& name;

  void
  operator()(const statistics<C>& s)
  const
  {
    if (conf.json_conf.count(json_export::stats))
    {
      exporter_helper( conf, name
                     , [&](auto& file)
                          {
                            cereal::JSONOutputArchive archive(file);
                            archive(cereal::make_nvp("pnmc", s));
                          });
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

inline
void
export_json(const conf::configuration&)
{}

/*------------------------------------------------------------------------------------------------*/

template <typename X, typename... Xs>
void
export_json(const conf::configuration& conf, conf::filename name, X&& x, Xs&&... xs)
{
  json_exporter<std::decay_t<X>>{conf, conf::default_filenames.at(name)}(std::forward<X>(x));
  export_json(conf, std::forward<Xs>(xs)...);
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::shared
