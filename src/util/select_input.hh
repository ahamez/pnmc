#ifndef _PNMC_UTIL_SELECT_INPUT_HH_
#define _PNMC_UTIL_SELECT_INPUT_HH_

#include <iosfwd>
#include <memory>

#include "conf/configuration.hh"

namespace pnmc { namespace util {

/*------------------------------------------------------------------------------------------------*/

/// @brief Select the correct input (std::cin or file) according to the configuration.
std::shared_ptr<std::istream>
select_input(const conf::configuration&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::util

#endif // _PNMC_UTIL_SELECT_INPUT_HH_
