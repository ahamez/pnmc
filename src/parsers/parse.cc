#include <istream>

#include "parsers/bpn.hh"
#include "parsers/parse.hh"
#include "parsers/prod.hh"
#include "parsers/tina.hh"
#include "parsers/xml.hh"

namespace pnmc { namespace parsers {
  
/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
parse(const conf::pnmc_configuration& conf, std::istream& in)
{
  std::shared_ptr<pn::net> net_ptr;

  switch (conf.file_type)
  {
    case (conf::input_format::bpn)  : return parsers::bpn(in);
    case (conf::input_format::prod) : return parsers::prod(in);
    case (conf::input_format::tina) : return parsers::tina(in);
    case (conf::input_format::xml)  : return parsers::xml(in);
  }
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
