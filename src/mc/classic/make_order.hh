#pragma once

#include <sdd/conf/default_configurations.hh>
#include <sdd/order/order.hh>

#include "conf/configuration.hh"
#include "mc/shared/statistics.hh"
#include "pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

sdd::order<sdd::conf1>
make_order(const conf::configuration&, shared::statistics&, const pn::net&);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
