/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "support/parsers/mcc.hh"
#include "support/parsers/parse_properties.hh"

namespace pnmc { namespace parsers {
  
/*------------------------------------------------------------------------------------------------*/

properties::formulae
parse(const properties_configuration& conf)
{
  if (not conf.file)
  {
    return {};
  }

  boost::filesystem::ifstream file_stream;
  file_stream.open(*conf.file);

  if (file_stream.peek() == std::char_traits<char>::eof())
  {
    throw std::runtime_error("Empty properties file");
  }

  return parsers::mcc(file_stream);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
