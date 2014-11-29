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
