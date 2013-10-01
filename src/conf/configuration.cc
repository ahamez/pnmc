#include <iostream>

#include "configuration.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const pnmc_configuration& c)
{
  os << std::endl << "Configuration summary:" << std::endl << std::endl;
  os << "File: " << c.file_name << std::endl;
  return os;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::configuration
