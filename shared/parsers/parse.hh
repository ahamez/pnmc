#pragma once

#include <iosfwd>
#include <memory>

#include "shared/parsers/configuration.hh"
#include "shared/pn/net.hh"

namespace pnmc { namespace parsers {
  
/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
parse(const configuration&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
