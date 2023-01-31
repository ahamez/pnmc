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

struct advance_capped
{
  const pn::clock_type lower_clock;
  const pn::clock_type upper_clock;

  flat_set
  operator()(const flat_set& val)
  const
  {
    auto builder = flat_set_builder{};
    builder.reserve(val.size());
    for (auto cit = val.cbegin(); cit != val.lower_bound(upper_clock); ++cit)
    {
      builder.insert(builder.end(), *cit >= lower_clock ? *cit : *cit + 1);
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

  /// @brief Equality.
  friend
  bool
  operator==(const advance_capped& lhs, const advance_capped& rhs)
  noexcept
  {
    return lhs.lower_clock == rhs.lower_clock and lhs.upper_clock == rhs.upper_clock;
  }

  /// @brief Textual output.
  friend
  std::ostream&
  operator<<(std::ostream& os, const advance_capped& a)
  {
    return os << "advance(" << a.lower_clock << "," << a.upper_clock << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::classic::advance_capped>
{
  std::size_t
  operator()(const pnmc::mc::classic::advance_capped& a)
  const noexcept
  {
    using namespace sdd::hash;
    return seed (7320863981) (val(a.lower_clock)) (val(a.upper_clock));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
