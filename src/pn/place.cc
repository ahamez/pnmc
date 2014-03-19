#include <ostream>

#include "pn/place.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

place::place(const std::string& id, unsigned int m)
  : id(id), marking(m), pre(), post()
{}

/*------------------------------------------------------------------------------------------------*/

bool
place::connected()
const noexcept
{
  return not(pre.empty() and post.empty());
}

/*------------------------------------------------------------------------------------------------*/

bool
operator<(const pn::place& lhs, const pn::place& rhs)
noexcept
{
  return lhs.id < rhs.id;
}

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const pn::place& p)
{
  return os << "pl " << p.id  <<  " (" << p.marking << ")";
}

/*------------------------------------------------------------------------------------------------*/

}} // pnmc::pn
