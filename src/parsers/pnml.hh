#ifndef _PNMC_PN_PARSERS_PNML_HH_
#define _PNMC_PN_PARSERS_PNML_HH_

#include <iosfwd>
#include <memory>

#include "pn/net.hh"

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
pnml(std::istream&);

/*------------------------------------------------------------------------------------------------*/

}} // pnmc::parsers

#endif // _PNMC_PN_PARSERS_PNML_HH_
