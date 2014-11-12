#pragma once

#include <type_traits>

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

using valuation_type = unsigned int;
static_assert(std::is_integral<valuation_type>::value, "Valuation must be an integral type");

using clock_type = unsigned int;
static_assert(std::is_integral<valuation_type>::value, "Clock must be an integral type");

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
