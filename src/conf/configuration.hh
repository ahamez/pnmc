#ifndef _PNMC_CONF_CONFIGURATION_HH_
#define _PNMC_CONF_CONFIGURATION_HH_

#include <chrono>
#include <iosfwd>
#include <string>

#include <boost/optional.hpp>

namespace pnmc { namespace conf {
  
/*------------------------------------------------------------------------------------------------*/

enum class input_format {bpn, prod, tina, xml};

struct pnmc_configuration
{
  std::string file_name;
  input_format file_type;
  bool read_stdin;
  bool delete_file;

  bool order_random;
  bool order_show;
  bool order_force_flat;
  bool order_ordering_force;
  unsigned int order_min_height;

  bool show_relation;
  bool show_time;

  bool compute_dead_transitions;
  bool compute_dead_states;
  unsigned int marking_bound;

  bool show_final_sdd_bytes;

  boost::optional<std::string> export_final_sdd_dot_file;
  boost::optional<std::string> export_to_lua_file;
  boost::optional<std::string> json_file;
  boost::optional<std::string> hypergraph_dot_file;

  std::chrono::duration<double> max_time;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::configuration

#endif // _PNMC_CONF_CONFIGURATION_HH_
