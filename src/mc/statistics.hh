#ifndef _PNMC_MC_STATISTICS_HH_
#define _PNMC_MC_STATISTICS_HH_

#include <chrono>
#include <iosfwd>

#include "conf/configuration.hh"

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

  const conf::pnmc_configuration& conf;

  statistics(const conf::pnmc_configuration& c)
    : conf(c)
  {}

  template<class Archive>
  void
  save(Archive& archive)
  const
  {
    archive( cereal::make_nvp("relation time", relation_duration.count())
           , cereal::make_nvp("rewrite time", rewrite_duration.count())
           , cereal::make_nvp("state space time", state_space_duration.count())
           );

    if (conf.compute_dead_states)
    {
      archive( cereal::make_nvp("dead states relation time", dead_states_relation_duration.count())
             , cereal::make_nvp("dead states rewrite time", dead_states_rewrite_duration.count())
             , cereal::make_nvp("dead states time", dead_states_duration.count())
             );
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

#endif // _PNMC_MC_STATISTICS_HH_
