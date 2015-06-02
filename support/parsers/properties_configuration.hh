/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

struct properties_configuration
{
  boost::optional<boost::filesystem::path> file;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
