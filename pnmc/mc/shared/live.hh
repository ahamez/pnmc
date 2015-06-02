/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <functional> // hash
#include <iosfwd>

#include <boost/dynamic_bitset.hpp>

namespace pnmc { namespace mc { namespace shared {

/*------------------------------------------------------------------------------------------------*/

struct live
{
  const std::size_t index;
  boost::dynamic_bitset<>& bitset;

  template <typename T>
  T
  operator()(const T& val)
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

}}} // namespace pnmc::mc::shared

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::shared::live>
{
  std::size_t
  operator()(const pnmc::mc::shared::live& l)
  const noexcept
  {
    using namespace sdd::hash;
    return seed(49979687) (val(l.index));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
