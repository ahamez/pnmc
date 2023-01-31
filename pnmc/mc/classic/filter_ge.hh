/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/classic/sdd.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

/// @brief Keep values greater or equal than a given value
struct filter_ge
{
  const pn::valuation_type value;

  flat_set
  operator()(const flat_set& val)
  const
  {
    auto builder = flat_set_builder{};
    builder.reserve(val.size());
    // Only keep values that are greater or equal than the requested valuation.
    std::copy(val.lower_bound(value), val.cend(), std::inserter(builder, builder.end()));
    return builder;
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
