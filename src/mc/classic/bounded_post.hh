#ifndef _PNMC_MC_BOUNDED_POST_HH_
#define _PNMC_MC_BOUNDED_POST_HH_

#include <functional> // hash
#include <iosfwd>

#include <sdd/values/flat_set.hh>
#include <sdd/values_manager.hh>

#include "mc/bound_error.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct bounded_post
{
  const unsigned int valuation_;
  const unsigned int bound_;
  const std::string& place_;

  bounded_post(unsigned int valuation, unsigned int bound, const std::string& place)
    : valuation_(valuation), bound_(bound), place_(place)
  {}

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const
  {
    sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
    for (const auto& v : val)
    {
      if (v > bound_)
      {
        throw bound_error<C>(place_);
      }
      builder.insert(v + valuation_);
    }
    return std::move(builder);
  }
};

/// @brief Equality of two bounded_post.
template <typename C>
bool
operator==(const bounded_post<C>& lhs, const bounded_post<C>& rhs)
noexcept
{
  return lhs.valuation_ == rhs.valuation_ and lhs.bound_ == rhs.bound_;
}

/// @brief Textual output of a bounded_post.
template <typename C>
std::ostream&
operator<<(std::ostream& os, const bounded_post<C>& p)
{
  return os << "bounded_post(" << p.valuation_ << "," << p.bound_ <<")";
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct hash<pnmc::mc::bounded_post<C>>
{
  std::size_t
  operator()(const pnmc::mc::bounded_post<C>& p)
  const noexcept
  {
    std::size_t seed = 98678683;
    sdd::util::hash_combine(seed, p.valuation_);
    sdd::util::hash_combine(seed, p.bound_);
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_BOUNDED_POST_HH_
