#ifndef _PNMC_UTIL_EXPORT_TO_TINA_HH_
#define _PNMC_UTIL_EXPORT_TO_TINA_HH_

#include "conf/configuration.hh"
#include "pn/net.hh"

namespace pnmc { namespace util {

/*------------------------------------------------------------------------------------------------*/

/// @brief Export a Petri net to the TINA net format.
void
export_to_tina(const conf::configuration&, const pn::net&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::util

#endif // _PNMC_UTIL_EXPORT_TO_TINA_HH_
