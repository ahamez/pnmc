#pragma once

#include <iosfwd>
#include <memory>

#include "support/parsers/configuration.hh"
#include "support/pn/net.hh"

namespace pnmc { namespace parsers {
  
/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
parse(const configuration&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
