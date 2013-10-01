#ifndef _PNMC_MC_POST_HH_
#define _PNMC_MC_POST_HH_

#include <functional> // hash
#include <iosfwd>

#include "sdd/values/flat_set.hh"
#include "sdd/values_manager.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

struct post
{
  const unsigned int valuation;

  post(unsigned int);

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>&)
  const;
};

/// @brief Equality of two post.
bool
operator==(const post&, const post&)
noexcept;

/// @brief Textual output of a post.
std::ostream&
operator<<(std::ostream&, const post&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::post>
{
  std::size_t operator()(const pnmc::mc::post&) const noexcept;
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_POST_HH_
