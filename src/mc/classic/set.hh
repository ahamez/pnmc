#ifndef _PNMC_MC_SET_HH_
#define _PNMC_MC_SET_HH_

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>
#include "sdd/values/flat_set.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct set
{
  const unsigned int value;

  set(unsigned int v)
    : value(v)
  {}

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>&)
  const
  {
    return sdd::values::flat_set<unsigned int>({value});
  }

  /// @brief Equality.
  friend
  bool
  operator==(const set& lhs, const set& rhs)
  noexcept
  {
    return lhs.value == rhs.value;
  }

  /// @brief Textual output.
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
    std::size_t seed = 1569850987;
    sdd::util::hash_combine(seed, s.value);
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_SET_HH_
