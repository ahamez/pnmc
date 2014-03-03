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
  const unsigned int valuation;

  pre(unsigned int);

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

struct timed_pre
{
  const conf::pnmc_configuration& conf;
  const unsigned int valuation;

  timed_pre(const conf::pnmc_configuration&, unsigned int);

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>&) const;
};

/// @brief Equality of two timed_pre.
bool
operator==(const timed_pre&, const timed_pre&)
noexcept;

/// @brief Textual output of a timed_pre.
std::ostream&
operator<<(std::ostream&, const timed_pre&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

namespace std {

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::pre>
{
  std::size_t operator()(const pnmc::mc::pre&) const noexcept;
};

template <>
struct hash<pnmc::mc::timed_pre>
{
  std::size_t operator()(const pnmc::mc::timed_pre&) const noexcept;
};


/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_PRE_HH_
