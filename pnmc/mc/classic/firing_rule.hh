#pragma once

#include <boost/dynamic_bitset.hpp>

#include <sdd/sdd.hh>
#include <sdd/conf/default_configurations.hh>
#include <sdd/order/order.hh>

#include "conf/configuration.hh"
#include "mc/shared/statistics.hh"
#include "shared/pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

sdd::homomorphism<sdd::conf1>
firing_rule( const conf::configuration& conf, const sdd::order<sdd::conf1>& o
           , const pn::net& net, boost::dynamic_bitset<>& transitions_bitset
           , shared::statistics& stats, const bool& stop);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
