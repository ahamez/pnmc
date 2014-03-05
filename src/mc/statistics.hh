#ifndef _PNMC_MC_STATISTICS_HH_
#define _PNMC_MC_STATISTICS_HH_

#include <chrono>

#include "conf/configuration.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

struct statistics
{
  const conf::pnmc_configuration& conf;

  std::chrono::duration<double> relation_duration;
  std::chrono::duration<double> rewrite_duration;
  std::chrono::duration<double> state_space_duration;
  std::chrono::duration<double> force_duration;
  std::chrono::duration<double> dead_states_relation_duration;
  std::chrono::duration<double> dead_states_rewrite_duration;
  std::chrono::duration<double> dead_states_duration;

  long double nb_states;

  bool interrupted;

  statistics(const conf::pnmc_configuration& c)
    : conf(c), relation_duration(), rewrite_duration(), state_space_duration(), force_duration()
    , dead_states_relation_duration(), dead_states_rewrite_duration()
    , dead_states_duration(), nb_states(0), interrupted(false)
  {}
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

#endif // _PNMC_MC_STATISTICS_HH_
