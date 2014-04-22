#include <fstream>
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
const auto bpn_str  = "bpn";
const auto pnml_str = "pnml";
const auto prod_str = "prod";
const auto tina_str = "tina";
const auto xml_str  = "xml";

input_format
file_type(const po::variables_map& vm)
{
  const bool bpn  = vm.count(bpn_str);
  const bool pnml = vm.count(pnml_str);
  const bool prod = vm.count(prod_str);
  const bool tina = vm.count(tina_str);

  if (not (bpn or pnml or prod or tina))
  {
    return input_format::xml;
  }
  else if (not (bpn xor pnml xor prod xor tina))
  {
    throw po::error("Can specify only one input format.");
  }
  else
  {
    if (bpn)
    {
      return input_format::bpn;
    }
    else if (pnml)
    {
      return input_format::pnml;
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
const auto help_exp_str = "help-exp";
const auto version_str = "version";
const auto conf_str = "conf";

// Order options
const auto order_show_str = "order-show";
const auto order_random_str = "order-random";
const auto order_flat_str = "order-flat";
const auto order_force_str = "order-force";
const auto order_force_iterations_str = "order-force-iterations";
const auto order_only_str = "order-only";
const auto order_reverse_str = "order-reverse";
const auto order_id_per_hier_str = "order-ids-per-hierarchy";
const auto order_load_str = "order-load";

// Homomorphisms options
const auto hom_show_relation_str = "hom-show-relation";

// Statistics options
const auto show_final_sdd_bytes_str = "show-final-sdd-bytes";

// Petri net options
const auto pn_marking_bound_str = "pn-marking-bound";

// Model checking options
const auto mc_dead_transitions_str = "dead-transitions";
const auto mc_dead_states_str = "dead-states";

// libsdd options
const auto libsdd_sdd_ut_size_str = "sdd-ut-size";
const auto libsdd_sdd_diff_cache_size_str = "sdd-diff-cache-size";
const auto libsdd_sdd_inter_cache_size_str = "sdd-inter-cache-size";
const auto libsdd_sdd_sum_cache_size_str = "sdd-sum-size-cache";
const auto libsdd_hom_ut_size_str = "hom-ut-size";
const auto libsdd_hom_cache_size_str = "hom-cache-size";

// Advanced options
const auto limit_time_str = "time-limit";
const auto delete_input_file_str = "delete-input-file";
const auto export_to_lua_str = "export-lua";
const auto final_sdd_dot_export_str = "final-sdd-dot";
const auto json_str = "json";
const auto results_json_str = "results-json";
const auto show_time_str = "show-time";
const auto hypergraph_dot_str = "hypergraph-force-dot";
const auto fast_exit_str = "fast-exit";
const auto hom_dot_export_str = "hom-dot";
const auto hom_sat_dot_export_str = "sat-hom-dot";
const auto export_tina_str = "export-tina";

boost::optional<configuration>
fill_configuration(int argc, char** argv)
{
  configuration conf;

  po::options_description general_options("General options");
  general_options.add_options()
    (help_str     , "Show this help")
    (help_exp_str , "Show experimental/dev features help")
    (version_str  , "Show version")
    (conf_str     , po::value<std::string>()
                  , "Configure PNMC with a file")
  ;

  po::options_description file_options("Input file format options");
  file_options.add_options()
    (bpn_str  , "Parse BPN format")
    (pnml_str , "Parse PNML format")
    (prod_str , "Parse PROD format")
    (tina_str , "Parse TINA format")
    (xml_str  , "Parse pnmc's XML format (default)")
  ;

  po::options_description order_options("Order options");
  order_options.add_options()
    (order_show_str             , "Show the order")
    (order_flat_str             , "Don't use hierarchy informations")
    (order_force_str            , "Use the FORCE ordering heuristic")
  ;

  po::options_description stats_options("Statistics options");
  stats_options.add_options()
    (show_time_str              , "Show a breakdown of all steps' times")
    (json_str                   , po::value<std::string>()
                                , "Export PNMC's statistics to a JSON file")
  ;

  po::options_description petri_options("Petri net options");
  petri_options.add_options()
    (pn_marking_bound_str      , po::value<unsigned int>()->default_value(0)
                               , "Limit the marking")
  ;

  po::options_description mc_options("Model checking options");
  mc_options.add_options()
    (mc_dead_transitions_str   , "Compute dead transitions")
    (mc_dead_states_str        , "Compute dead states")
    (results_json_str          , po::value<std::string>()
                               , "Export PNMC's results to a JSON file")
  ;

  po::options_description hidden_exp_options("Hidden dev/experimental options");
  hidden_exp_options.add_options()
    (order_random_str           , "Random order")
    (order_force_iterations_str , po::value<unsigned int>()->default_value(100)
                                , "Number of FORCE iterations")
    (order_reverse_str          , "Reverse order (depends on on strategy)")
    (order_id_per_hier_str      , po::value<unsigned int>()->default_value(0)
                                , "Number of identifiers per hierarchy")
    (order_only_str             , "Compute order only")
    (show_final_sdd_bytes_str   , "Show the number of bytes used by the final state space's SDD")
    (delete_input_file_str      , "Delete input file after reading it")
    (export_to_lua_str          , po::value<std::string>()
                                , "Export the final SDD to a Lua structure")
    (final_sdd_dot_export_str   , po::value<std::string>()
                                , "Export the SDD state space to a DOT file")
    (hypergraph_dot_str         , po::value<std::string>()
                                , "Export FORCE's hypergraph to a DOT file")
    (hom_dot_export_str         , po::value<std::string>()
                                , "Export homomorphism to a DOT file")
    (hom_sat_dot_export_str     , po::value<std::string>()
                                , "Export saturated homomorphism to a DOT file")
    (hom_show_relation_str      , "Show the transition relation")
    (export_tina_str            , po::value<std::string>()
                                , "Export Petri net to a TINA file")
    (order_load_str             , po::value<std::string>()
                                , "Load order from a JSON file")
  ;

  po::options_description hidden_libsdd_options("Hidden libsdd options");
  hidden_libsdd_options.add_options()
    (libsdd_sdd_ut_size_str          , po::value<unsigned int>()->default_value(2000000))
    (libsdd_sdd_diff_cache_size_str  , po::value<unsigned int>()->default_value(500000))
    (libsdd_sdd_inter_cache_size_str , po::value<unsigned int>()->default_value(500000))
    (libsdd_sdd_sum_cache_size_str   , po::value<unsigned int>()->default_value(2000000))
    (libsdd_hom_ut_size_str          , po::value<unsigned int>()->default_value(25000))
    (libsdd_hom_cache_size_str       , po::value<unsigned int>()->default_value(2000000))
  ;

  po::options_description hidden_options("Hidden options");
  hidden_options.add_options()
    ("input-file"               , po::value<std::string>()
                                , "The Petri net file to analyse")
  ;

  po::options_description advanced_options("Advanced options");
  advanced_options.add_options()
    (limit_time_str             , po::value<unsigned int>()->default_value(0)
                                , "Limit the execution time (s)")
    (fast_exit_str              , "Don't cleanup memory on exit")
  ;

  po::positional_options_description p;
  p.add("input-file", 1);
  
  po::options_description cmdline_options;
  cmdline_options
  	.add(general_options)
    .add(file_options)
    .add(order_options)
    .add(petri_options)
    .add(mc_options)
    .add(hidden_exp_options)
    .add(hidden_libsdd_options)
    .add(stats_options)
    .add(advanced_options)
    .add(hidden_options);

  po::options_description config_file_options;
  config_file_options
    .add(order_options)
    .add(petri_options)
    .add(mc_options)
    .add(hidden_exp_options)
    .add(hidden_libsdd_options)
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
    std::cout << petri_options << std::endl;
    std::cout << mc_options << std::endl;
    std::cout << stats_options << std::endl;
    std::cout << advanced_options << std::endl;
    return boost::optional<configuration>();
  }
  else if (vm.count(help_exp_str))
  {
    std::cout << hidden_exp_options << std::endl;
    std::cout << hidden_libsdd_options << std::endl;
    return boost::optional<configuration>();
  }

  if (vm.count(version_str))
  {
    std::cout << version << std::endl;
    return boost::optional<configuration>();
  }

  if (not vm.count("input-file"))
  {
    throw po::error("No file specified.");
  }

  if (vm.count(conf_str))
  {
    const auto file_name = vm[conf_str].as<std::string>();
    std::ifstream file(file_name);
    if (file.is_open())
    {
      store(parse_config_file(file, config_file_options), vm);
      notify(vm);
    }
    else
    {
      throw po::error("Cannot open configuration file " + file_name);
    }
  }

  // Input options
  conf.file_name = vm["input-file"].as<std::string>();
  conf.file_type = file_type(vm);
  conf.read_stdin = conf.file_name == "-";

  // Order options
  conf.order_show = vm.count(order_show_str);
  conf.order_random = vm.count(order_random_str);
  conf.order_flat = vm.count(order_flat_str);
  conf.order_ordering_force = vm.count(order_force_str);
  conf.order_force_iterations = vm[order_force_iterations_str].as<unsigned int>();
  conf.order_only = vm.count(order_only_str);
  conf.order_reverse = vm.count(order_reverse_str);
  conf.order_id_per_hierarchy = vm[order_id_per_hier_str].as<unsigned int>();

  // Hom options
  conf.show_relation = vm.count(hom_show_relation_str);

  // Statistics options
  conf.show_final_sdd_bytes = vm.count(show_final_sdd_bytes_str);

  // Petri net options
  conf.marking_bound = vm[pn_marking_bound_str].as<unsigned int>();

  // Hidden libsdd options
  conf.sdd_ut_size= vm[libsdd_sdd_ut_size_str].as<unsigned int>();
  conf.sdd_diff_cache_size = vm[libsdd_sdd_diff_cache_size_str].as<unsigned int>();;
  conf.sdd_inter_cache_size = vm[libsdd_sdd_inter_cache_size_str].as<unsigned int>();;
  conf.sdd_sum_cache_size = vm[libsdd_sdd_sum_cache_size_str].as<unsigned int>();;
  conf.hom_ut_size = vm[libsdd_hom_ut_size_str].as<unsigned int>();;
  conf.hom_cache_size = vm[libsdd_hom_cache_size_str].as<unsigned int>();;

  // Model checking options
  conf.compute_dead_states = vm.count(mc_dead_states_str);
  conf.compute_dead_transitions = vm.count(mc_dead_transitions_str);

  // Advanced options
  conf.delete_file = vm.count(delete_input_file_str);
  conf.show_time = vm.count(show_time_str);
  conf.fast_exit = vm.count(fast_exit_str);
  if (vm.count(export_to_lua_str))
  {
    conf.export_to_lua_file = vm[export_to_lua_str].as<std::string>();
  }
  if (vm.count(final_sdd_dot_export_str))
  {
    conf.export_final_sdd_dot_file = vm[final_sdd_dot_export_str].as<std::string>();
  }
  if (vm.count(json_str))
  {
    conf.json_file = vm[json_str].as<std::string>();
  }
  if (vm.count(results_json_str))
  {
    conf.results_json_file = vm[results_json_str].as<std::string>();
  }
  if (conf.order_ordering_force)
  {
    conf.order_flat = true;
  }
  conf.max_time = std::chrono::duration<double>(vm[limit_time_str].as<unsigned int>());
  if (vm.count(hypergraph_dot_str))
  {
    conf.hypergraph_dot_file = vm[hypergraph_dot_str].as<std::string>();
  }
  if (vm.count(hom_dot_export_str))
  {
    conf.export_hom_to_dot_file = vm[hom_dot_export_str].as<std::string>();
  }
  if (vm.count(hom_sat_dot_export_str))
  {
    conf.export_sat_hom_to_dot_file = vm[hom_sat_dot_export_str].as<std::string>();
  }
  if (vm.count(export_tina_str))
  {
    conf.export_tina_file = vm[export_tina_str].as<std::string>();
  }
  if (vm.count(order_load_str))
  {
    conf.load_order_file = vm[order_load_str].as<std::string>();
  }
  return conf;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
