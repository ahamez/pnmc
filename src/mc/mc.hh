#ifndef _PNMC_MC_MC_HH_
#define _PNMC_MC_MC_HH_

#include <memory>

#include "conf/configuration.hh"
#include "mc/mc_impl.hh"
#include "pn/net.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

class mc
{
private:

  const std::unique_ptr<mc_impl> impl_;

public:

  mc(const conf::configuration&);

  void
  operator()(const pn::net&)
  const;

private:

  static
  std::unique_ptr<mc_impl>
  mk_impl(const conf::configuration&);
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

#endif // _PNMC_MC_MC_HH_
