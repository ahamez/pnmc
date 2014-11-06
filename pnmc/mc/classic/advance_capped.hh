#pragma once

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>
#include <sdd/values/flat_set.hh>

#include "shared/pn/transition.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct advance_capped
{
  const unsigned int lower_clock;
  const unsigned int upper_clock;

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const
  {
    sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
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
    return std::move(builder);
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