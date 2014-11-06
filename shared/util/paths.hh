#pragma once

#include <boost/filesystem.hpp>

namespace pnmc { namespace util {

/*------------------------------------------------------------------------------------------------*/

boost::filesystem::path
file(const std::string&);

/*------------------------------------------------------------------------------------------------*/

boost::filesystem::path
in_file(const std::string&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::util
