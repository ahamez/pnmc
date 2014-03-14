#include <algorithm> // copy
#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/classic/dead.hh"

namespace pnmc { namespace mc { namespace classic {

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

}}} // namespace pnmc::mc::classic

namespace std {

/*------------------------------------------------------------------------------------------------*/

std::size_t
hash<pnmc::mc::classic::dead>::operator()(const pnmc::mc::classic::dead& p)
const noexcept
{
  return sdd::util::hash(p.valuation);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace std
