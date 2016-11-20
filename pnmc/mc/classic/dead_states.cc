/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <set>

#include "mc/classic/dead_states.hh"
#include "mc/classic/filter_ge.hh"
#include "mc/classic/filter_lt.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

SDD
dead_states(const order& o, const pn::net& net, const SDD& state_space)
{
  auto and_operands = std::set<homomorphism>{};
  auto or_operands = std::set<homomorphism>{};

  for (const auto& transition : net.transitions())
  {
    // We are only interested in pre actions.
    for (const auto& arc : transition.pre)
    {
      switch (arc.second.kind)
      {
        case pn::arc::type::normal:
        case pn::arc::type::read:
          or_operands.insert(function(o, arc.first, filter_lt{arc.second.weight}));
          break;

        case pn::arc::type::reset:
          // Does not impose a precondition on firing.
          continue;

        case pn::arc::type::inhibitor:
          or_operands.insert(function(o, arc.first, filter_ge{arc.second.weight}));
          break;

        case pn::arc::type::stopwatch:
          throw std::runtime_error{"Unsupported stopwatch pre arc for dead states"};

        case pn::arc::type::stopwatch_inhibitor:
          throw std::runtime_error{"Unsupported stopwatch inhibitor pre arc for dead states"};
      }
    }

    and_operands.insert(sum(o, or_operands.cbegin(), or_operands.cend()));
    or_operands.clear();
  }
  const auto tmp = intersection(o, and_operands.cbegin(), and_operands.cend());

  return sdd::rewrite(o, tmp)(o, state_space); // compute dead states
}
/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
