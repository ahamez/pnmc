/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

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
