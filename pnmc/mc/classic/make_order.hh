#pragma once

#include "conf/configuration.hh"
#include "mc/classic/sdd.hh"
#include "mc/shared/statistics.hh"
#include "support/pn/net.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

order
make_order(const conf::configuration&, shared::statistics&, const pn::net&);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
