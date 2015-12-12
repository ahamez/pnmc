/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include "conf/configuration.hh"
#include "mc/mc_impl.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct classic
  : public mc_impl
{
  conf::configuration conf;

  classic(conf::configuration  c)
    : conf(std::move(c))
  {}

  void
  operator()(const pn::net& net, const properties::formulae&) override;
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
