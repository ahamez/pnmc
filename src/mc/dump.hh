#ifndef _PNMC_MC_DUMP_HH_
#define _PNMC_MC_DUMP_HH_

#include <sdd/conf/default_configurations.hh>
#include <sdd/dd/definition.hh>

#include "conf/configuration.hh"
#include "mc/statistics.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

/// @brief Export SDD to the DOT format when required by the configuration.
void
dump_sdd_dot(const conf::pnmc_configuration&, const sdd::SDD<sdd::conf1>&);

/// @brief Export SDD as a Lua data structure when required by the configuration.
void
dump_lua( const conf::pnmc_configuration&, const sdd::SDD<sdd::conf1>&);

/// @brief Export statistics to a JSON file when required by the configuration.
void
dump_json( const conf::pnmc_configuration&, const statistics&,const sdd::manager<sdd::conf1>&
         , const sdd::SDD<sdd::conf1>&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

#endif // _PNMC_MC_DUMP_HH_
