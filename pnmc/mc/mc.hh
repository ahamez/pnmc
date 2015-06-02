/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <memory>

#include "conf/configuration.hh"
#include "support/pn/net.hh"
#include "support/properties/formulae.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

// Forward declaration of implementation.
struct mc_impl;

class mc
{
private:

  const std::shared_ptr<mc_impl> impl_;

public:

  mc(const conf::configuration&);

  void
  operator()(const pn::net&, const properties::formulae&);
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc
