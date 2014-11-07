#pragma once

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/classic/sdd.hh"
#include "shared/pn/types.hh"

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

    // Find the first entry in val that is greater or equal than valuation.
    // Will cut the path if cit == end.
    for (auto cit = val.lower_bound(valuation); cit != val.cend(); ++cit)
    {
      builder.insert(builder.end(), *cit - valuation);
    }
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
