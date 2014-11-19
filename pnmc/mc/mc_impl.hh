#pragma once

#include "support/pn/net.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

struct mc_impl
{
  virtual ~mc_impl() {}
  virtual void operator()(const pn::net&) = 0;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc
