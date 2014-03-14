#ifndef _PNMC_CONF_CONFIGURATION_HH_
#define _PNMC_CONF_CONFIGURATION_HH_

#include <chrono>
#include <iosfwd>
#include <string>

#include <boost/optional.hpp>

namespace pnmc { namespace conf {
  
/*------------------------------------------------------------------------------------------------*/

/// @brief The accepted input formats.
enum class input_format {bpn, prod, tina, xml};

/*------------------------------------------------------------------------------------------------*/

/// @brief The configuration of pnmc at runtime.
struct pnmc_configuration
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
  bool order_force_flat;

  /// @brief Use the FORCE ordering heuristic.
  bool order_ordering_force;
  unsigned int order_min_height;

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

  boost::optional<std::string> export_final_sdd_dot_file;
  boost::optional<std::string> export_to_lua_file;
  boost::optional<std::string> json_file;
  boost::optional<std::string> hypergraph_dot_file;

  /// @brief Stop state space generation after this much time.
  std::chrono::duration<double> max_time;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::configuration

#endif // _PNMC_CONF_CONFIGURATION_HH_
