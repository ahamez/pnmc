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

std::deque<std::pair<std::string, SDD>>
path_to(const order& o, const SDD& initial, SDD all, const SDD& targets, const pn::net& net);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
