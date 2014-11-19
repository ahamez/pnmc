#include <set>

#include "mc/classic/dead_states.hh"
#include "mc/classic/filter_lt.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

SDD
dead_states(const order& o, const pn::net& net, const SDD& state_space)
{
  std::set<homomorphism> and_operands;
  std::set<homomorphism> or_operands;

  for (const auto& transition : net.transitions())
  {
    // We are only interested in pre actions.
    for (const auto& arc : transition.pre)
    {
      or_operands.insert(function(o, arc.first, filter_lt{arc.second.weight}));
    }

    and_operands.insert(sum(o, or_operands.cbegin(), or_operands.cend()));
    or_operands.clear();
  }
  const auto tmp = intersection(o, and_operands.cbegin(), and_operands.cend());

  return sdd::rewrite(o, tmp)(o, state_space); // compute dead states
}
/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
