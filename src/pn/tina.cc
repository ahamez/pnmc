#include <algorithm>
#include <ostream>

#include "pn/tina.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

namespace /* anonymous */ {

std::string
format(const std::string& str)
{
  std::string res;
  res.reserve(str.size());
  std::transform( str.cbegin(), str.cend(), std::back_inserter(res)
                , [](char c){if (c == '.') return '_'; else return c;});
  return res;
}

}

/*------------------------------------------------------------------------------------------------*/

void
tina(std::ostream& out, const net& net)
{
  for (const auto& transition : net.transitions())
  {
    out << "tr t" << format(transition.id);
    for (const auto& arc : transition.pre)
    {
      out << " p" << format(arc.first);
    }
    out << " ->";
    for (const auto& arc : transition.post)
    {
      out << " p" << format(arc.first);
    }
    out << "\n";
  }

  for (const auto& place : net.places())
  {
    out << "pl p" << format(place.id) << " (" << place.marking << ")\n";
  }

  out << "net generated_net\n";
}

/*------------------------------------------------------------------------------------------------*/

}} // pnmc::pn
