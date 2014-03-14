#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/classic/post.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

post::post(unsigned int v)
  : valuation(v)
{}

/*------------------------------------------------------------------------------------------------*/

sdd::values::flat_set<unsigned int>
post::operator()(const sdd::values::flat_set<unsigned int>& val)
const
{
  sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
  for (const auto& v : val)
  {
    builder.insert(v + valuation);
  }
  return std::move(builder);
}

/*------------------------------------------------------------------------------------------------*/

bool
operator==(const post& lhs, const post& rhs)
noexcept
{
  return lhs.valuation == rhs.valuation;
}

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const post& p)
{
  return os << "post(" << p.valuation << ")";
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

std::size_t
hash<pnmc::mc::classic::post>::operator()(const pnmc::mc::classic::post& p)
const noexcept
{
  std::size_t seed = 2564450027;
  sdd::util::hash_combine(seed, p.valuation);
  return seed;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace std
