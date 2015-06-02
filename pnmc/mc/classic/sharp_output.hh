/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <iosfwd>

#include "sdd/values/flat_set.hh"
#include "support/pn/constants.hh"

namespace sdd { namespace values {

/*------------------------------------------------------------------------------------------------*/

template <>
struct display_value<unsigned int>
{
  void
  operator()(std::ostream& os, unsigned int v)
  const
  {
    if (v == pnmc::pn::sharp)
    {
      os << "#";
    }
    else
    {
      os << v;
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::values
