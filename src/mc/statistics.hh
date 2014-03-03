#ifndef _PNMC_MC_STATISTICS_HH_
#define _PNMC_MC_STATISTICS_HH_

#include <chrono>
#include <iosfwd>

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

struct statistics
{
  std::chrono::duration<double> relation_duration;
  std::chrono::duration<double> rewrite_duration;
  std::chrono::duration<double> state_space_duration;
  std::chrono::duration<double> dead_states_relation_duration;
  std::chrono::duration<double> dead_states_rewrite_duration;
  std::chrono::duration<double> dead_states_duration;

  long double nb_states;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

#endif // _PNMC_MC_STATISTICS_HH_
