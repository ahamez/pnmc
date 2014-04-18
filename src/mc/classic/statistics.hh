#ifndef _PNMC_MC_STATISTICS_HH_
#define _PNMC_MC_STATISTICS_HH_

#include <chrono>
#include <deque>

#include "conf/configuration.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct statistics
{
  const conf::configuration& conf;

  std::chrono::duration<double> relation_duration;
  std::chrono::duration<double> rewrite_duration;
  std::chrono::duration<double> state_space_duration;
  std::chrono::duration<double> tokens_duration;
  std::chrono::duration<double> force_duration;
  std::chrono::duration<double> dead_states_relation_duration;
  std::chrono::duration<double> dead_states_rewrite_duration;
  std::chrono::duration<double> dead_states_duration;
  std::chrono::duration<double> total_duration;

  long double nb_states;

  bool interrupted;

  std::deque<unsigned int> sdd_ut_size;

  std::deque<double> force_spans;

  statistics(const conf::configuration& c)
    : conf(c), relation_duration(), rewrite_duration(), state_space_duration(), tokens_duration()
    , force_duration(), dead_states_relation_duration(), dead_states_rewrite_duration()
    , dead_states_duration(), total_duration(), nb_states(0), interrupted(false), sdd_ut_size()
    , force_spans()
  {}
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

#endif // _PNMC_MC_STATISTICS_HH_
