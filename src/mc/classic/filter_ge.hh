#pragma once

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>
#include "sdd/values/flat_set.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct filter_ge
{
  const unsigned int value;

  filter_ge(unsigned int v)
    : value(v)
  {}

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const
  {
    sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
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

  /// @brief Equality.
  friend
  bool
  operator==(const filter_ge& lhs, const filter_ge& rhs)
  noexcept
  {
    return lhs.value == rhs.value;
  }

  /// @brief Textual output.
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
