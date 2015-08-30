/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <fstream>
#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "fill_configuration.hh"
#include "support/conf/options.hh"
#include "support/util/paths.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

namespace po = boost::program_options;

/*------------------------------------------------------------------------------------------------*/

static const auto help_str = "help";

/*------------------------------------------------------------------------------------------------*/

boost::optional<configuration>
fill_configuration(int argc, const char** argv)
{
  configuration conf;

  auto general_options = po::options_description{"General options"};
  general_options.add_options()
    (help_str, "Show this help")
  ;

  auto hidden_options = po::options_description{"Hidden options"};
  hidden_options.add_options()
    ("input-file"  , po::value<std::string>()
                   , "The Petri net file to read")
    ("output-file" , po::value<std::string>()
                   , "The TINA output file")
  ;

  auto p = po::positional_options_description{};
  p.add("input-file", 1)
   .add("output-file", 1)
  ;

  auto cmdline_options = po::options_description{};
  cmdline_options.add(general_options)
                 .add(hidden_options)
                 .add(pn_input_options());

  auto vm = po::variables_map{};
  auto parsed = po::command_line_parser(argc, argv).options(cmdline_options)
                                                   .positional(p)
                                                   .allow_unregistered()
                                                   .run();
  po::store(parsed, vm);
  po::notify(vm);

  auto unrecognized = po::collect_unrecognized(parsed.options, po::exclude_positional);

  if (vm.count(help_str) or unrecognized.size() > 0)
  {
    if (unrecognized.size() > 0)
    {
      std::stringstream ss;
      std::copy( unrecognized.cbegin(), unrecognized.cend()
               , std::ostream_iterator<std::string>(ss, " "));
      throw po::error("Unknown option(s): " + ss.str());
    }

    std::cout << "Usage: " << argv[0] << " [options] infile outfile" << std::endl << std::endl;
    std::cout << pn_input_options() << std::endl;
    return boost::optional<configuration>();
  }

  if (not vm.count("input-file"))
  {
    throw po::error("No input file specified.");
  }

  if (not vm.count("output-file"))
  {
    throw po::error("No output file specified.");
  }

  conf.input = configure_pn_parser(vm);
  conf.tina_output_file = util::file(vm["output-file"].as<std::string>());

  return conf;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
