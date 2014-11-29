#pragma once

#include <iosfwd>
#include <memory>

#include "support/parsers/pn_configuration.hh"
#include "support/pn/net.hh"

namespace pnmc { namespace parsers {
  
/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
parse(const pn_configuration&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
