#pragma once

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include "support/conf/pn_format.hh"

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

struct pn_configuration
{
  boost::optional<boost::filesystem::path> file;
  conf::pn_format file_type;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
