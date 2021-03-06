/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <chrono>
#include <map>
#include <set>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include "mc/shared/export_configuration.hh"
#include "mc/shared/statistics_configuration.hh"
#include "support/parsers/pn_configuration.hh"
#include "support/parsers/properties_configuration.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

/// @brief The configuration of pnmc at runtime.
struct configuration
{
  /// @brief Describe how to read the Petri net file.
  parsers::pn_configuration pn_input;

  /// @brief Describe how to read the properties file.
  parsers::properties_configuration properties_input;

  /// @brief Stop after order computation.
  bool order_only;

  /// @brief Remove all hierarchy from order.
  bool order_flat;

  /// @brief Reverse order.
  bool order_reverse;

  /// @brief Use the FORCE ordering heuristic.
  bool order_ordering_force;
  unsigned int order_force_iterations;

  /// @brief Sort variables using place names.
  bool order_lexical;

  /// @brief Maximum number of identifiers per hierarchy.
  unsigned int order_id_per_hierarchy;

  /// @bref Compute and display, if any, dead transitions.
  bool compute_dead_transitions;

  /// @brief Compute and display, if any, dead states.
  bool compute_dead_states;

  /// @brief Compute the shortest trace to the closest dead state.
  bool trace;

  /// @brief Stop state space generation if a place's marking reaches this limit.
  unsigned int marking_bound;

  /// @brief Compute the maximal markings.
  bool count_tokens;

  /// @brief The processed Petri net is 1-safe.
  bool one_safe;

  /// @brief Stop state space generation after this much time.
  boost::optional<std::chrono::duration<double>> max_time;

  /// @brief Where to save all output files.
  boost::filesystem::path output_dir;

  /// @brief Path to a file containing an order.
  boost::optional<boost::filesystem::path> order_file;

  /// @brief Configure what to export to DOT
  std::set<mc::shared::dot_export> dot_conf;

  /// @brief Configure statistics to show
  std::set<mc::shared::stats> stats_conf;

  /// @brief Configure what to export to JSON
  std::set<mc::shared::json_export> json_conf;

  /// @brief Configure cache sizes
  std::map<std::string, std::size_t> cache_sizes;

   /// @brief Configure unique table sizes
  std::map<std::string, std::size_t> ut_sizes;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::configuration
