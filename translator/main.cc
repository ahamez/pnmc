/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <iostream>
#include <sysexits.h>

#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>

#include "fill_configuration.hh"
#include "tina.hh"
#include "support/parsers/parse_pn.hh"
#include "support/parsers/parse_error.hh"
#include "support/pn/net.hh"

/*------------------------------------------------------------------------------------------------*/

int
main(int argc, const char** argv)
{
  using namespace pnmc;

  try
  {
    const auto conf_opt = conf::fill_configuration(argc, argv);
    if (conf_opt)
    {
      const auto& conf = *conf_opt;
      const auto net_ptr = parsers::parse(conf.input);

      boost::filesystem::ofstream stream{conf.tina_output_file};
      if (stream.is_open())
      {
        stream << translator::tina(*net_ptr);
      }
    }
    return EX_OK;
  }
  catch (const boost::program_options::error& e)
  {
    std::cerr << e.what() << std::endl;
    return EX_USAGE;
  }
  catch (const parsers::parse_error& e)
  {
    std::cerr << "Error when parsing input:" << std::endl;
    std::cerr << e.what() << std::endl;
    return EX_DATAERR;
  }
  catch (const std::runtime_error& e)
  {
    std::cerr << "Error:" << std::endl;
    std::cerr << e.what() << std::endl;
    return EX_DATAERR;
  }
}

/*------------------------------------------------------------------------------------------------*/
