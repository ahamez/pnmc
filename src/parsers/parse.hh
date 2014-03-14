#ifndef _PNMC_PARSERS_PARSE_HH_
#define _PNMC_PARSERS_PARSE_HH_

#include <iosfwd>
#include <memory>

#include "conf/configuration.hh"
#include "pn/net.hh"

namespace pnmc { namespace parsers {
  
/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
parse(const conf::configuration&, std::istream&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers

#endif // _PNMC_PARSERS_PARSE_HH_
