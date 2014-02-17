#ifndef _PNMC_MC_BOUNDED_POST_HH_
#define _PNMC_MC_BOUNDED_POST_HH_

#include <functional> // hash
#include <iosfwd>

#include "sdd/values/flat_set.hh"
#include "sdd/values_manager.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

struct bounded_post
{
  const unsigned int valuation_;
  const unsigned int bound_;
  const std::string& place_;

  bounded_post(unsigned int valuation, unsigned int bound, const std::string& place);

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>&)
  const;
};

/// @brief Equality of two bounded_post.
bool
operator==(const bounded_post&, const bounded_post&)
noexcept;

/// @brief Textual output of a bounded_post.
std::ostream&
operator<<(std::ostream&, const bounded_post&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::bounded_post>
{
  std::size_t operator()(const pnmc::mc::bounded_post&) const noexcept;
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_BOUNDED_POST_HH_
