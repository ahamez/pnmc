#pragma once

#include <algorithm>  // transform
#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/classic/sdd.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct pre
{
  const pn::valuation_type valuation;

  flat_set
  operator()(const flat_set& val)
  const
  {
    flat_set_builder builder;
    builder.reserve(val.size());
    // Start from first entry in val that is greater or equal than valuation.
    std::transform( val.lower_bound(valuation), val.cend(), std::inserter(builder, builder.end())
                  , [this](unsigned int v){return v - valuation;});
    return std::move(builder);
  }

  bool
  selector()
  const noexcept
  {
    return true;
  }

  friend
  bool
  operator==(const pre& lhs, const pre& rhs)
  noexcept
  {
    return lhs.valuation == rhs.valuation;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const pre& p)
  {
    return os << "pre(" << p.valuation << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std {

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::classic::pre>
{
  std::size_t
  operator()(const pnmc::mc::classic::pre& p)
  const noexcept
  {
    using namespace sdd::hash;
    return seed(3464152273) (val(p.valuation));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
