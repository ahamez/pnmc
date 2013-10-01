#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "conf/configuration.hh"
#include "conf/fill_configuration.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

namespace po = boost::program_options;

const std::string version = "Petri Net Model Checker v__DATE__ (__TIME__)";

/*------------------------------------------------------------------------------------------------*/

boost::optional<pnmc_configuration>
fill_configuration(int argc, char** argv)
{
  typedef boost::optional<pnmc_configuration> empty_result;

  pnmc_configuration conf;

  po::options_description general_options("General options");
  general_options.add_options()
      ("help"                  , "Show this help")
      ("version"               , "Show version")
      ("show-configuration"    , "Show the configuration")
  ;

  po::options_description hidden_options("Hidden options");
  hidden_options.add_options()
      ("file"                  , po::value<std::string>()
                               , "The model's file to process")
  ;      
  
  po::positional_options_description p;
  p.add("file", -1);
  
  po::options_description cmdline_options;
  cmdline_options
  	.add(general_options)
  	.add(hidden_options);
  
  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(argc, argv)
                                    .options(cmdline_options)
                                    .positional(p)
                                    .allow_unregistered()
                                    .run();
  po::store(parsed, vm);
  po::notify(vm);
  
  std::vector<std::string> unrecognized
    = po::collect_unrecognized(parsed.options,po::exclude_positional);
  
  if (vm.count("help") or unrecognized.size() > 0)
  {
    std::cout << version << std::endl;
    std::cout << "Usage: " << argv[0] << " [options] file " << std::endl << std::endl;
    std::cout << general_options << std::endl;
    return empty_result();
  }
  
  if (vm.count("version"))
  {
    std::cout << version << std::endl;
    return empty_result();
  }

  if (not vm.count("file"))
  {
    std::cerr << "No file specified." << std::endl;
    return empty_result();
  }

  conf.file_name = vm["file"].as<std::string>();
  
  if (vm.count("show-configuration"))
  {
    std::cout << conf << std::endl;
  }
  
  return conf;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
