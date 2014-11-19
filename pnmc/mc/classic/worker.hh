#pragma once

#include "conf/configuration.hh"
#include "mc/mc_impl.hh"
#include "support/pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct worker
  : public mc_impl
{
  conf::configuration conf;

  worker(const conf::configuration& c)
    : conf(c)
  {}

  void
  operator()(const pn::net& net);
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
