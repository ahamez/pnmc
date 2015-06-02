/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <limits>

#include "support/pn/types.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

/// @brief Time infinity.
constexpr auto infinity = std::numeric_limits<valuation_type>::max();

/// @brief Time sharp.
constexpr auto sharp = infinity - 1;

/*------------------------------------------------------------------------------------------------*/
  
}} // namespace pnmc::pn
