/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <algorithm>  // copy
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
    // Only keep values that are less than the requested valuation.
    std::copy(val.cbegin(), val.lower_bound(value), std::inserter(builder, builder.end()));
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
