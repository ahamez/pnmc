#include <iostream>
#include <string>

#pragma GCC diagnostic push
#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <boost/program_options.hpp>
#pragma GCC diagnostic pop

#include "conf/configuration.hh"
#include "conf/fill_configuration.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

namespace po = boost::program_options;

const std::string version
  = "Petri Net Model Checker (built " + std::string(__DATE__) + " " + std::string(__TIME__)  + ")";

/*------------------------------------------------------------------------------------------------*/

input_format
file_type(const po::variables_map& vm)
{
  const bool bpn = vm.count("bpn");
  const bool prod = vm.count("prod");
  const bool tina = vm.count("tina");

  if (not (bpn or prod or tina))
  {
    return input_format::xml;
  }
  else if (not (bpn xor prod xor tina))
  {
    throw po::error("Can specify only one input format.");
  }
  else
  {
    if (bpn)
    {
      return input_format::bpn;
    }
    else if (prod)
    {
      return input_format::prod;
    }
    else // tina
    {
      return input_format::tina;
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

boost::optional<pnmc_configuration>
fill_configuration(int argc, char** argv)
{
  pnmc_configuration conf;

  po::options_description general_options("General options");
  general_options.add_options()
    ( "help"                  , "Show this help")
    ( "version"               , "Show version")
  ;

  po::options_description file_options("Input file options");
  file_options.add_options()
    ("bpn"                      , "Parse BPN format")
    ("prod"                     , "Parse PROD format")
    ("tina"                     , "Parse TINA format")
    ("xml"                      , "Parse pnmc's XML format (default)")
  ;

  po::options_description order_options("Order options");
  order_options.add_options()
    ("order-show"             , "Show the order")
    ("order-random"           , "Random order")
    ("order-flat"             , "Don't use hierarchy informations")
    ("order-min-height"       , po::value<unsigned int>()->default_value(10)
                              , "Minimal number of variables at every level of the SDD")
  ;

  po::options_description hom_options("Homomorphisms options");
  hom_options.add_options()
    ("relation-show"          , "Show the transition relation")
  ;

  po::options_description stats_options("Statistics options");
  stats_options.add_options()
    ("show-hash-stats"          , "Show the hash tables statistics")
    ("show-time"                , "Show miscellaneous execution times")
  ;

  po::options_description petri_options("Petri net options");
  petri_options.add_options()
    ("dead-transitions"         , "Compute dead transitions")
    ("dead-states"              , "Compute dead states")
    ("marking-bound"            , po::value<unsigned int>()->default_value(0)
                                , "Limit the marking")
  ;


  po::options_description hidden_options("Hidden options");
  hidden_options.add_options()
    ("export-to-lua", po::value<std::string>(), "Export the final SDD to a Lua structure")
    ("input-file", po::value<std::string>(), "The Petri net file to analyse")
    ("delete-file", "Delete model file after reading it")
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
    return boost::optional<pnmc_configuration>();
  }
  
  if (vm.count("version"))
  {
    std::cout << version << std::endl;
    return boost::optional<pnmc_configuration>();
  }

  if (not vm.count("input-file"))
  {
    throw po::error("No file specified.");
  }

  conf.file_name = vm["input-file"].as<std::string>();
  conf.file_type = file_type(vm);
  conf.read_stdin = conf.file_name == "-";
  conf.delete_file = vm.count("delete-file");
  conf.order_show = vm.count("order-show");
  conf.order_random = vm.count("order-random");
  conf.order_force_flat = vm.count("order-flat");
  conf.order_min_height = vm["order-min-height"].as<unsigned int>();
  conf.show_relation = vm.count("relation-show");
  conf.show_hash_tables_stats = vm.count("show-hash-stats");
  conf.show_time = vm.count("show-time");
  conf.compute_dead_transitions = vm.count("dead-transitions");
  conf.compute_dead_states = vm.count("dead-states");
  conf.export_to_lua = vm.count("export-to-lua");
  conf.marking_bound = vm["marking-bound"].as<unsigned int>();
  if (conf.export_to_lua)
  {
    conf.export_to_lua_file = vm["export-to-lua"].as<std::string>();
  }

  return conf;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
