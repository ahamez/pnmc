#pragma once

#include <iosfwd>
#include <vector>

#include "support/properties/formulae.hh"

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

properties::formulae
mcc(std::istream&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
