#pragma once

#include <iosfwd>
#include <memory>

#include "support/pn/net.hh"

namespace pnmc { namespace parsers {
  
/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
net(std::istream&);

/*------------------------------------------------------------------------------------------------*/

}} // pnmc::parsers