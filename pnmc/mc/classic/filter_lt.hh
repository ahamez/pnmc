#pragma once

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/classic/sdd.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct filter_lt
{
  const pn::valuation_type value;

  flat_set
  operator()(const flat_set& val)
  const
  {
    flat_set_builder builder;
    builder.reserve(val.size());
    for (auto cit = val.cbegin(); cit != val.lower_bound(value); ++cit)
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
  operator==(const filter_lt& lhs, const filter_lt& rhs)
  noexcept
  {
    return lhs.value == rhs.value;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const filter_lt& e)
  {
    return os << "filter_lt(" << e.value << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::classic::filter_lt>
{
  std::size_t
  operator()(const pnmc::mc::classic::filter_lt& e)
  const noexcept
  {
    using namespace sdd::hash;
    return seed(7247563421) (val(e.value));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
