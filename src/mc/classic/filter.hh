#ifndef _PNMC_MC_FILTER_HH_
#define _PNMC_MC_FILTER_HH_

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>
#include "sdd/values/flat_set.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct filter
{
  const unsigned int value;

  filter(unsigned int v)
    : value(v)
  {}

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const
  {
    sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
    for (auto cit = val.lower_bound(value); cit != val.cend(); ++cit)
    {
      builder.insert(*cit);
    }
    return std::move(builder);
  }

  bool
  selector()
  const noexcept
  {
    return true;
  }

  /// @brief Equality.
  friend
  bool
  operator==(const filter& lhs, const filter& rhs)
  noexcept
  {
    return lhs.value == rhs.value;
  }

  /// @brief Textual output.
  friend
  std::ostream&
  operator<<(std::ostream& os, const filter& e)
  {
    return os << "filter(" << e.value << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::classic::filter>
{
  std::size_t
  operator()(const pnmc::mc::classic::filter& e)
  const noexcept
  {
    std::size_t seed = 3267553927;
    sdd::util::hash_combine(seed, e.value);
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_FILTER_HH_
