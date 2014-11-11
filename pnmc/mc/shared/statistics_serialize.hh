#pragma once

#include <cereal/archives/json.hpp>
#include <cereal/types/deque.hpp>

#include "conf/configuration.hh"
#include "mc/shared/statistics.hh"

namespace pnmc { namespace mc { namespace shared {

/*------------------------------------------------------------------------------------------------*/

template<class Archive>
void
save(Archive& archive, const statistics& s)
{
  if (s.conf.order_only)
  {
    archive(cereal::make_nvp("order only", true));
    if (s.conf.order_ordering_force)
    {
      archive( cereal::make_nvp("FORCE time", s.force_duration.count())
             , cereal::make_nvp("FORCE spans", s.force_spans));
    }
    return;
  }
  archive( cereal::make_nvp("order only", false)
         , cereal::make_nvp("interrupted", s.interrupted)
         , cereal::make_nvp("time limit", s.conf.max_time.count())
         , cereal::make_nvp("states", s.nb_states)
         , cereal::make_nvp("total time", s.total_duration.count())
         , cereal::make_nvp("relation time", s.relation_duration.count())
         , cereal::make_nvp("rewrite time", s.rewrite_duration.count())
         , cereal::make_nvp("state space time", s.state_space_duration.count()));
  if (s.conf.count_tokens)
  {
    archive(cereal::make_nvp("count tokens time", s.tokens_duration.count()));
  }
  if (s.conf.sample_nb_sdd)
  {
    archive(cereal::make_nvp("sdd samples", s.sdd_ut_size));
  }
  if (s.conf.order_ordering_force)
  {
    archive( cereal::make_nvp("FORCE time", s.force_duration.count())
           , cereal::make_nvp("FORCE spans", s.force_spans));
  }
  if (s.conf.compute_dead_states)
  {
    archive( cereal::make_nvp("dead states relation time", s.dead_states_relation_duration.count())
           , cereal::make_nvp("dead states rewrite time", s.dead_states_rewrite_duration.count())
           , cereal::make_nvp("dead states time", s.dead_states_duration.count())
           , cereal::make_nvp("trace time", s.trace_duration.count()));
  }
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::shared
