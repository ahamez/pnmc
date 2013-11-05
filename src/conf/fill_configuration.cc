#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "conf/configuration.hh"
#include "conf/fill_configuration.hh"

namespace boost {

namespace po = boost::program_options;

void validate(boost::any& v, const std::vector<std::string>& values, pnmc::conf::input_format*, int)
{
  // Make sure no previous assignment to 'v' was made.
  po::validators::check_first_occurrence(v);

  // Extract the first string from 'values'. If there is more than
  // one string, it's an error, and exception will be thrown.
  const std::string& s = po::validators::get_single_string(values);

  if (s == "bpn")
  {
    v = boost::any(pnmc::conf::input_format::bpn);
  }
  else if (s == "prod")
  {
    v = boost::any(pnmc::conf::input_format::prod);
  }
  else if (s == "tina")
  {
    v = boost::any(pnmc::conf::input_format::tina);
  }
  else if (s == "xml")
  {
    v = boost::any(pnmc::conf::input_format::xml);
  }
  else
  {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
}

} // namespace boost

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

namespace po = boost::program_options;

const std::string version
  = "Petri Net Model Checker (built " + std::string(__DATE__) + " " + std::string(__TIME__)  + ")";

/*------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/

boost::optional<pnmc_configuration>
fill_configuration(int argc, char** argv)
{
  typedef boost::optional<pnmc_configuration> empty_result;

  pnmc_configuration conf;

  po::options_description general_options("General options");
  general_options.add_options()
    ( "help"                  , "Show this help")
    ( "version"               , "Show version")
    ( "show-configuration"    , "Show the configuration")
  ;

  po::options_description file_options("Input file options");
  file_options.add_options()
    ( "input-format"          , po::value<input_format>()->default_value(input_format::xml, "xml")
                              , "The file format: [bpn|prod|tina|xml]")
  ;

  po::options_description order_options("Order options");
  order_options.add_options()
    ("show-order"             , "Show the order")
  ;

  po::options_description hom_options("Homomorphisms options");
  hom_options.add_options()
    ("show-relation"          , "Show the transition relation")
  ;

  po::options_description stats_options("Statistics options");
  stats_options.add_options()
    ("show-hash-stats"          , "Show the hash tables statistics")
    ("show-time"                , "Show miscellaneous execution times")
  ;

  po::options_description petri_options("Petri net options");
  petri_options.add_options()
    ("dead-transitions"         , "Compute dead transitions")
  ;


  po::options_description hidden_options("Hidden options");
  hidden_options.add_options()
    ("input-file"             , po::value<std::string>(), "The Petri net file to analyse")
  ;

  po::positional_options_description p;
  p.add("input-file", 1);
  
  po::options_description cmdline_options;
  cmdline_options
  	.add(general_options)
    .add(file_options)
    .add(order_options)
    .add(hom_options)
    .add(petri_options)
    .add(stats_options)
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
    std::cout << file_options << std::endl;
    std::cout << order_options << std::endl;
    std::cout << hom_options << std::endl;
    std::cout << petri_options << std::endl;
    std::cout << stats_options << std::endl;
    return empty_result();
  }
  
  if (vm.count("version"))
  {
    std::cout << version << std::endl;
    return empty_result();
  }

  if (not vm.count("input-file"))
  {
    std::cerr << "No file specified." << std::endl;
    return empty_result();
  }

  conf.file_name = vm["input-file"].as<std::string>();
  conf.file_type = vm["input-format"].as<input_format>();
  conf.read_stdin = conf.file_name == "-";
  conf.show_order = vm.count("show-order");
  conf.show_relation = vm.count("show-relation");
  conf.show_hash_tables_stats = vm.count("show-hash-stats");
  conf.show_time = vm.count("show-time");
  conf.compute_dead_transitions = vm.count("dead-transitions");

  if (vm.count("show-configuration"))
  {
    std::cout << conf << std::endl;
  }
  
  return conf;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
