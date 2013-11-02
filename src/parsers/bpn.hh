#ifndef _PNMC_PN_PARSERS_BPN_HH_
#define _PNMC_PN_PARSERS_BPN_HH_

#include <iosfwd>
#include <memory>

#include "pn/net.hh"

namespace pnmc { namespace parsers {
  
/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
bpn(std::istream&);

/*------------------------------------------------------------------------------------------------*/

}} // pnmc::parsers

#endif // _PNMC_PN_PARSERS_BPN_HH_
