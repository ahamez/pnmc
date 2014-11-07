#pragma once

#include <boost/dynamic_bitset.hpp>

#include "conf/configuration.hh"
#include "mc/classic/sdd.hh"
#include "mc/shared/statistics.hh"
#include "shared/pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

homomorphism
firing_rule( const conf::configuration& conf, const order& o
           , const pn::net& net, boost::dynamic_bitset<>& transitions_bitset
           , shared::statistics& stats, const bool& stop);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
