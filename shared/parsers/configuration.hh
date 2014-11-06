#pragma once

#include <string>

#include "shared/conf/pn_format.hh"

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

struct configuration
{
  bool read_stdin;
  bool decompress;
  std::string file_name;
  conf::pn_format file_type;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
