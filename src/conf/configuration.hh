#ifndef _PNMC_CONF_CONFIGURATION_HH_
#define _PNMC_CONF_CONFIGURATION_HH_

#include <chrono>
#include <iosfwd>
#include <string>

#include <boost/optional.hpp>

namespace pnmc { namespace conf {
  
/*------------------------------------------------------------------------------------------------*/

/// @brief The accepted input formats.
enum class input_format {bpn, pnml, prod, tina, xml};

/*------------------------------------------------------------------------------------------------*/

/// @brief The configuration of pnmc at runtime.
struct configuration
{
  /// @brief The model's file to process.
  std::string file_name;

  /// @brief The model's format to process.
  input_format file_type;

  /// @brief Read from standard input rather than from a file.
  bool read_stdin;

  /// @brief Delete model file after reading it.
  bool delete_file;

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

  /// @brief Show the homomorphism of the transition relation.
  bool show_relation;

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

  std::size_t sdd_ut_size;
  std::size_t sdd_diff_cache_size;
  std::size_t sdd_inter_cache_size;
  std::size_t sdd_sum_cache_size;
  std::size_t hom_ut_size;
  std::size_t hom_cache_size;

  boost::optional<std::string> export_final_sdd_dot_file;
  boost::optional<std::string> export_to_lua_file;
  boost::optional<std::string> json_file;
  boost::optional<std::string> results_json_file;
  boost::optional<std::string> hypergraph_dot_file;
  boost::optional<std::string> export_hom_to_dot_file;
  boost::optional<std::string> export_sat_hom_to_dot_file;

  /// @brief Stop state space generation after this much time.
  std::chrono::duration<double> max_time;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::configuration

#endif // _PNMC_CONF_CONFIGURATION_HH_
