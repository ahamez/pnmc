#pragma once

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>
#include "sdd/values/flat_set.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct dead
{
  const unsigned int valuation;

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const
  {
    sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
    builder.reserve(val.size());
    for (const auto v : val)
    {
      if (v < valuation)
      {
        builder.insert(builder.end(), v);
      }
    }
    return std::move(builder);
  }

  friend
  bool
  operator==(const dead& lhs, const dead& rhs)
  noexcept
  {
    return lhs.valuation == rhs.valuation;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const dead& d)
  {
    return os << "dead(" << d.valuation << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::classic::dead>
{
  std::size_t
  operator()(const pnmc::mc::classic::dead& d)
  const noexcept
  {
    using namespace sdd::hash;
    return seed(586486319) (val(d.valuation));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
