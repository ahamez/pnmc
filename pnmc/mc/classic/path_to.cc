#include <algorithm>
//#include <iostream>
#include <unordered_map>

#include "mc/classic/path_to.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

using function_base = sdd::hom::function_base<sdd_conf>;

struct get_function_base
{
  std::unordered_map<sdd_conf::variable_type, const function_base*>& res;

  void
  operator()(const sdd::hom::_composition<sdd_conf>& c)
  const
  {
    sdd::visit(*this, c.left);
    sdd::visit(*this, c.right);
  }

  void
  operator()(const sdd::hom::_function<sdd_conf>& f)
  const
  {
    res.emplace(f.target, f.fun_ptr.get());
  }

  template <typename T>
  void
  operator()(const T&)
  const
  {
    assert(false);
  }
};

std::unordered_map<sdd_conf::variable_type, const function_base*>
transition_functions(const homomorphism& h)
{
  std::unordered_map<sdd_conf::variable_type, const function_base*> res;
  sdd::visit(get_function_base{res}, h);
  return res;
}

/*------------------------------------------------------------------------------------------------*/

struct apply_transition
{
  const std::unordered_map<sdd_conf::variable_type, const function_base*>& transition;

  SDD
  operator()(const sdd::hierarchical_node<sdd_conf>&, const order&)
  const
  {
    assert(false);
    return zero();
  }

  SDD
  operator()(const sdd::flat_node<sdd_conf>& n, const order& o)
  const
  {
    const auto search = transition.find(o.variable());
    assert(search != end(transition));
    const auto f = search->second;
    for (const auto& arc : n)
    {
      const auto rec = visit(*this, arc.successor(), o.next());
      if (rec != zero())
      {
        for (const auto v : arc.valuation())
        {

        }
      }
    }
    return zero();
  }

  SDD
  operator()(const sdd::one_terminal<sdd_conf>&, const order&)
  const
  {
    return one();
  }

  SDD
  operator()(const sdd::zero_terminal<sdd_conf>&, const order&)
  const
  {
    return zero();
  }
};

/*------------------------------------------------------------------------------------------------*/

std::deque<SDD>
path_to( const order& o, const SDD& initial, const SDD& targets
       , const std::set<homomorphism>& operands)
{
  const auto firing_rule = rewrite(o, sum(o, begin(operands), end(operands)));

  SDD current_layer = initial;
  SDD acc = current_layer; // Keep all computed states to avoid to recompute states.

  std::deque<SDD> initial_to_targets;
  initial_to_targets.push_back(initial);

  // BFS
  while (true)
  {
    const auto next_layer = firing_rule(o, current_layer) // get all successors of the current layer
                          - acc; // remove all states that were computed in a previous layer;

    const auto test_targets = next_layer & targets;
    if ((test_targets) != zero()) // some targets found
    {
      initial_to_targets.emplace_back(test_targets); // add found targets
      break;
    }

    acc += next_layer;
    initial_to_targets.emplace_back(next_layer);
    current_layer = next_layer; // move on to the next layer
  }

  // We now have the shortest paths to the targets that are the closest to the initial state, along
  // with some other possible paths which do not lead to the targets.

  std::deque<std::unordered_map<sdd_conf::variable_type, const function_base*>> transitions;
  std::transform( begin(operands), end(operands), std::inserter(transitions, transitions.end())
                , std::bind(transition_functions, std::placeholders::_1));


  /// @todo Prune shortest paths of useless paths.
  auto rcit = rbegin(initial_to_targets);
  while (std::next(rcit) != rend(initial_to_targets))
  {
    for (const auto& t : transitions)
    {
      const auto res = sdd::visit(apply_transition{t}, *std::next(rcit), o);
      if ((res & *rcit) != zero())
      {
//        *rcit &= res;
        ++rcit;
        *rcit = res;
        break;
      }
    }
  }

  return initial_to_targets;
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
