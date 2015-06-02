/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <functional> // hash
#include <iosfwd>

#include <sdd/values/flat_set.hh>
#include <sdd/values_manager.hh>

#include "mc/classic/sdd.hh"
#include "mc/shared/exceptions.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct bounded_post
{
  const pn::valuation_type valuation;
  const pn::valuation_type bound;
  const std::string& place;

  flat_set
  operator()(const flat_set& val)
  const
  {
    flat_set_builder builder;
    builder.reserve(val.size());
    for (const auto& v : val)
    {
      if (v >= bound)
      {
        throw shared::bound_error(place);
      }
      builder.insert(builder.end(), v + valuation);
    }
    return std::move(builder);
  }

  friend
  bool
  operator==(const bounded_post& lhs, const bounded_post& rhs)
  noexcept
  {
    return lhs.valuation == rhs.valuation and lhs.bound == rhs.bound;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const bounded_post& p)
  {
    return os << "bounded_post(" << p.valuation << "," << p.bound <<")";
  }

};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::classic::bounded_post>
{
  std::size_t
  operator()(const pnmc::mc::classic::bounded_post& p)
  const noexcept
  {
    using namespace sdd::hash;
    return seed(98678683) (val(p.valuation)) (val(p.bound));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
