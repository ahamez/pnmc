#include <ostream>

#include "pn/transition.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

transition::transition(const std::string& id, std::size_t index)
  : id(id), index(index), pre(), post()
{}

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
  for(auto p : t.pre)
  {
    os << " " << p.first << p.second;
  }
  os << " -> ";
  for(auto p : t.post)
  {
    os << " " << p.first << p.second;
  }  
  return os;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
