/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <boost/optional.hpp>

#include "configuration.hh"

namespace pnmc { namespace conf {
  
/*------------------------------------------------------------------------------------------------*/

boost::optional<configuration>
fill_configuration(int, const char**);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
