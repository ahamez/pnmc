#ifndef _PNMC_MC_DEAD_HH_
#define _PNMC_MC_DEAD_HH_

#include <functional> // hash
#include <iosfwd>

#include "sdd/values/flat_set.hh"
#include "sdd/values_manager.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

struct dead
{
  const unsigned int valuation;

  dead(unsigned int);

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>&) const;
};

/// @brief Equality of two dead.
bool
operator==(const dead&, const dead&)
noexcept;

/// @brief Textual output of a dead.
std::ostream&
operator<<(std::ostream&, const dead&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::dead>
{
  std::size_t operator()(const pnmc::mc::dead&) const noexcept;
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_DEAD_HH_
