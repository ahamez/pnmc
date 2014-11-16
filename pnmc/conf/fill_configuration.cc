#include <algorithm> // copy, transform
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/algorithm/string/join.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/tokenizer.hpp>

#include "conf/configuration.hh"
#include "conf/fill_configuration.hh"
#include "support/conf/options.hh"
#include "support/util/paths.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

namespace po = boost::program_options;
using namespace std::string_literals;

const std::string version
  = "Petri Net Model Checker (built " + std::string(__DATE__) + " " + std::string(__TIME__)  + ")";

/*------------------------------------------------------------------------------------------------*/

// General options
const auto help_str = "help";
const auto help_exp_str = "help-exp";
const auto version_str = "version";
const auto conf_str = "conf";
const auto output_dir_str = "output-dir";

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

// Statistics options
const auto show_final_sdd_bytes_str = "show-final-sdd-bytes";

// Petri net options
const auto pn_marking_bound_str = "marking-bound";
const auto pn_one_safe_str = "1-safe";

// Model checking options
const auto mc_dead_transitions_str = "dead-transitions";
const auto mc_dead_states_str = "dead-states";
const auto mc_count_tokens_str = "count-tokens";

// libsdd options
const auto libsdd_sdd_ut_size_str = "sdd-ut-size";
const auto libsdd_hom_ut_size_str = "hom-ut-size";

// Advanced options
const auto limit_time_str = "time-limit";
const auto json_str = "json";
const auto final_sdd_stats_str = "final-sdd-stats";
const auto results_json_str = "results-json";
const auto show_time_str = "show-time";
const auto fast_exit_str = "fast-exit";
const auto sample_nb_sdd_str = "sample-nb-sdd";
const auto pn_stats_str = "pn-stats";
const auto hom_json_export_str = "hom-json";

/*------------------------------------------------------------------------------------------------*/

// Caches size
const auto cache_str = "cache";
static const auto default_cache_sizes = std::map<std::string, std::size_t>
  { {"hom" , 8'000'000}
  , {"diff", 2'000'000}
  , {"inter", 2'000'000}
  , {"sum", 2'000'000}};
static const auto cache_values = [&]
{
  using namespace boost::adaptors;
  return "["s + boost::algorithm::join(default_cache_sizes | map_keys, "|") + "]+"s;
}();

/*------------------------------------------------------------------------------------------------*/

// DOT export
const auto dot_export_str = "dot";
static const auto dot_export_values_map = std::map<std::string, mc::shared::dot_export>
  { {"force" , mc::shared::dot_export::force}
  , {"hom"   , mc::shared::dot_export::hom}
  , {"sdd"   , mc::shared::dot_export::sdd}};
static const auto dot_export_values = [&]
{
  using namespace boost::adaptors;
  return "["s + boost::algorithm::join(dot_export_values_map | map_keys, "|") + "]+"s;
}();

/*------------------------------------------------------------------------------------------------*/

boost::optional<configuration>
fill_configuration(int argc, const char** argv)
{
  configuration conf;

  po::options_description general_options("General options");
  general_options.add_options()
    (help_str       , "Show this help")
    (help_exp_str   , "Show experimental/dev features help")
    (version_str    , "Show version")
    (conf_str       , po::value<std::string>()
                    , "Configure pnmc with a file")
    (output_dir_str , po::value<std::string>()
                    , "Output directory path")
  ;

  po::options_description order_options("Order options");
  order_options.add_options()
    (order_show_str             , "Show order")
    (order_flat_str             , "Don't use hierarchy informations")
    (order_force_str            , "Use FORCE ordering heuristic")
  ;

  po::options_description stats_options("Statistics options");
  stats_options.add_options()
    (show_time_str              , "Show a breakdown of all steps' times")
    (json_str                   , po::value<std::string>()
                                , "Export pnmc's statistics to a JSON file")
  ;

  po::options_description petri_options("Petri net options");
  petri_options.add_options()
    (pn_marking_bound_str      , po::value<unsigned int>()->default_value(0)
                               , "Limit marking")
    (pn_one_safe_str           , "Optimize for 1-safe Petri nets")
  ;

  po::options_description mc_options("Model checking options");
  mc_options.add_options()
    (mc_dead_transitions_str   , "Compute dead transitions")
    (mc_dead_states_str        , "Compute dead states")
    (mc_count_tokens_str       , "Compute maximal markings")
    (results_json_str          , po::value<std::string>()
                               , "Export pnmc's results to a JSON file")
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
    (hom_json_export_str        , po::value<std::string>()
                                , "Export homomorphism to a JSON file")
    (dot_export_str             , po::value<std::string>()
                                , dot_export_values.c_str())
    (order_load_str             , po::value<std::string>()
                                , "Load order from a JSON file")
    (sample_nb_sdd_str          , "Sample the number of SDD regularly")
    (final_sdd_stats_str        , "Export final SDD's statistics (may be costly) to JSON file")
    (pn_stats_str               , "Export model's statistics to JSON file")
    (cache_str                  , po::value<std::string>()
                                , cache_values.c_str())
  ;

  po::options_description hidden_libsdd_options("Hidden libsdd options");
  hidden_libsdd_options.add_options()
    (libsdd_sdd_ut_size_str          , po::value<unsigned int>()->default_value(2000000))
    (libsdd_hom_ut_size_str          , po::value<unsigned int>()->default_value(25000))
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
    .add(input_options())
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
      std::stringstream ss;
      std::copy( unrecognized.cbegin(), unrecognized.cend()
               , std::ostream_iterator<std::string>(ss, " "));
      throw po::error("Unknown option(s): " + ss.str());
    }

    std::cout << version << std::endl;
    std::cout << "Usage: " << argv[0] << " [options] file " << std::endl << std::endl;
    std::cout << general_options << std::endl;
    std::cout << input_options() << std::endl;
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
  conf.input = configure_parser(vm);

  // Order options
  conf.order_show = vm.count(order_show_str);
  conf.order_random = vm.count(order_random_str);
  conf.order_flat = vm.count(order_flat_str);
  conf.order_ordering_force = vm.count(order_force_str);
  conf.order_force_iterations = vm[order_force_iterations_str].as<unsigned int>();
  conf.order_only = vm.count(order_only_str);
  conf.order_reverse = vm.count(order_reverse_str);
  conf.order_id_per_hierarchy = vm[order_id_per_hier_str].as<unsigned int>();

  // Statistics options
  conf.show_final_sdd_bytes = vm.count(show_final_sdd_bytes_str);

  // Petri net options
  conf.marking_bound = vm[pn_marking_bound_str].as<unsigned int>();
  conf.one_safe = vm.count(pn_one_safe_str);

  // Hidden libsdd options
  conf.sdd_ut_size= vm[libsdd_sdd_ut_size_str].as<unsigned int>();
  conf.hom_ut_size = vm[libsdd_hom_ut_size_str].as<unsigned int>();;

  // Model checking options
  conf.compute_dead_states = vm.count(mc_dead_states_str);
  conf.compute_dead_transitions = vm.count(mc_dead_transitions_str);
  conf.count_tokens = vm.count(mc_count_tokens_str);

  // Advanced options
  conf.show_time = vm.count(show_time_str);
  conf.fast_exit = vm.count(fast_exit_str);
  conf.sample_nb_sdd = vm.count(sample_nb_sdd_str);
  conf.final_sdd_statistics = vm.count(final_sdd_stats_str);
  conf.pn_statistics = vm.count(pn_stats_str);
  if (vm.count(json_str))
  {
    conf.json_file = util::file(vm[json_str].as<std::string>());
  }
  if (vm.count(results_json_str))
  {
    conf.results_json_file = util::file(vm[results_json_str].as<std::string>());
  }
  if (conf.order_ordering_force)
  {
    conf.order_flat = true;
  }
  conf.max_time = std::chrono::duration<double>(vm[limit_time_str].as<unsigned int>());
  if (vm.count(order_load_str))
  {
    conf.order_file = util::in_file(vm[order_load_str].as<std::string>());
  }

  if (vm.count(output_dir_str))
  {
    conf.output_dir = util::file(vm[output_dir_str].as<std::string>());
  }
  else
  {
    conf.output_dir = boost::filesystem::current_path();
  }

  if (vm.count(dot_export_str))
  {
    using separator = boost::char_separator<char>;
    boost::tokenizer<separator> tks{vm[dot_export_str].as<std::string>(), separator{","}};
    std::transform( begin(tks), end(tks), std::inserter(conf.dot_dump, end(conf.dot_dump))
                  , [](const auto& s)
                      {
                        const auto search = dot_export_values_map.find(s);
                        if (search == end(dot_export_values_map))
                        {
                          throw po::error("Invalid "s + dot_export_str + " option value: "s + s);
                        }
                        return search->second;
                      });
  }

  conf.cache_sizes = default_cache_sizes;
  if (vm.count(cache_str))
  {
    using separator = boost::char_separator<char>;
    boost::tokenizer<separator> tks{vm[cache_str].as<std::string>(), separator{","}};
    for (const auto& tk : tks)
    {
      std::vector<std::string> tmp;
      boost::tokenizer<separator> subtks{tk, separator{":"}};
      std::copy(begin(subtks), end(subtks), std::back_inserter(tmp));
      if (tmp.size() != 2)
      {
        throw po::error("Invalid "s + cache_str + " option value: "s + tk);
      }
      const auto search = conf.cache_sizes.find(tmp[0]);
      if (search == end(conf.cache_sizes))
      {
        throw po::error("Invalid "s + cache_str + " option value: "s + tk);
      }
      try
      {
        search->second = std::stoul(tmp[1]);
      }
      catch (std::invalid_argument&)
      {
        throw po::error("Invalid "s + cache_str + " option value: "s + tk);
      }
    }
  }

  return conf;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
