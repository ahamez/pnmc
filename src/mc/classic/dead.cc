#include <algorithm> // copy
#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/dead.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

dead::dead(unsigned int v)
  : valuation(v)
{}

/*------------------------------------------------------------------------------------------------*/

sdd::values::flat_set<unsigned int>
dead::operator()(const sdd::values::flat_set<unsigned int>& val)
const
{
  sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
  for (const auto v : val)
  {
    if (v < valuation)
    {
      builder.insert(v);
    }
  }
  return std::move(builder);
}

/*------------------------------------------------------------------------------------------------*/

bool
operator==(const dead& lhs, const dead& rhs)
noexcept
{
  return lhs.valuation == rhs.valuation;
}

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const dead& p)
{
  return os << "dead(" << p.valuation << ")";
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

namespace std {

/*------------------------------------------------------------------------------------------------*/

std::size_t
hash<pnmc::mc::dead>::operator()(const pnmc::mc::dead& p)
const noexcept
{
  return sdd::util::hash(p.valuation);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace std
