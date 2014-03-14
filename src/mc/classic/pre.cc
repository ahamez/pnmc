#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/classic/pre.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

pre::pre(unsigned int v)
  : valuation(v)
{}

/*------------------------------------------------------------------------------------------------*/

sdd::values::flat_set<unsigned int>
pre::operator()(const sdd::values::flat_set<unsigned int>& val)
const
{
  sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;

  // Find the first entry in val that is greater or equal than valuation.
  // We can do that as values in val are sorted.
  // Will cut the path if cit == end.
  for (auto cit = val.lower_bound(valuation); cit != val.cend(); ++cit)
  {
    builder.insert(*cit - valuation);
  }
  return std::move(builder);
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

}}} // namespace pnmc::mc::classic

/*------------------------------------------------------------------------------------------------*/

namespace std {

/*------------------------------------------------------------------------------------------------*/

std::size_t
hash<pnmc::mc::classic::pre>::operator()(const pnmc::mc::classic::pre& p)
const noexcept
{
  std::size_t seed = 3464152273;
  sdd::util::hash_combine(seed, p.valuation);
  return seed;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace std
