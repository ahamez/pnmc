#include <chrono>
#include <ostream>

#include <sdd/sdd.hh>
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

/*------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------*/

timed_pre::timed_pre(const conf::pnmc_configuration& c, unsigned int v)
  : conf(c), valuation(v)
{}

/*------------------------------------------------------------------------------------------------*/

sdd::values::flat_set<unsigned int>
timed_pre::operator()(const sdd::values::flat_set<unsigned int>& val)
const
{
  if (std::chrono::system_clock::now() - conf.beginning >= conf.max_time)
  {
    throw sdd::interrupt<sdd::SDD<sdd::conf1>>();
  }

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
operator==(const timed_pre& lhs, const timed_pre& rhs)
noexcept
{
  return lhs.valuation == rhs.valuation;
}

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const timed_pre& p)
{
  return os << "pre(" << p.valuation << ")";
}

}} // namespace pnmc::mc

/*------------------------------------------------------------------------------------------------*/

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

std::size_t
hash<pnmc::mc::timed_pre>::operator()(const pnmc::mc::timed_pre& p)
const noexcept
{
  std::size_t seed = 3464152273;
  sdd::util::hash_combine(seed, p.valuation);
  return seed;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace std
