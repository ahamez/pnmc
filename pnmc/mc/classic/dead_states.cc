#include <iostream>
#include <set>

#include "mc/classic/dead.hh"
#include "mc/classic/dead_states.hh"
#include "shared/util/timer.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

SDD
dead_states(const order& o, const pn::net& net, const SDD& states, shared::statistics& stats)
{
  std::set<homomorphism> and_operands;
  std::set<homomorphism> or_operands;

  util::timer timer;

  // Get relation
  for (const auto& transition : net.transitions())
  {
    // We are only interested in pre actions.
    for (const auto& arc : transition.pre)
    {
      or_operands.insert(function(o, arc.first, dead{arc.second.weight}));
    }

    and_operands.insert(sum(o, or_operands.cbegin(), or_operands.cend()));
    or_operands.clear();
  }
  const auto tmp = intersection(o, and_operands.cbegin(), and_operands.cend());
  stats.dead_states_relation_duration = timer.duration();

  // Rewrite the relation
  timer.reset();
  const auto h = sdd::rewrite(o, tmp);
  stats.dead_states_rewrite_duration = timer.duration();

  // Compute the dead states
  timer.reset();
  const auto res = h(o, states);
  stats.dead_states_duration = timer.duration();

  return res;
}
/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
