#pragma once

#include <string>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include "support/conf/pn_format.hh"

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

struct configuration
{
  boost::optional<boost::filesystem::path> file;
  conf::pn_format file_type;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
