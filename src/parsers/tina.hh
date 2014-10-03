#pragma once

#include <iosfwd>
#include <memory>

#include "pn/net.hh"

namespace pnmc { namespace parsers {
  
/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
tina(std::istream&);

/*------------------------------------------------------------------------------------------------*/

}} // pnmc::parsers
