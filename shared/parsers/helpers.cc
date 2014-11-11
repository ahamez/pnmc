#include "shared/parsers/helpers.hh"
#include "shared/parsers/parse_error.hh"

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

kw::kw(const std::string& k)
  : k_(k)
{}

/*------------------------------------------------------------------------------------------------*/

std::istream&
operator>>(std::istream& in, const kw& manip)
{
  std::string s;
  if (not (in >> s))
  {
    throw parse_error("Expected " + manip.k_ + ", got nothing.");
  }
  else if (s != manip.k_)
  {
    throw parse_error("Expected " + manip.k_ + ", got " + s);
  }
  return in;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
