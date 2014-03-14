#ifndef _PNMC_CONF_FILL_CONFIGURATION_HH_
#define _PNMC_CONF_FILL_CONFIGURATION_HH_

#include <boost/optional.hpp>

#include "conf/configuration.hh"

namespace pnmc { namespace conf {
  
/*------------------------------------------------------------------------------------------------*/

boost::optional<configuration>
fill_configuration(int, char**);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf

#endif // _PNMC_CONF_FILL_CONFIGURATION_HH_
