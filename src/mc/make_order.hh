#ifndef _PNMC_MC_MAKE_ORDER_HH_
#define _PNMC_MC_MAKE_ORDER_HH_

#include <sdd/conf/default_configurations.hh>
#include <sdd/order/order.hh>

#include "conf/configuration.hh"
#include "mc/statistics.hh"
#include "pn/net.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

sdd::order<sdd::conf1>
make_order(const conf::pnmc_configuration&, statistics&, const pn::net&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

#endif // _PNMC_MC_MAKE_ORDER_HH_
