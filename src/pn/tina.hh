#ifndef _PNMC_PN_TINA_HH_
#define _PNMC_PN_TINA_HH_

#include <iosfwd>
#include <string>

#include "pn/net.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

/// @brief Export net to TINA format.
void
tina(std::ostream&, const net&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn

#endif // _PNMC_PN_TINA_HH_
