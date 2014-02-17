#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/bound_error.hh"
#include "mc/bounded_post.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

bounded_post::bounded_post(unsigned int v, unsigned int b, const std::string& p)
  : valuation_(v), bound_(b), place_(p)
{}

/*------------------------------------------------------------------------------------------------*/

sdd::values::flat_set<unsigned int>
bounded_post::operator()(const sdd::values::flat_set<unsigned int>& val)
const
{
  sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
  for (const auto& v : val)
  {
    if (v > bound_)
    {
      throw bound_error(place_);
    }
    builder.insert(v + valuation_);
  }
  return std::move(builder);
}

/*------------------------------------------------------------------------------------------------*/

bool
operator==(const bounded_post& lhs, const bounded_post& rhs)
noexcept
{
  return lhs.valuation_ == rhs.valuation_ and lhs.bound_ == rhs.bound_;
}

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const bounded_post& p)
{
  return os << "bounded_post(" << p.valuation_ << "," << p.bound_ <<")";
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

namespace std
{

/*------------------------------------------------------------------------------------------------*/

std::size_t
hash<pnmc::mc::bounded_post>::operator()(const pnmc::mc::bounded_post& p)
const noexcept
{
  std::size_t seed = 98678683;
  sdd::util::hash_combine(seed, p.valuation_);
  sdd::util::hash_combine(seed, p.bound_);
  return seed;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace std
