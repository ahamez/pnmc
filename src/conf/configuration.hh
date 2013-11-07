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

  bool show_order;
  bool force_flat_order;

  bool show_relation;
  bool show_time;
  bool show_hash_tables_stats;

  bool compute_dead_transitions;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::configuration

#endif // _PNMC_CONF_CONFIGURATION_HH_
