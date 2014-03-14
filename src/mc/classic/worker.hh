#ifndef _PNMC_MC_CLASSIC_WORKER_HH_
#define _PNMC_MC_CLASSIC_WORKER_HH_

#include "conf/configuration.hh"
#include "mc/mc_impl.hh"
#include "pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct worker
  : public mc_impl
{
  const conf::configuration& conf;

  worker(const conf::configuration& c);

  void
  operator()(const pn::net& net) const;
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

#endif // _PNMC_MC_CLASSIC_WORKER_HH_
