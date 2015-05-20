#include <ostream>

#include "support/pn/place.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

place::place(std::size_t id, const std::string& n, valuation_type m)
  : uid{id}, name{n}, marking{m}, pre{}, post{}
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
  return lhs.uid < rhs.uid;
}

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const pn::place& p)
{
  return os << "pl " << p.name  <<  " (" << p.marking << ")";
}

/*------------------------------------------------------------------------------------------------*/

}} // pnmc::pn
