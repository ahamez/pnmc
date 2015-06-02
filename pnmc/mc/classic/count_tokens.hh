/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include "mc/classic/sdd.hh"
#include "mc/shared/results.hh"
#include "support/pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

void
count_tokens(results&, const SDD& state_space, const pn::net& net);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
