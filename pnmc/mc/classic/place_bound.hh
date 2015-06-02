/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <set>
#include <unordered_map>

#include "mc/classic/sdd.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

std::unordered_map<std::string /* place id */, pn::valuation_type>
place_bound(const order&, const SDD&, const std::set<std::string>&);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
