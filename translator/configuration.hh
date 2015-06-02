/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include "support/parsers/pn_configuration.hh"

namespace pnmc { namespace conf {
  
/*------------------------------------------------------------------------------------------------*/

struct configuration
{
  parsers::pn_configuration input;
  boost::filesystem::path tina_output_file;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::configuration
