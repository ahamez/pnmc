#include <istream>

#include "parsers/parse.hh"
#include "parsers/prod.hh"
#include "parsers/tina.hh"
#include "parsers/xml.hh"

namespace pnmc { namespace parsers {
  
/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
parse(std::istream& in)
{
  std::shared_ptr<pn::net> net_ptr;

  // TINA pase.
  net_ptr = parsers::tina(in);
  if (net_ptr)
  {
    return net_ptr;
  }

  // TINA parse failed, let's try PROD.
  in.seekg(0, std::ios::beg);
  net_ptr = parsers::prod(in);
  if (net_ptr)
  {
    return net_ptr;
  }

  // PROD parse failed, let's try XML.
  in.seekg(0, std::ios::beg);
  net_ptr = parsers::xml(in);
  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
