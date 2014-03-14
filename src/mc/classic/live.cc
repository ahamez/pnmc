#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/live.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

live::live(std::size_t index, boost::dynamic_bitset<>& bitset)
  : index(index), bitset(bitset)
{}

/*------------------------------------------------------------------------------------------------*/

sdd::values::flat_set<unsigned int>
live::operator()(const sdd::values::flat_set<unsigned int>& val)
const noexcept
{
  bitset[index] = true;
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

}} // namespace pnmc::mc

namespace std
{

/*------------------------------------------------------------------------------------------------*/

std::size_t
hash<pnmc::mc::live>::operator()(const pnmc::mc::live& l)
const noexcept
{
  std::size_t seed = 49979687;
  sdd::util::hash_combine(seed, l.index);
  return seed;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace std
