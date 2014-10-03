#pragma once

#include <sdd/dd/definition.hh>
#include <sdd/conf/default_configurations.hh>

#include "mc/classic/results.hh"
#include "pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

void
count_tokens(results&, const sdd::SDD<sdd::conf1>& state_space, const pn::net& net);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
