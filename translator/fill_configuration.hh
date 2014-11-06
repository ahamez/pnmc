#pragma once

#include <boost/optional.hpp>

#include "configuration.hh"

namespace pnmc { namespace conf {
  
/*------------------------------------------------------------------------------------------------*/

boost::optional<configuration>
fill_configuration(int, const char**);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
