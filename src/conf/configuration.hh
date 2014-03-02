#ifndef _PNMC_CONF_CONFIGURATION_HH_
#define _PNMC_CONF_CONFIGURATION_HH_

#include <iosfwd>
#include <string>

namespace pnmc { namespace conf {
  
/*------------------------------------------------------------------------------------------------*/

enum class input_format {bpn, prod, tina, xml};

struct pnmc_configuration
{
  std::string file_name;
  input_format file_type;
  bool read_stdin;
  bool delete_file;

  bool export_final_sdd_dot;
  std::string export_final_sdd_dot_file;

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

  bool export_to_lua;
  std::string export_to_lua_file;

  bool show_final_sdd_bytes;

  bool final_sdd_stats_json;
  std::string final_sdd_stats_json_file;
  bool manager_stats_json;
  std::string manager_stats_json_file;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::configuration

#endif // _PNMC_CONF_CONFIGURATION_HH_
