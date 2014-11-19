#include <algorithm>
#include <vector>

#include <boost/iterator/transform_iterator.hpp>

#include "mc/classic/filter_ge.hh"
#include "mc/classic/filter_lt.hh"
#include "mc/classic/path_to.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

std::deque<std::pair<std::string, SDD>>
path_to( const order& o, const SDD& initial, const SDD& targets, const pn::net& net
       , const std::multimap<homomorphism, std::string>& operands)
{
  auto top_bottom = [&]
  {
    const auto key = [](const auto& kv){return kv.first;};
    const auto firing_rule = rewrite(o, sum( o
                                           , boost::make_transform_iterator(begin(operands), key)
                                           , boost::make_transform_iterator(end(operands), key)));

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

  const auto enabled_operations = [&]
  {
    std::map<std::string, homomorphism> res;
    for (const auto& transition : net.transitions())
    {
      auto h = sdd::id<sdd_conf>();
      for (const auto& arc : transition.pre)
      {
        auto h_arc = sdd::id<sdd_conf>();
        switch (arc.second.kind)
        {
          case pn::arc::type::normal:
            h_arc = function(o, arc.first, filter_ge{arc.second.weight});
            break;

          case pn::arc::type::inhibitor:
            h_arc = function(o, arc.first, filter_lt{arc.second.weight});
            break;

          case pn::arc::type::read:
            h_arc = function(o, arc.first, filter_ge{arc.second.weight});
            break;

          default:
            throw std::runtime_error(__PRETTY_FUNCTION__);
        }
        h = composition(h, h_arc);
      }
      res.emplace(transition.id, h);
    }
    return res;
  }();

  // We now have the shortest paths to the targets that are the closest to the initial state, along
  // with some other possible paths which do not lead to the targets.
  std::deque<std::pair<std::string, SDD>> trace;
  for (auto rcit = std::next(rbegin(top_bottom)); rcit != rend(top_bottom); ++rcit)
  {
    const auto next_states = *std::prev(rcit);
    for (const auto& h_id : operands)
    {
      const auto& transition = h_id.first;
      const auto& tid = h_id.second;
      if (tid == "id") continue; // ugly
      const auto& enabled = enabled_operations.at(tid);
      const auto current = enabled(o, *rcit);
      if (current != zero())
      {
        const auto succ = transition(o, current);
        const auto inter = succ & next_states;
        if (inter != zero())
        {
          *std::prev(rcit) = inter;
          *rcit = current;
          trace.emplace_front(tid, inter);
          break;
        }
      }
    }
  }
  trace.emplace_front("initial", initial);

  return trace;
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
