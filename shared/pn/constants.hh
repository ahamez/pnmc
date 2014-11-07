#pragma once

#include <limits>

#include "shared/pn/types.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

/// @brief Time infinity.
constexpr auto infinity = std::numeric_limits<valuation_type>::max();

/// @brief Time sharp.
constexpr auto sharp = infinity - 1;

/*------------------------------------------------------------------------------------------------*/
  
}} // namespace pnmc::pn
