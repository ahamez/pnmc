#pragma once

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/classic/sdd.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct filter_ge
{
  const pn::valuation_type value;

  flat_set
  operator()(const flat_set& val)
  const
  {
    flat_set_builder builder;
    builder.reserve(val.size());
    for (auto cit = val.lower_bound(value); cit != val.cend(); ++cit)
    {
      builder.insert(builder.end(), *cit);
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
  operator==(const filter_ge& lhs, const filter_ge& rhs)
  noexcept
  {
    return lhs.value == rhs.value;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const filter_ge& e)
  {
    return os << "filter_ge(" << e.value << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::classic::filter_ge>
{
  std::size_t
  operator()(const pnmc::mc::classic::filter_ge& e)
  const noexcept
  {
    using namespace sdd::hash;
    return seed(3267553927) (val(e.value));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
