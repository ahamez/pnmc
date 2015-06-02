/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <map>

#include <boost/dynamic_bitset.hpp>

#include "conf/configuration.hh"
#include "mc/classic/sdd.hh"
#include "support/pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

std::multimap<homomorphism, std::string>
firing_rule( const conf::configuration& conf, const order& o, const pn::net& net
           , boost::dynamic_bitset<>& transitions_bitset, const bool& stop);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
