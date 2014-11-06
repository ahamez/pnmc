#pragma once

#include "shared/pn/net.hh"

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