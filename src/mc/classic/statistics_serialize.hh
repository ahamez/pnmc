#ifndef _PNMC_MC_STATISTICS_SERIALIZE_HH_
#define _PNMC_MC_STATISTICS_SERIALIZE_HH_

#include <cereal/archives/json.hpp>
#include <cereal/types/deque.hpp>

#include "conf/configuration.hh"
#include "mc/classic/statistics.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

template<class Archive>
void
save(Archive& archive, const statistics& s)
{
  archive( cereal::make_nvp("interrupted", s.interrupted)
         , cereal::make_nvp("time limit", s.conf.max_time.count())
         , cereal::make_nvp("states", s.nb_states)
         , cereal::make_nvp("states as string", std::to_string(s.nb_states))
         , cereal::make_nvp("relation time", s.relation_duration.count())
         , cereal::make_nvp("rewrite time", s.rewrite_duration.count())
         , cereal::make_nvp("state space time", s.state_space_duration.count())
         , cereal::make_nvp("sdd samples", s.sdd_ut_size)
         );
  if (s.conf.order_ordering_force)
  {
    archive(cereal::make_nvp("FORCE time", s.force_duration.count()));
  }
  if (s.conf.compute_dead_states)
  {
    archive( cereal::make_nvp("dead states relation time", s.dead_states_relation_duration.count())
           , cereal::make_nvp("dead states rewrite time", s.dead_states_rewrite_duration.count())
           , cereal::make_nvp("dead states time", s.dead_states_duration.count())
           );
  }
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

#endif // _PNMC_MC_STATISTICS_SERIALIZE_HH_
