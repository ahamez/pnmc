#pragma once

#include <algorithm>  // transform
#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>
#include "sdd/values/flat_set.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct post
{
  const unsigned int valuation;

  post(unsigned int v)
    : valuation(v)
  {}

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const
  {
    sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
    builder.reserve(val.size());
    std::transform( val.cbegin(), val.cend(), std::inserter(builder, builder.end())
                  , [this](unsigned int v){return v + valuation;});
    return std::move(builder);
  }

  bool
  shifter()
  const noexcept
  {
    return true;
  }

  /// @brief Equality of two post.
  friend
  bool
  operator==(const post& lhs, const post& rhs)
  noexcept
  {
    return lhs.valuation == rhs.valuation;
  }

  /// @brief Textual output of a post.
  friend
  std::ostream&
  operator<<(std::ostream& os, const post& p)
  {
    return os << "post(" << p.valuation << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::classic::post>
{
  std::size_t
  operator()(const pnmc::mc::classic::post& p)
  const noexcept
  {
    using namespace sdd::hash;
    return seed(2564450027) (val(p.valuation));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
