#pragma once

#include <set>

#include <boost/dynamic_bitset.hpp>

#include "conf/configuration.hh"
#include "mc/classic/sdd.hh"
#include "support/pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

std::set<homomorphism>
firing_rule( const conf::configuration& conf, const order& o, const pn::net& net
           , boost::dynamic_bitset<>& transitions_bitset, const bool& stop);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
