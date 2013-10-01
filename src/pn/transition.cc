#include <ostream>

#include "pn/transition.hh"

namespace pnmc { namespace pn {
  
/*------------------------------------------------------------------------------------------------*/
  
/// Compare two transitions using their ids.
bool
operator<(const transition& left, const transition& right)
noexcept
{
  return left.id < right.id;
}

/*------------------------------------------------------------------------------------------------*/
  
/// Export a transition to an output stream.
std::ostream&
operator<<(std::ostream& os, const transition& t)
{
  os << "tr " << t.id << ":" << t.label << " ";
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
