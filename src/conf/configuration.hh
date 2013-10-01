#ifndef _PNMC_CONF_CONFIGURATION_HH_
#define _PNMC_CONF_CONFIGURATION_HH_

#include <iosfwd>
#include <string>

namespace pnmc { namespace conf {
  
/*------------------------------------------------------------------------------------------------*/

struct pnmc_configuration
{
  std::string file_name;

  pnmc_configuration()
  	: file_name()
  {}
};

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream&, const pnmc_configuration&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::configuration

#endif // _PNMC_CONF_CONFIGURATION_HH_
