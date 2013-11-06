#include "parsers/helpers.hh"
#include "parsers/parse_error.hh"

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

std::vector<std::string>
split(std::string::const_iterator cit, std::string::const_iterator cend, char delim)
{
  std::string tmp;
  tmp.reserve(32);
  std::vector<std::string> res;
  std::string::const_iterator last_delim;
  for (; cit != cend; ++cit)
  {
    if (*cit == delim)
    {
      res.push_back(tmp);
      tmp.clear();
      last_delim = cit;
    }
    else
    {
      tmp.push_back(*cit);
    }
  }

  if (last_delim != (cend - 1))
  {
    res.push_back(tmp);
  }

  return res;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
