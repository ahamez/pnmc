#ifndef _PNMC_MC_PRE_HH_
#define _PNMC_MC_PRE_HH_

#include <functional> // hash
#include <iosfwd>

#include <sdd/values/flat_set.hh>
#include <sdd/values_manager.hh>

#include "conf/configuration.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

struct pre
{
  const conf::pnmc_configuration& conf;
  const unsigned int valuation;

  pre(const conf::pnmc_configuration&, unsigned int);

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>&) const;
};

/// @brief Equality of two pre.
bool
operator==(const pre&, const pre&)
noexcept;

/// @brief Textual output of a pre.
std::ostream&
operator<<(std::ostream&, const pre&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

namespace std {

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::pre>
{
  std::size_t operator()(const pnmc::mc::pre&) const noexcept;
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_PRE_HH_
