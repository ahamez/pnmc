#include <fstream>

#include <sdd/tools/dot/sdd.hh>
#include <sdd/tools/dot/force_hypergraph.hh>
#include <sdd/tools/lua.hh>
#include <sdd/tools/sdd_statistics.hh>
#include <sdd/tools/serialization.hh>

#include <cereal/archives/json.hpp>

#include "mc/dump.hh"
#include "mc/statistics_serialize.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

void
dump_sdd_dot(const conf::pnmc_configuration& conf, const sdd::SDD<sdd::conf1>& s)
{
  if (conf.export_final_sdd_dot)
  {
    std::ofstream file(conf.export_final_sdd_dot_file);
    if (file.is_open())
    {
      file << sdd::tools::dot(s) << std::endl;
    }
    else
    {
      std::cerr << "Can't export state space to " << conf.export_final_sdd_dot_file << std::endl;
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

void
dump_lua(const conf::pnmc_configuration& conf, const sdd::SDD<sdd::conf1>& s)
{
  if (conf.export_to_lua)
  {
    std::ofstream file(conf.export_to_lua_file);
    if (file.is_open())
    {
      file << sdd::tools::lua(s) << std::endl;
    }
    else
    {
      std::cerr << "Can't export Lua data structure to " << conf.export_to_lua_file << std::endl;
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

void
dump_json( const conf::pnmc_configuration& conf, const statistics& stats
         , const sdd::manager<sdd::conf1>& manager, const sdd::SDD<sdd::conf1>& s)
{
  if (conf.json)
  {
    std::ofstream file(conf.json_file);
    if (file.is_open())
    {
      const sdd::tools::sdd_statistics<sdd::conf1> final_sdd_stats(s);

      cereal::JSONOutputArchive archive(file);
      if (not conf.read_stdin)
      {
        archive(cereal::make_nvp("file", conf.file_name));
      }
      archive(cereal::make_nvp("pnmc", stats));
      archive(cereal::make_nvp("libsdd", manager));
      archive(cereal::make_nvp("final sdd", final_sdd_stats));
    }
    else
    {
      std::cerr << "Can't export statistics to " << conf.json_file << std::endl;
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

void
dump_hypergraph_dot( const conf::pnmc_configuration& conf
                   , const sdd::force::hypergraph<sdd::conf1>& graph)
{
  if (conf.hypergraph_dot)
  {
    std::ofstream file(conf.hypergraph_dot_file);
    if (file.is_open())
    {
      file << sdd::tools::dot(graph) << std::endl;
    }
    else
    {
      std::cerr << "Can't export FORCE hypergraph to " << conf.hypergraph_dot_file << std::endl;
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc
