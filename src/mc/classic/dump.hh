#pragma once

#include <sdd/manager.hh>
#include <sdd/conf/default_configurations.hh>
#include <sdd/dd/definition.hh>
#include <sdd/order/order.hh>
#include <sdd/order/strategies/force_hypergraph.hh>

#include "conf/configuration.hh"
#include "mc/classic/results.hh"
#include "mc/classic/statistics.hh"
#include "pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

/// @brief Export SDD to the DOT format when required by the configuration.
void
dump_sdd_dot( const conf::configuration&, const sdd::SDD<sdd::conf1>&
            , const sdd::order<sdd::conf1>&);

/// @brief Export statistics to a JSON file when required by the configuration.
void
dump_json( const conf::configuration&, const statistics&, const sdd::manager<sdd::conf1>&
         , const sdd::SDD<sdd::conf1>&, const pn::net&);

/// @brief Export results to a JSON file when required by the configuration.
void
dump_results( const conf::configuration&, const results&);

/// @brief Export the FORCE's hypergraph to the DOT format when required by the configuration.
void
dump_hypergraph_dot(const conf::configuration&, const sdd::force::hypergraph<sdd::conf1>&);

/// @brief Export homomorphisms to the DOT format when required by the configuration.
void
dump_hom_dot( const conf::configuration&, const sdd::homomorphism<sdd::conf1>& classic
            , const sdd::homomorphism<sdd::conf1>& sat);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
