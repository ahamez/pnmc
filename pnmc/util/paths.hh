#pragma once

#include <boost/filesystem.hpp>

namespace pnmc { namespace util {

/*------------------------------------------------------------------------------------------------*/

boost::filesystem::path
canonize_path(const std::string&);

/*------------------------------------------------------------------------------------------------*/

boost::filesystem::path
output_file(const std::string&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::util
