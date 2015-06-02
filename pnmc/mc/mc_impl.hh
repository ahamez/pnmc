/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <vector>

#include "support/pn/net.hh"
#include "support/properties/formulae.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

struct mc_impl
{
  virtual ~mc_impl() {}
  virtual void operator()(const pn::net&, const properties::formulae&) = 0;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc
