#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/classic/live.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

live::live(std::size_t i, boost::dynamic_bitset<>& b, results& r)
  : index(i), bitset(b), res(r)
{}

/*------------------------------------------------------------------------------------------------*/

sdd::values::flat_set<unsigned int>
live::operator()(const sdd::values::flat_set<unsigned int>& val)
const noexcept
{
  bitset[index] = true;
  ++res.nb_fired_transitions;
  return val;
}

/*------------------------------------------------------------------------------------------------*/

bool
operator==(const live& lhs, const live& rhs)
noexcept
{
  return lhs.index == rhs.index;
}

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const live& l)
{
  return os << "live(" << l.index << ")";
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

std::size_t
hash<pnmc::mc::classic::live>::operator()(const pnmc::mc::classic::live& l)
const noexcept
{
  std::size_t seed = 49979687;
  sdd::util::hash_combine(seed, l.index);
  return seed;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace std
