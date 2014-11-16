#pragma once

#include <iosfwd>
#include <string>

#include "support/pn/net.hh"

namespace pnmc { namespace translator {

/*------------------------------------------------------------------------------------------------*/

struct net_to_tina
{
  const pn::net& net;

  friend
  std::ostream&
  operator<<(std::ostream& os, const net_to_tina& manip);
};

/*------------------------------------------------------------------------------------------------*/

net_to_tina
tina(const pn::net&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::translator
