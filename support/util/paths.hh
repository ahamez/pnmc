/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <boost/filesystem.hpp>

namespace pnmc { namespace util {

/*------------------------------------------------------------------------------------------------*/

boost::filesystem::path
file(std::string);

/*------------------------------------------------------------------------------------------------*/

boost::filesystem::path
in_file(const std::string&);

/*------------------------------------------------------------------------------------------------*/

boost::filesystem::path
directory(const std::string&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::util
