/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

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
