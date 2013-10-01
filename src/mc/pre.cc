#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/pre.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

pre::pre(unsigned int v)
  : valuation(v)
{}

/*------------------------------------------------------------------------------------------------*/

sdd::values::flat_set<unsigned int>
pre::operator()(const sdd::values::flat_set<unsigned int>& val)
const
{
  // Find the first entry in val that is greater or equal than valuation.
  // We can do that as values in val are sorted.
  auto cit = val.lower_bound(valuation);
  sdd::values::flat_set<unsigned int> new_val;

  // Will cut the path if cit == end.
  for (; cit != val.cend(); ++cit)
  {
    new_val.insert(*cit - valuation);
  }
  return new_val;
}

/*------------------------------------------------------------------------------------------------*/

bool
operator==(const pre& lhs, const pre& rhs)
noexcept
{
  return lhs.valuation == rhs.valuation;
}

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const pre& p)
{
  return os << "pre(" << p.valuation << ")";
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

namespace std {

/*------------------------------------------------------------------------------------------------*/

std::size_t
hash<pnmc::mc::pre>::operator()(const pnmc::mc::pre& p)
const noexcept
{
  std::size_t seed = 3464152273;
  sdd::util::hash_combine(seed, p.valuation);
  return seed;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace std
