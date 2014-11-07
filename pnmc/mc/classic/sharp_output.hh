#pragma once

#include <iosfwd>

#include "sdd/values/flat_set.hh"
#include "shared/pn/constants.hh"

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
