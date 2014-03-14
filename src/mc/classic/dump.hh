#ifndef _PNMC_MC_DUMP_HH_
#define _PNMC_MC_DUMP_HH_

#include <sdd/manager.hh>
#include <sdd/conf/default_configurations.hh>
#include <sdd/dd/definition.hh>
#include "sdd/order/strategies/force_hypergraph.hh"

#include "conf/configuration.hh"
#include "mc/classic/statistics.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

/// @brief Export SDD to the DOT format when required by the configuration.
void
dump_sdd_dot(const conf::configuration&, const sdd::SDD<sdd::conf1>&);

/// @brief Export SDD as a Lua data structure when required by the configuration.
void
dump_lua(const conf::configuration&, const sdd::SDD<sdd::conf1>&);

/// @brief Export statistics to a JSON file when required by the configuration.
void
dump_json( const conf::configuration&, const statistics&, const sdd::manager<sdd::conf1>&
         , const sdd::SDD<sdd::conf1>&);

/// @brief Export the FORCE's hypergraph to the DOT format when required by the configuration.
void
dump_hypergraph_dot(const conf::configuration&, const sdd::force::hypergraph<sdd::conf1>&);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

#endif // _PNMC_MC_DUMP_HH_
