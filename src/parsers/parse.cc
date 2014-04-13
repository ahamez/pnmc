#include <istream>

#include "parsers/bpn.hh"
#include "parsers/parse.hh"
#include "parsers/pnml.hh"
#include "parsers/prod.hh"
#include "parsers/tina.hh"
#include "parsers/xml.hh"

namespace pnmc { namespace parsers {
  
/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
parse(const conf::configuration& conf, std::istream& in)
{
  switch (conf.file_type)
  {
    case (conf::input_format::bpn)  : return parsers::bpn(in);
    case (conf::input_format::pnml) : return parsers::pnml(in);
    case (conf::input_format::prod) : return parsers::prod(in);
    case (conf::input_format::tina) : return parsers::tina(in);
    case (conf::input_format::xml)  : break;
  }
  return parsers::xml(in);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
