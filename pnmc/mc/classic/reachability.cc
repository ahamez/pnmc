#include <boost/variant.hpp>

#include "mc/classic/dead_states.hh"
#include "mc/classic/place_bound.hh"
#include "mc/classic/reachability.hh"
#include "mc/classic/reachability_ast.hh"
#include "mc/classic/reachability_eval.hh"
#include "mc/shared/step.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

void
reachability(const order& o, const properties::formulae& f, results& res)
{
  const bool deadlock = res.dead_states
                      ? *res.dead_states != zero()
                      : false;

  const auto bounds = place_bound(o, *res.states, f.places_bounds);
  const auto dead_transitions = [&]
  {
    if (res.dead_transitions) {return *res.dead_transitions;}
    else                      {return std::set<std::string>();}
  }();

  for (const auto& formula : f.booleans)
  {
    if (not res.reachability)
    {
      res.reachability.emplace();
    }
    const auto ast = make_ast(formula.expression, deadlock, dead_transitions, bounds);
    res.reachability->emplace_back(formula.id, eval(ast, *res.states));
  }

  for (const auto& formula : f.integers)
  {
    if (not res.evaluations)
    {
      res.evaluations.emplace();
    }
    const auto ast = make_ast(formula.expression, bounds);
    res.evaluations->emplace_back(formula.id, eval(ast, *res.states));
  }
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
