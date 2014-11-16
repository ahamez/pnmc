#pragma once

#include <chrono>
#include <map>
#include <set>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include "mc/shared/export_configuration.hh"
#include "support/parsers/configuration.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

/// @brief The configuration of pnmc at runtime.
struct configuration
{
  /// @brief Describe how to read input.
  parsers::configuration input;

  /// @brief A random order is computed.
  bool order_random;

  /// @brief Show order after its creation.
  bool order_show;

  /// @brief Remove all hierarchy from order.
  bool order_flat;

  /// @brief Reverse order.
  bool order_reverse;

  /// @brief Use the FORCE ordering heuristic.
  bool order_ordering_force;
  unsigned int order_force_iterations;

  /// @brief Maximum number of identifiers per hierarchy.
  unsigned int order_id_per_hierarchy;

  /// @brief Stop after order computation.
  bool order_only;

  /// @brief Show the time spent in various steps.
  bool show_time;

  /// @bref Display, if any, dead transitions.
  bool compute_dead_transitions;

  /// @brief Display, if any, dead states.
  bool compute_dead_states;

  /// @brief Stop state space generation if a place's marking reaches this limit.
  unsigned int marking_bound;

  /// @brief Display the memory usage of the state space's SDD.
  bool show_final_sdd_bytes;

  /// @brief Don't cleanup memory on exit.
  bool fast_exit;

  /// @brief Sample the number of SDD regularly.
  bool sample_nb_sdd;

  /// @brief Compute the maximal markings.
  bool count_tokens;

  /// @brief The processed Petri net is 1-safe.
  bool one_safe;

  /// @brief Export costly SDD statistics.
  bool final_sdd_statistics;

  /// @brief Compute some statistics on the Petri net.
  bool pn_statistics;

  std::size_t sdd_ut_size;
  std::size_t hom_ut_size;

  boost::filesystem::path output_dir;

  boost::optional<boost::filesystem::path> json_file;
  boost::optional<boost::filesystem::path> results_json_file;
  boost::optional<boost::filesystem::path> hom_json_file;
  boost::optional<boost::filesystem::path> order_file;

  std::set<mc::shared::dot_export> dot_dump;

  /// @brief Stop state space generation after this much time.
  std::chrono::duration<double> max_time;

  std::map<std::string, std::size_t> cache_sizes;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::configuration
