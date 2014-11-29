#pragma once

#include <iosfwd>
#include <vector>

#include "support/parsers/properties_configuration.hh"
#include "support/properties/formulae.hh"

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

properties::formulae
parse(const properties_configuration&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
