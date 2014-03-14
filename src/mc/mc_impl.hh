#ifndef _PNMC_MC_MC_IMPL_HH_
#define _PNMC_MC_MC_IMPL_HH_

#include "pn/net.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

class mc_impl
{
public:

  virtual ~mc_impl() {}

  virtual
  void
  operator()(const pn::net&)
  const = 0;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

#endif // _PNMC_MC_MC_IMPL_HH_
