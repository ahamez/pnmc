#ifndef _PNMC_MC_LIVE_HH
#define _PNMC_MC_LIVE_HH

#include <functional> // hash
#include <iosfwd>

#include <boost/dynamic_bitset.hpp>

#include "sdd/values/flat_set.hh"
#include "sdd/values_manager.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

struct live
{
  const std::size_t index;
  boost::dynamic_bitset<>& bitset;

  live(std::size_t, boost::dynamic_bitset<>&);

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>&)
  const noexcept;
};

/// @brief Equality of two post.
bool
operator==(const live&, const live&)
noexcept;

/// @brief Textual output of a post.
std::ostream&
operator<<(std::ostream&, const live&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::live>
{
  std::size_t operator()(const pnmc::mc::live&) const noexcept;
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_LIVE_HH
