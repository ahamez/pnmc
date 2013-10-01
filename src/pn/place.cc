#include <ostream>

#include "pn/place.hh"

namespace pnmc { namespace pn {
  
/*------------------------------------------------------------------------------------------------*/

bool
operator<(const pn::place& lhs, const pn::place& rhs)
{
  return lhs.id < rhs.id;
}

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const pn::place& p)
{
  return os << "pl " << p.id << ":" << p.label <<  " (" << p.marking << ")";
}

/*------------------------------------------------------------------------------------------------*/

}} // pnmc::pn
