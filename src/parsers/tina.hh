#ifndef _PNMC_PN_PARSERS_TINA_HH_
#define _PNMC_PN_PARSERS_TINA_HH_

#include <iosfwd>
#include <memory>

#include "pn/net.hh"

namespace pnmc { namespace parsers {
  
/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
tina(std::istream&);

/*------------------------------------------------------------------------------------------------*/

}} // pnmc::parsers

#endif // _PNMC_PN_PARSERS_TINA_HH_
