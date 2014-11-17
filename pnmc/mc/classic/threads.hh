#pragma once

#include <thread>

#include "conf/configuration.hh"
#include "mc/classic/sdd.hh"
#include "mc/shared/statistics.hh"
#include "support/util/timer.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct threads
{
  bool finished;
  std::thread clock;
  std::thread sdd_sampling;

  threads( const conf::configuration& conf, statistics& stats, bool& stop
         , const sdd::manager<sdd_conf>& manager, util::timer& beginnning);

  ~threads();
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
