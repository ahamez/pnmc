#ifndef _PNMC_MC_PRE_HH_
#define _PNMC_MC_PRE_HH_

#include <functional> // hash
#include <ostream>


#include <sdd/util/hash.hh>
#include <sdd/values/flat_set.hh>

#include "conf/configuration.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct pre
{
  const unsigned int valuation;

  pre(unsigned int v)
    : valuation(v)
  {}

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const
  {
    sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;

    // Find the first entry in val that is greater or equal than valuation.
    // Will cut the path if cit == end.
    for (auto cit = val.lower_bound(valuation); cit != val.cend(); ++cit)
    {
      builder.insert(*cit - valuation);
    }
    return std::move(builder);
  }

  bool
  selector()
  const noexcept
  {
    return true;
  }

  /// @brief Equality of two pre.
  friend
  bool
  operator==(const pre& lhs, const pre& rhs)
  noexcept
  {
    return lhs.valuation == rhs.valuation;
  }

  /// @brief Textual output of a pre.
  friend
  std::ostream&
  operator<<(std::ostream& os, const pre& p)
  {
    return os << "pre(" << p.valuation << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std {

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::classic::pre>
{
  std::size_t
  operator()(const pnmc::mc::classic::pre& p)
  const noexcept
  {
    std::size_t seed = 3464152273;
    sdd::util::hash_combine(seed, p.valuation);
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_PRE_HH_
