/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

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
#include "conf/default.hh"
#include "conf/fill_configuration.hh"
#include "support/conf/options.hh"
#include "support/util/paths.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

namespace po = boost::program_options;
using namespace boost::adaptors;

/*------------------------------------------------------------------------------------------------*/

const std::string version
  = "Petri Net Model Checker (built " + std::string(__DATE__) + " " + std::string(__TIME__)  + ")";

/*------------------------------------------------------------------------------------------------*/

// General options
const auto help_str = "help";
const auto help_exp_str = "help-exp";
const auto version_str = "version";
const auto conf_str = "conf";
const auto output_dir_str = "output-dir";

/*------------------------------------------------------------------------------------------------*/

// Order options
const auto order_flat_str = "order-flat";
const auto order_force_str = "order-force";
const auto order_lexical_str = "order-lexical";
const auto order_only_str = "order-only";
const auto order_force_iterations_str = "order-force-iterations";
const auto order_reverse_str = "order-reverse";
const auto order_id_per_hier_str = "order-ids-per-hierarchy";
const auto order_load_str = "order-load";

/*------------------------------------------------------------------------------------------------*/

// Petri net options
const auto pn_marking_bound_str = "marking-bound";
const auto pn_one_safe_str = "1-safe";

/*------------------------------------------------------------------------------------------------*/

// Model checking options
const auto mc_dead_transitions_str = "dead-transitions";
const auto mc_dead_states_str = "dead-states";
const auto mc_count_tokens_str = "count-tokens";
const auto mc_trace_str = "trace";

/*------------------------------------------------------------------------------------------------*/

// Advanced options
const auto time_limit_str = "time-limit";

/*------------------------------------------------------------------------------------------------*/

// Caches size
const auto cache_str = "cache-size";
static const auto cache_values = []
{
  return "([" + boost::algorithm::join(default_cache_sizes | map_keys, "|") + "]:value)+";
}();

/*------------------------------------------------------------------------------------------------*/

// Unicity tables size
const auto ut_str = "ut-size";
static const auto ut_values = []
{
  return "([" + boost::algorithm::join(default_ut_sizes | map_keys, "|") + "]:value)+";
}();

/*------------------------------------------------------------------------------------------------*/

// DOT export
const auto dot_export_str = "dot";
static const auto dot_export_values_map = std::map<std::string, mc::shared::dot_export>
  { {"force" , mc::shared::dot_export::force}
  , {"hom"   , mc::shared::dot_export::hom}
  , {"sdd"   , mc::shared::dot_export::sdd}};
static const auto dot_export_values_str = []
{
  return "[" + boost::algorithm::join(dot_export_values_map | map_keys, "|") + "]+";
}();

/*------------------------------------------------------------------------------------------------*/

// JSON export
const auto json_str = "json";
static const auto json_export_values_map = std::map<std::string, mc::shared::json_export>
  { {"hom"        , mc::shared::json_export::hom}
  , {"order"      , mc::shared::json_export::order}
  , {"order-cout" , mc::shared::json_export::order_cout}
  , {"results"    , mc::shared::json_export::results}
  , {"stats"      , mc::shared::json_export::stats}
  };
static const auto json_export_values_str = []
{
  return "[" + boost::algorithm::join(json_export_values_map | map_keys, "|") + "]+";
}();

/*------------------------------------------------------------------------------------------------*/

// Statistics
const auto stats_str = "stats";
static const auto stats_values_map = std::map<std::string, mc::shared::stats>
  { {"pn"        , mc::shared::stats::pn}
  , {"final-sdd" , mc::shared::stats::final_sdd}
  , {"nb-sdd"    , mc::shared::stats::nb_sdd}};
static const auto stats_values_str = []
{
  return "[" + boost::algorithm::join(stats_values_map | map_keys, "|") + "]+";
}();

/*------------------------------------------------------------------------------------------------*/

boost::optional<configuration>
fill_configuration(int argc, const char** argv)
{
  auto conf = configuration{};

  auto general_options = po::options_description{"General options"};
  general_options.add_options()
    (help_str       , "Show this help")
    (help_exp_str   , "Show experimental/dev features help")
    (version_str    , "Show version")
    (conf_str       , po::value<std::string>()
                    , "Configure pnmc with a file")
    (output_dir_str , po::value<std::string>()
                    , "Output directory path")
  ;

  auto hidden_input_options = po::options_description{"Hidden input options"};
  hidden_input_options.add_options()
    ("input-file"   , po::value<std::string>()
                    , "The Petri net file to analyse")
  ;

  auto export_options = po::options_description{"Export options"};
  export_options.add_options()
    (dot_export_str , po::value<std::string>()
                    , dot_export_values_str.c_str())
    (json_str       , po::value<std::string>()
                    , json_export_values_str.c_str())
  ;

  auto stats_options = po::options_description{"Statistics options"};
  stats_options.add_options()
    (stats_str      , po::value<std::string>()
                    , stats_values_str.c_str())
  ;

  auto order_options = po::options_description{"Order options"};
  order_options.add_options()
    (order_flat_str    , "Don't use hierarchy informations")
    (order_force_str   , "Use FORCE ordering heuristic")
    (order_lexical_str , "Sort variables using a lexical order on the place names")
    (order_only_str    , "Only compute an order")
  ;

  auto petri_options = po::options_description{"Petri net options"};
  petri_options.add_options()
    (pn_marking_bound_str      , po::value<unsigned int>()->default_value(0)
                               , "Limit marking")
    (pn_one_safe_str           , "Optimize for 1-safe Petri nets")
  ;

  auto mc_options = po::options_description{"Model checking options"};
  mc_options.add_options()
    (mc_dead_transitions_str   , "Compute dead transitions")
    (mc_dead_states_str        , "Compute dead states")
    (mc_count_tokens_str       , "Compute maximal markings")
  ;

  auto advanced_options = po::options_description{"Advanced options"};
  advanced_options.add_options()
    (time_limit_str             , po::value<unsigned int>()
                                , "Limit the execution time (s)")
    (cache_str                  , po::value<std::string>()
                                , cache_values.c_str())
    (ut_str                     , po::value<std::string>()
                                , ut_values.c_str())
  ;

  auto hidden_exp_options = po::options_description{"Hidden dev/experimental options"};
  hidden_exp_options.add_options()
    (order_force_iterations_str , po::value<unsigned int>()->default_value(100)
                                , "Number of FORCE iterations")
    (order_reverse_str          , "Reverse order (depends on on strategy)")
    (order_id_per_hier_str      , po::value<unsigned int>()->default_value(0)
                                , "Number of identifiers per hierarchy")
    (order_load_str             , po::value<std::string>()
                                , "Load order from a JSON file")
    (mc_trace_str               , "Compute the shortest trace")
  ;

  auto p = po::positional_options_description{};
  p.add("input-file", 1);
  
  auto cmdline_options = po::options_description{};
  cmdline_options
  	.add(general_options)
    .add(pn_input_options())
    .add(properties_input_options())
    .add(export_options)
    .add(order_options)
    .add(petri_options)
    .add(mc_options)
    .add(hidden_exp_options)
    .add(stats_options)
    .add(advanced_options)
    .add(hidden_input_options);

  auto config_file_options = po::options_description{};
  config_file_options
    .add(order_options)
    .add(export_options)
    .add(petri_options)
    .add(mc_options)
    .add(hidden_exp_options)
    .add(stats_options)
    .add(advanced_options)
    .add(hidden_input_options);

  auto vm = po::variables_map{};
  auto parsed = po::command_line_parser(argc, argv).options(cmdline_options)
                                                   .positional(p)
                                                   .allow_unregistered()
                                                   .run();
  po::store(parsed, vm);
  po::notify(vm);

  auto unrecognized = po::collect_unrecognized(parsed.options, po::exclude_positional);

  if (vm.count(help_exp_str))
  {
    std::cout << hidden_exp_options << '\n';
    return boost::optional<configuration>();
  }
  else if (vm.count(version_str))
  {
    std::cout << version << '\n';
    return boost::optional<configuration>();
  }
  else if (vm.count(help_str) or not unrecognized.empty() or not vm.count("input-file"))
  {
    if (not unrecognized.empty())
    {
      std::cerr << "Unknown option(s): " << boost::algorithm::join(unrecognized, " ") << "\n\n";
    }
    if (not vm.count("input-file"))
    {
      std::cerr << "No file specified\n\n";
    }

    std::cout << version << '\n'
              << "Usage: " << argv[0] << " [options] file\n\n"
              << general_options << '\n'
              << pn_input_options() << '\n'
              << properties_input_options() << '\n'
              << export_options << '\n'
              << order_options << '\n'
              << petri_options << '\n'
              << mc_options << '\n'
              << stats_options << '\n'
              << advanced_options << '\n';

    return boost::optional<configuration>();
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
  conf.pn_input = configure_pn_parser(vm);
  conf.properties_input = configure_properties_parser(vm);

  // Order options
  conf.order_flat = vm.count(order_flat_str);
  conf.order_ordering_force = vm.count(order_force_str);
  conf.order_force_iterations = vm[order_force_iterations_str].as<unsigned int>();
  conf.order_reverse = vm.count(order_reverse_str);
  conf.order_id_per_hierarchy = vm[order_id_per_hier_str].as<unsigned int>();
  conf.order_lexical = vm.count(order_lexical_str);
  conf.order_only = vm.count(order_only_str);

  if (conf.order_ordering_force and conf.order_lexical)
  {
    throw po::error("Can't use both lexical and FORCE orders");
  }

  // Petri net options
  conf.marking_bound = vm[pn_marking_bound_str].as<unsigned int>();
  conf.one_safe = vm.count(pn_one_safe_str);

  // Model checking options
  conf.compute_dead_states = vm.count(mc_dead_states_str);
  conf.compute_dead_transitions = vm.count(mc_dead_transitions_str);
  conf.count_tokens = vm.count(mc_count_tokens_str);
  conf.trace = vm.count(mc_trace_str);

  if (conf.order_ordering_force)
  {
    conf.order_flat = true;
  }

  if (vm.count(time_limit_str))
  {
    conf.max_time = std::chrono::duration<double>(vm[time_limit_str].as<unsigned int>());
  }
  if (vm.count(order_load_str))
  {
    conf.order_file = util::in_file(vm[order_load_str].as<std::string>());
  }

  if (vm.count(output_dir_str))
  {
    conf.output_dir = util::directory(vm[output_dir_str].as<std::string>());
  }
  else
  {
    conf.output_dir = boost::filesystem::current_path();
  }

  /* ----------------------------------------------------------------- */

  const auto parse_conf = [&](const std::string& option, auto& conf_map, const auto& values_map)
  {
    using separator = boost::char_separator<char>;
    auto tks = boost::tokenizer<separator>{vm[option].as<std::string>(), separator{","}};
    std::transform( begin(tks), end(tks), std::inserter(conf_map, end(conf_map))
                  , [&](const auto& s)
                    {
                      const auto search = values_map.find(s);
                      if (search == end(values_map))
                      {
                        throw po::error("Invalid " + option + " option value: " + s);
                      }
                      return search->second;
                    });
  };

  if (vm.count(dot_export_str))
  {
    parse_conf(dot_export_str, conf.dot_conf, dot_export_values_map);
  }

  if (vm.count(stats_str))
  {
    parse_conf(stats_str, conf.stats_conf, stats_values_map);
  }

  if (vm.count(json_str))
  {
    parse_conf(json_str, conf.json_conf, json_export_values_map);
  }

  /* ----------------------------------------------------------------- */

  const auto parse_sizes = [&](const std::string& option, auto& map)
  {
    using separator = boost::char_separator<char>;
    auto tks = boost::tokenizer<separator>{vm[option].as<std::string>(), separator{","}};
    for (const auto& tk : tks)
    {
      auto tmp = std::vector<std::string>{};
      auto subtks = boost::tokenizer<separator>{tk, separator{":"}};
      std::copy(begin(subtks), end(subtks), std::back_inserter(tmp));
      if (tmp.size() != 2)
      {
        throw po::error("Invalid " + option + " option value: " + tk);
      }
      const auto search = map.find(tmp[0]);
      if (search == end(conf.cache_sizes))
      {
        throw po::error("Invalid " + option + " option value: " + tk);
      }
      try
      {
        search->second = std::max(min_cache_size, std::stoul(tmp[1]));
      }
      catch (std::invalid_argument&)
      {
        throw po::error("Invalid " + option + " option value: " + tk);
      }
    }
  };

  conf.cache_sizes = default_cache_sizes;
  if (vm.count(cache_str))
  {
    parse_sizes(cache_str, conf.cache_sizes);
  }

  conf.ut_sizes = default_ut_sizes;
  if (vm.count(ut_str))
  {
    parse_sizes(ut_str, conf.ut_sizes);
  }

  return conf;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
