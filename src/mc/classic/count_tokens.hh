#ifndef _PNMC_MC_CLASSIC_COUNT_TOKENS_HH_
#define _PNMC_MC_CLASSIC_COUNT_TOKENS_HH_

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

#endif // _PNMC_MC_CLASSIC_COUNT_TOKENS_HH_
