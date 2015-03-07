#pragma once

#include "conf/configuration.hh"
#include "mc/mc_impl.hh"

namespace pnmc { namespace mc { namespace nupn {

/*------------------------------------------------------------------------------------------------*/

struct worker
  : public mc_impl
{
  conf::configuration conf;

  worker(const conf::configuration& c)
    : conf(c)
  {}

  void
  operator()(const pn::net& net, const properties::formulae&) override;
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::nupn
