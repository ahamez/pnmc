/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <iosfwd>
#include <memory>

#include "support/parsers/pn_configuration.hh"
#include "support/pn/net.hh"

namespace pnmc { namespace parsers {
  
/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
parse(const pn_configuration&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
