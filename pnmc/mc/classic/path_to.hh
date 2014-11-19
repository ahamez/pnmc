#pragma once

#include <deque>
#include <set>
#include <string>

#include "mc/classic/sdd.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

std::deque<SDD>
path_to(const order& , const SDD&, const SDD&, const std::set<homomorphism>&);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
