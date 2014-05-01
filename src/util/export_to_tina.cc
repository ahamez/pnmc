#include <fstream>
#include <stdexcept>

#include "pn/tina.hh"
#include "util/export_to_tina.hh"

namespace pnmc { namespace util {

/*------------------------------------------------------------------------------------------------*/

void
export_to_tina(const conf::configuration& conf, const pn::net& net)
{
  if (conf.export_tina_file)
  {
    std::ofstream file(*conf.export_tina_file);
    if (file.is_open())
    {
      pn::tina(file, net);
    }
    else
    {
      throw std::runtime_error("Can't export Petri net to " + *conf.export_tina_file);
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::util
