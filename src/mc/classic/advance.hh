#ifndef _PNMC_MC_ADVANCE_HH_
#define _PNMC_MC_ADVANCE_HH_

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>
#include <sdd/values/flat_set.hh>

#include "pn/transition.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct advance
{
  const unsigned int upper_clock;

  advance(unsigned int c)
    : upper_clock(c)
  {}

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const
  {
    sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
    for (const auto v : val)
    {
      if ((v + 1) <= upper_clock)
      {
        builder.insert(v + 1);
      }
    }
    // As a flat_set is sorted, # is always the last, if it exists
    if (*val.crbegin() == pn::sharp)
    {
      builder.insert(pn::sharp);
    }
    return std::move(builder);
  }

//  ???
//  bool
//  shifter()
//  const noexcept
//  {
//    return true;
//  }

  /// @brief Equality.
  friend
  bool
  operator==(const advance& lhs, const advance& rhs)
  noexcept
  {
    return lhs.upper_clock == rhs.upper_clock;
  }

  /// @brief Textual output.
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
    std::size_t seed = 1790863982;
    sdd::util::hash_combine(seed, a.upper_clock);
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_ADVANCE_HH_
