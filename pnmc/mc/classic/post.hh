/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <algorithm>  // transform
#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>

#include "mc/classic/sdd.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct post
{
  const pn::valuation_type valuation;

  flat_set
  operator()(const flat_set& val)
  const
  {
    auto builder = flat_set_builder{};
    builder.reserve(val.size());
    std::transform( val.cbegin(), val.cend(), std::inserter(builder, builder.end())
                  , [this](unsigned int v){return v + valuation;});
    return builder;
  }

  bool
  shifter()
  const noexcept
  {
    return true;
  }

  friend
  bool
  operator==(const post& lhs, const post& rhs)
  noexcept
  {
    return lhs.valuation == rhs.valuation;
  }

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
