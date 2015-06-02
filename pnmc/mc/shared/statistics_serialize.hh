/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <cereal/archives/json.hpp>
#include <cereal/types/deque.hpp>

#include <sdd/tools/serialization.hh>

#include "conf/configuration.hh"
#include "mc/shared/statistics.hh"

namespace pnmc { namespace mc { namespace shared {

/*------------------------------------------------------------------------------------------------*/

template <typename Archive, typename C>
void
save(Archive& archive, const statistics<C>& s)
{
  if (s.max_time)
  {
    archive(cereal::make_nvp("time limit", s.max_time->count()));
  }
  archive( cereal::make_nvp("interrupted", s.interrupted)
         , cereal::make_nvp("total time", s.total_duration.count())
         , cereal::make_nvp("relation time", s.relation_duration.count())
         , cereal::make_nvp("rewrite time", s.rewrite_duration.count())
         , cereal::make_nvp("state space time", s.state_space_duration.count()));
  if (s.force_duration)
  {
    archive(cereal::make_nvp("FORCE time", s.force_duration->count()));
  }
  if (s.dead_states_duration)
  {
    archive(cereal::make_nvp("dead states time", s.dead_states_duration->count()));
  }
  if (s.trace_duration)
  {
    archive(cereal::make_nvp("trace time", s.trace_duration->count()));
  }
  if (s.tokens_duration)
  {
    archive(cereal::make_nvp("count tokens time", s.tokens_duration->count()));
  }
  if (s.manager_statistics)
  {
    archive(cereal::make_nvp("libsdd", *s.manager_statistics));
  }
  if (s.force_spans)
  {
    archive(cereal::make_nvp("FORCE spans", *s.force_spans));
  }
  if (s.sdd_ut_size)
  {
    archive(cereal::make_nvp("sdd samples", *s.sdd_ut_size));
  }
  if (s.pn_statistics)
  {
    archive(cereal::make_nvp("pn", *s.pn_statistics));
  }
  if (s.sdd_statistics)
  {
    archive(cereal::make_nvp("state space", *s.sdd_statistics));
  }
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::shared