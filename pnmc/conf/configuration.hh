#pragma once

#include <chrono>
#include <map>
#include <set>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include "mc/shared/export_configuration.hh"
#include "mc/shared/statistics_configuration.hh"
#include "support/parsers/configuration.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

/// @brief The configuration of pnmc at runtime.
struct configuration
{
  /// @brief Describe how to read input.
  parsers::configuration input;

  /// @brief Remove all hierarchy from order.
  bool order_flat;

  /// @brief Reverse order.
  bool order_reverse;

  /// @brief Use the FORCE ordering heuristic.
  bool order_ordering_force;
  unsigned int order_force_iterations;

  /// @brief Maximum number of identifiers per hierarchy.
  unsigned int order_id_per_hierarchy;

  /// @bref Display, if any, dead transitions.
  bool compute_dead_transitions;

  /// @brief Display, if any, dead states.
  bool compute_dead_states;

  /// @brief Stop state space generation if a place's marking reaches this limit.
  unsigned int marking_bound;

  /// @brief Don't cleanup memory on exit.
  bool fast_exit;

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

  std::set<mc::shared::dot_export> dot_conf;
  std::set<mc::shared::stats> stats_conf;
  std::set<mc::shared::json_export> json_conf;

  std::map<std::string, std::size_t> cache_sizes;
  std::map<std::string, std::size_t> ut_sizes;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::configuration
