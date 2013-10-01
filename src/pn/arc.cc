#include <cassert>
#include <iostream>

#include "pn/arc.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const arc& a)
{
  switch(a.kind)
  {
    case arc::type::normal:
      os << "*";
      break;

    case arc::type::test:
      os << "?";
      break;            

    case arc::type::inhibitor:
      os << "?-";
      break;      

    case arc::type::stopwatch:
      os << "!";
      break;      

    case arc::type::stopwatch_inhibitor:
      os << "!-";
      break;            
            
    default:
      assert(false);
      break;
  }
  
  return os << " " << a.weight;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
