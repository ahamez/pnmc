#ifndef _PNMC_MC_WORK_HH_
#define _PNMC_MC_WORK_HH_

#include "conf/configuration.hh"
#include "pn/net.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

void
work(const conf::pnmc_configuration&, const pn::net&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

#endif // _PNMC_MC_WORK_HH_
