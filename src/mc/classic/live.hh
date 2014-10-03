#pragma once

#include <functional> // hash
#include <iosfwd>

#include <boost/dynamic_bitset.hpp>

#include "sdd/values/flat_set.hh"
#include "sdd/values_manager.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct live
{
  const std::size_t index;
  boost::dynamic_bitset<>& bitset;

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const noexcept
  {
    bitset[index] = true;
    return val;
  }

  friend
  bool
  operator==(const live& lhs, const live& rhs)
  noexcept
  {
    return lhs.index == rhs.index;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const live& l)
  {
    return os << "live(" << l.index << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::classic::live>
{
  std::size_t
  operator()(const pnmc::mc::classic::live& l)
  const noexcept
  {
    using namespace sdd::hash;
    return seed(49979687) (val(l.index));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
