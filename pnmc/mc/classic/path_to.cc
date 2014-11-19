#include <algorithm>
#include <vector>

#include <iostream>

#include "mc/classic/path_to.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

std::deque<SDD>
path_to( const order& o, const SDD& initial, const SDD& targets
       , const std::set<homomorphism>& operands)
{
  auto top_bottom = [&]
  {
    using std::placeholders::_1;
    const auto firing_rule = rewrite(o, sum(o, begin(operands), end(operands)));

    SDD current_layer = initial;
    SDD acc = current_layer; // Keep all computed states to avoid to recompute states.

    std::deque<SDD> res;
    res.push_back(initial);

    // BFS
    while (true)
    {
      const auto next_layer = firing_rule(o, current_layer) // get successors of the current layer
                            - acc; // remove all states that were computed in a previous layer;

      const auto test_targets = next_layer & targets;
      if (test_targets != zero()) // some targets found
      {
        res.emplace_back(test_targets); // add found targets
        break;
      }

      acc += next_layer;
      res.emplace_back(next_layer);
      current_layer = next_layer; // move on to the next layer
    }

    return res;
  }();

  // We now have the shortest paths to the targets that are the closest to the initial state, along
  // with some other possible paths which do not lead to the targets.
  std::deque<SDD> trace;
  for (auto rcit = std::next(rbegin(top_bottom)); rcit != rend(top_bottom); ++rcit)
  {
    const auto next_states = *std::prev(rcit);
    for (const auto& transition : operands)
    {
      const auto succ = transition(o, *rcit);
      const auto inter = succ & next_states;
      if (inter != zero())
      {
        *std::prev(rcit) = inter;
        trace.emplace_front(inter);
        break;
      }
    }
  }
  trace.emplace_front(initial);

//  std::cout << '\n';
//  for (const auto& xy : trace)
//  {
//    std::deque<std::reference_wrapper<const std::string>> identifiers;
//    o.flat(std::back_inserter(identifiers));
//    auto path_generator = xy.second.paths();
//    while (path_generator)
//    {
//      const auto& path = path_generator.get();
//      path_generator(); // advance generator
//      auto id_cit = begin(identifiers);
//      auto path_cit = begin(path);
//      std::cout << xy.first << std::setw(int(20-xy.first.size())) << " -> ";
//      for (; path_cit != path.cend(); ++path_cit, ++id_cit)
//      {
//        auto copy = *path_cit;
//        copy.erase(0);
//        if (not copy.empty())
//        {
//          std::cout << id_cit->get() << ':' << *path_cit << ' ';
//        }
//      }
//      std::cout << '\n';
//    }
//  }
  return trace;
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
