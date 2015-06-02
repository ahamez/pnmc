/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include "conf/configuration.hh"
#include "mc/classic/sdd.hh"
#include "mc/shared/statistics.hh"
#include "support/pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

order
make_order(const conf::configuration&, statistics&, const pn::net&);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
