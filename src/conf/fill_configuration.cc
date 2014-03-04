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

// Input format options.
const auto bpn_str = "bpn";
const auto prod_str = "prod";
const auto tina_str = "tina";
const auto xml_str = "xml";

input_format
file_type(const po::variables_map& vm)
{
  const bool bpn = vm.count(bpn_str);
  const bool prod = vm.count(prod_str);
  const bool tina = vm.count(tina_str);

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

// General options
const auto help_str = "help";
const auto version_str = "version";

// Order options
const auto order_show_str = "order-show";
const auto order_random_str = "order-random";
const auto order_flat_str = "order-flat";
const auto order_min_height_str = "order-min-height";
const auto order_force_str = "order-force-heuristic";

// Homomorphisms options
const auto hom_show_relation_str = "hom-show-relation";

// Statistics options
const auto show_final_sdd_bytes_str = "show-final-sdd-bytes";

// Petri net options
const auto pn_marking_bound_str = "pn-marking-bound";

// Model checking options
const auto mc_dead_transitions_str = "dead-transitions";
const auto mc_dead_states_str = "dead-states";

// Advanced options
const auto limit_time_str = "limit-time";
const auto delete_input_file_str = "delete-input-file";
const auto export_to_lua_str = "export-to-lua";
const auto final_sdd_dot_export_str = "final-sdd-dot";
const auto json_str = "json";
const auto show_time_str = "show-time";

boost::optional<pnmc_configuration>
fill_configuration(int argc, char** argv)
{
  pnmc_configuration conf;

  po::options_description general_options("General options");
  general_options.add_options()
    (help_str     , "Show this help")
    (version_str  , "Show version")
  ;

  po::options_description file_options("Input file options");
  file_options.add_options()
    (bpn_str  , "Parse BPN format")
    (prod_str , "Parse PROD format")
    (tina_str , "Parse TINA format")
    (xml_str  , "Parse pnmc's XML format (default)")
  ;

  po::options_description order_options("Order options");
  order_options.add_options()
    (order_show_str             , "Show the order")
    (order_random_str           , "Random order (not recommanded)")
    (order_flat_str             , "Don't use hierarchy informations")
    (order_min_height_str       , po::value<unsigned int>()->default_value(10)
                                , "Minimal number of variables at every level of the SDD")
  ;

  po::options_description hom_options("Homomorphisms options");
  hom_options.add_options()
    (hom_show_relation_str      , "Show the transition relation")
  ;

  po::options_description stats_options("Statistics options");
  stats_options.add_options()
    (show_final_sdd_bytes_str     , "Show the number of bytes used by the final state space's SDD")
  ;

  po::options_description petri_options("Petri net options");
  petri_options.add_options()
    (pn_marking_bound_str      , po::value<unsigned int>()->default_value(0)
                               , "Limit the marking")
  ;

  po::options_description mc_options("Model checking options");
  mc_options.add_options()
    (mc_dead_transitions_str      , "Compute dead transitions")
    (mc_dead_states_str           , "Compute dead states")
  ;

  po::options_description hidden_options("Hidden options");
  hidden_options.add_options()
    ("input-file", po::value<std::string>(), "The Petri net file to analyse")
  ;

  po::options_description advanced_options("Advanced options");
  advanced_options.add_options()
    (delete_input_file_str      , "Delete input file after reading it")
    (export_to_lua_str          , po::value<std::string>()
                                , "Export the final SDD to a Lua structure")
    (final_sdd_dot_export_str   , po::value<std::string>()
                                , "Export the SDD state space to DOT file")
    (json_str                   , po::value<std::string>()
                                , "Export PNMC's statistics to a JSON file")
    (show_time_str              , "Show a breakdown of all steps' times")
    (limit_time_str             , po::value<unsigned int>()->default_value(0)
                                , "Limit the execution time (s)")
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
    .add(mc_options)
    .add(stats_options)
    .add(advanced_options)
    .add(hidden_options);
  
  po::variables_map vm;
  po::parsed_options parsed
    = po::command_line_parser(argc, argv).options(cmdline_options)
                                         .positional(p)
                                         .allow_unregistered()
                                         .run();
  po::store(parsed, vm);
  po::notify(vm);
  
  std::vector<std::string> unrecognized
    = po::collect_unrecognized(parsed.options, po::exclude_positional);
  
  if (vm.count(help_str) or unrecognized.size() > 0)
  {
    if (unrecognized.size() > 0)
    {
      std::cout << "Unknown option(s):";
      std::copy( unrecognized.cbegin(), unrecognized.cend()
               , std::ostream_iterator<std::string>(std::cout, " "));
      std::cout << std::endl << std::endl;
    }

    std::cout << version << std::endl;
    std::cout << "Usage: " << argv[0] << " [options] file " << std::endl << std::endl;
    std::cout << general_options << std::endl;
    std::cout << file_options << std::endl;
    std::cout << order_options << std::endl;
    std::cout << hom_options << std::endl;
    std::cout << petri_options << std::endl;
    std::cout << mc_options << std::endl;
    std::cout << stats_options << std::endl;
    std::cout << advanced_options << std::endl;
    return boost::optional<pnmc_configuration>();
  }
  
  if (vm.count(version_str))
  {
    std::cout << version << std::endl;
    return boost::optional<pnmc_configuration>();
  }

  if (not vm.count("input-file"))
  {
    throw po::error("No file specified.");
  }

  // Input options
  conf.file_name = vm["input-file"].as<std::string>();
  conf.file_type = file_type(vm);
  conf.read_stdin = conf.file_name == "-";

  // Order options
  conf.order_show = vm.count(order_show_str);
  conf.order_random = vm.count(order_random_str);
  conf.order_force_flat = vm.count(order_flat_str);
  conf.order_min_height = vm[order_min_height_str].as<unsigned int>();

  // Hom options
  conf.show_relation = vm.count(hom_show_relation_str);

  // Statistics options
  conf.show_final_sdd_bytes = vm.count(show_final_sdd_bytes_str);

  // Petri net options
  conf.marking_bound = vm[pn_marking_bound_str].as<unsigned int>();

  // Model checking options
  conf.compute_dead_states = vm.count(mc_dead_states_str);
  conf.compute_dead_transitions = vm.count(mc_dead_transitions_str);

  // Advanced options
  conf.delete_file = vm.count(delete_input_file_str);
  conf.export_to_lua = vm.count(export_to_lua_str);
  conf.show_time = vm.count(show_time_str);
  if (conf.export_to_lua)
  {
    conf.export_to_lua_file = vm[export_to_lua_str].as<std::string>();
  }
  conf.export_final_sdd_dot = vm.count(final_sdd_dot_export_str);
  if (conf.export_final_sdd_dot)
  {
    conf.export_final_sdd_dot_file = vm[final_sdd_dot_export_str].as<std::string>();
  }
  conf.json = vm.count(json_str);
  if (conf.json)
  {
    conf.json_file = vm[json_str].as<std::string>();
  }
  conf.max_time = std::chrono::duration<double>(vm[limit_time_str].as<unsigned int>());

  return conf;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
