#pragma once

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>
#include <sdd/values/flat_set.hh>

#include "mc/classic/sdd.hh"
#include "shared/pn/constants.hh"
#include "shared/pn/transition.hh"
#include "shared/pn/types.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct advance
{
  const pn::clock_type upper_clock;

  flat_set
  operator()(const flat_set& val)
  const
  {
    flat_set_builder builder;
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
    return std::move(builder);
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
