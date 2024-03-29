/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>
#include <sdd/values/flat_set.hh>

#include "mc/classic/sdd.hh"
#include "support/pn/constants.hh"
#include "support/pn/transition.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

/// @brief Advance clocks up to a given value, always keep # if it exists
struct advance
{
  const pn::clock_type upper_clock;

  flat_set
  operator()(const flat_set& val)
  const
  {
    auto builder = flat_set_builder{};
    builder.reserve(val.size());
    for (const auto v : val)
    {
      if ((v + 1) <= upper_clock)
      {
        builder.insert(builder.end(), v + 1);
      }
    }
    // As a flat_set is sorted, # is always the last, if it exists
    if (*val.crbegin() == pn::sharp)
    {
      builder.insert(builder.end(), pn::sharp);
    }
    return builder;
  }

  bool
  shifter()
  const noexcept
  {
    return true;
  }

  friend
  bool
  operator==(const advance& lhs, const advance& rhs)
  noexcept
  {
    return lhs.upper_clock == rhs.upper_clock;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const advance& a)
  {
    return os << "advance(" << a.upper_clock << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::classic::advance>
{
  std::size_t
  operator()(const pnmc::mc::classic::advance& a)
  const noexcept
  {
    using namespace sdd::hash;
    return seed(1790863982) (val(a.upper_clock));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
