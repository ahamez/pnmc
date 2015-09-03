/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <ostream>

#include "support/pn/constants.hh"
#include "support/pn/transition.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

transition::transition(std::size_t id, std::string  n)
  : uid(id), name(std::move(n)), pre(), post(), low(0), high(infinity)
{}

/*------------------------------------------------------------------------------------------------*/

bool
transition::timed()
const
{
  return this->low != 0 or this->high != infinity;
}

/*------------------------------------------------------------------------------------------------*/

bool
operator<(const transition& left, const transition& right)
noexcept
{
  return left.uid < right.uid;
}

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const transition& t)
{
  os << "tr " << t.name;
  if (t.low != 0 or t.high != infinity)
  {
    os << " |" << t.low << ",";
    if (t.high != infinity)
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
    os << " " << p.first << p.second.weight;
  }
  os << " -> ";
  for(auto p : t.post)
  {
    os << " " << p.first << p.second.weight;
  }  
  return os;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
