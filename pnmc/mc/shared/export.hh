#pragma once

#include <sdd/tools/dot/force_hypergraph.hh>
#include <sdd/tools/dot/homomorphism.hh>
#include <sdd/tools/dot/sdd.hh>
#include <sdd/tools/js.hh>
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

template <typename... Xs>
void
dot_exporter_helper(const conf::configuration& conf, const std::string& name, Xs&&... args)
{
  const auto path = conf.output_dir / boost::filesystem::path(name + ".dot");
  boost::filesystem::ofstream file(path);
  if   (file.is_open()) {file << sdd::tools::dot(std::forward<Xs>(args)...);}
  else                  {std::cerr << "Can't write to " << path.string() << '\n';}
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
    if (conf.dot_dump.count(dot_export::sdd)) {dot_exporter_helper(conf, name, x.sdd, x.order);}
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
    if (conf.dot_dump.count(dot_export::hom)) {dot_exporter_helper(conf, name, h);}
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
    if (conf.dot_dump.count(dot_export::force)) {dot_exporter_helper(conf, name, h);}
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
export_dot(const conf::configuration& conf, std::string name, X&& x, Xs&&... xs)
{
  dot_exporter<std::decay_t<X>>{conf, name}(std::forward<X>(x));
  export_dot(conf, std::forward<Xs>(xs)...);
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Export statistics to a JSON file when required by the configuration.
template <typename Manager, typename SDD>
void
dump_json( const conf::configuration& conf, const statistics& stats, const Manager& manager
         , const SDD& s, const pn::net& net)
{
  if (conf.json_file)
  {
    boost::filesystem::ofstream file(*conf.json_file);
    if (file.is_open())
    {
      cereal::JSONOutputArchive archive(file);
      if (not conf.input.file)
      {
        archive(cereal::make_nvp("file", "stdin"));
      }
      else
      {
        archive(cereal::make_nvp("file", conf.input.file->string()));
      }
      archive(cereal::make_nvp("pnmc", stats), cereal::make_nvp("libsdd", manager));
      if (conf.final_sdd_statistics)
      {
        archive(cereal::make_nvp("final sdd", sdd::tools::statistics(s)));
      }
      if (conf.pn_statistics)
      {
        archive(cereal::make_nvp("pn", pn::statistics(net)));
      }
    }
    else
    {
      std::cerr << "Can't export statistics to " << *conf.json_file << std::endl;
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Export results to a JSON file when required by the configuration.
inline
void
dump_results(const conf::configuration& conf, const results& res)
{
  if (conf.results_json_file)
  {
    boost::filesystem::ofstream file(*conf.results_json_file);
    if (file.is_open())
    {
      cereal::JSONOutputArchive archive(file);
      archive(cereal::make_nvp("pnmc", res));
    }
    else
    {
      std::cerr << "Can't export results to " << *conf.results_json_file << std::endl;
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

///// @brief Export homomorphisms to the DOT format when required by the configuration.
//template <typename Homomorphism>
//void
//dump_hom(const conf::configuration& conf, const Homomorphism& classic, const Homomorphism& sat)
//{
//  if (conf.hom_json_file)
//  {
//    boost::filesystem::ofstream file(*conf.hom_json_file);
//    if (file.is_open())
//    {
//      file << sdd::tools::js(classic) << std::endl;
//    }
//    else
//    {
//      std::cerr << "Can't export homomorphism to " << *conf.hom_json_file << std::endl;
//    }
//  }
//}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::shared
