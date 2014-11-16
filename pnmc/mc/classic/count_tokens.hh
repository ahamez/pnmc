#pragma once

#include "mc/classic/sdd.hh"
#include "mc/shared/results.hh"
#include "support/pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

void
count_tokens(shared::results&, const SDD& state_space, const pn::net& net);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
