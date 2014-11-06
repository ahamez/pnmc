#pragma once

#include <boost/optional.hpp>

#include "conf/configuration.hh"

namespace pnmc { namespace conf {
  
/*------------------------------------------------------------------------------------------------*/

boost::optional<configuration>
fill_configuration(int, const char**);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
