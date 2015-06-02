/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include "mc/classic/sdd.hh"
#include "support/pn/net.hh"
#include "support/properties/formulae.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

void
reachability(const order&, const properties::formulae&, results&);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
