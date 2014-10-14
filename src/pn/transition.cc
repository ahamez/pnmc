#include <ostream>

#include "pn/transition.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

transition::transition(const std::string& i, std::size_t idx)
  : id(i), index(idx), pre(), post(), low(0), high(inf)
{}

/*------------------------------------------------------------------------------------------------*/

bool
transition::timed()
const
{
  return this->low != 0 or this->high != inf;
}

/*------------------------------------------------------------------------------------------------*/

/// Compare two transitions using their ids.
bool
operator<(const transition& left, const transition& right)
noexcept
{
  return left.index < right.index;
}

/*------------------------------------------------------------------------------------------------*/

/// Export a transition to an output stream.
std::ostream&
operator<<(std::ostream& os, const transition& t)
{
  os << "tr " << t.id;
  if (t.low != 0 or t.high != inf)
  {
    os << " |" << t.low << ",";
    if (t.high != inf)
    {
      os << t.high;
    }
    else
    {
      os << "w";
    }
    os << "|";
  }
  for(auto p : t.pre)
  {
    os << " " << p.target << p.weight;
  }
  os << " -> ";
  for(auto p : t.post)
  {
    os << " " << p.target << p.weight;
  }  
  return os;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
