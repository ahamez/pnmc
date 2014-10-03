#pragma once

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>
#include "sdd/values/flat_set.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct set
{
  const unsigned int value;

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>&)
  const
  {
    return sdd::values::flat_set<unsigned int>({value});
  }

  friend
  bool
  operator==(const set& lhs, const set& rhs)
  noexcept
  {
    return lhs.value == rhs.value;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const set& s)
  {
    return os << "set(" << s.value << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::classic::set>
{
  std::size_t
  operator()(const pnmc::mc::classic::set& s)
  const noexcept
  {
    using namespace sdd::hash;
    return seed(1569850987) (val(s.value));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
