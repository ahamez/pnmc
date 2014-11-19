#pragma once

#include <memory>

#include "conf/configuration.hh"
#include "mc/mc_impl.hh"
#include "support/pn/net.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

class mc
{
private:

  const std::unique_ptr<mc_impl> impl_;

public:

  mc(const conf::configuration&);

  void
  operator()(const pn::net&);
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc
