#include <algorithm>
#include <ostream>

#include "pn/tina.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

namespace /* anonymous */ {

std::string
format(std::string str)
{
  std::transform( str.cbegin(), str.cend(), str.begin()
                , [](char c){if (c == '.' or c == '-') return '_'; else return c;});
  return str;
}

}

/*------------------------------------------------------------------------------------------------*/

void
tina(std::ostream& out, const net& net)
{
  for (const auto& transition : net.transitions())
  {
    out << "tr " << format(transition.id);
    for (const auto& arc : transition.pre)
    {
      out << " " << format(arc.target);
    }
    out << " ->";
    for (const auto& arc : transition.post)
    {
      out << " " << format(arc.target);
    }
    out << std::endl;
  }

  for (const auto& place : net.places())
  {
    if (place.marking > 0)
    {
      out << "pl " << format(place.id) << " (" << place.marking << ")" << std::endl;
    }
  }

  out << "net " << format(net.name) << std::endl;
}

/*------------------------------------------------------------------------------------------------*/

}} // pnmc::pn
