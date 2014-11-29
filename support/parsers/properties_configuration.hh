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
