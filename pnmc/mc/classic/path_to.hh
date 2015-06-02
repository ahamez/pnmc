/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <deque>
#include <set>
#include <string>

#include "mc/classic/sdd.hh"
#include "support/pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

std::deque<std::pair<std::string, SDD>>
shortest_path( const order& , const SDD&, const SDD&, const pn::net&
             , const std::multimap<homomorphism, std::string>&);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
