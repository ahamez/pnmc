/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/classic/sdd.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct set
{
  const pn::clock_type value;

  flat_set
  operator()(const flat_set&)
  const
  {
    return {value};
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
