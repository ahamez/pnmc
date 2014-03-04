#ifndef _PNMC_MC_STATISTICS_HH_
#define _PNMC_MC_STATISTICS_HH_

#include <chrono>
#include <iosfwd>

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

  template<class Archive>
  void
  save(Archive& archive)
  const
  {
    archive( cereal::make_nvp("interrupted", interrupted)
           , cereal::make_nvp("states", nb_states)
           , cereal::make_nvp("states as string", std::to_string(nb_states))
           , cereal::make_nvp("relation time", relation_duration.count())
           , cereal::make_nvp("rewrite time", rewrite_duration.count())
           , cereal::make_nvp("state space time", state_space_duration.count())
           );
    if (conf.order_ordering_force)
    {
      archive(cereal::make_nvp("FORCE time", force_duration.count()));
    }
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
