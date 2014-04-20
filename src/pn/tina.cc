#include <ostream>

#include "pn/tina.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

void
tina(std::ostream& out, const net& net)
{
  for (const auto& transition : net.transitions())
  {
    out << "tr t" << transition.id;
    for (const auto& arc : transition.pre)
    {
      out << " p" << arc.first;
    }
    out << " ->";
    for (const auto& arc : transition.post)
    {
      out << " p" << arc.first;
    }
    out << "\n";
  }

  for (const auto& place : net.places())
  {
    out << "pl p" << place.id << " (" << place.marking << ")\n";
  }

  out << "net generated_net\n";
}

/*------------------------------------------------------------------------------------------------*/

}} // pnmc::pn
