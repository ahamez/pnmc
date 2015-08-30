/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <algorithm>
#include <cassert>
#include <functional> // function
#include <memory>
#include <random>
#include <unordered_map>
#include <vector>

#include <iosfwd>

#include <boost/iterator/transform_iterator.hpp>

#include "mc/classic/filter_ge.hh"
#include "mc/classic/filter_lt.hh"
#include "mc/classic/path_to.hh"
#include "mc/classic/post.hh"
#include "mc/classic/pre.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

using boost::make_transform_iterator;

/*------------------------------------------------------------------------------------------------*/

// Ordered by identifier position in SDD's order.
using state_type = std::vector<pn::valuation_type>;

state_type
one_state_of(const SDD& x)
{
  assert(x != zero());
  auto res = state_type{};
  auto path_generator = x.paths();
  const auto& path = path_generator.get(); // get only first path
  for (const auto& values : path)
  {
    res.emplace_back(*values.cbegin());
  }
  return res;
}

/*------------------------------------------------------------------------------------------------*/

using fn_type = std::function<flat_set (const flat_set&)>;

using identifier_map_type =
  std::unordered_map< std::string // transition identifier
                    , std::vector<fn_type>>; // operations to apply on a flat_set

using functions_type = std::vector<identifier_map_type>;

functions_type
mk_functions(const std::deque<std::string>& identifiers, const pn::net& net)
{
  auto map = std::unordered_map< std::string /*identifier*/, identifier_map_type>{};

  // intialize map for each identifier
  for (const auto& transition : net.transitions())
  {
    for (const auto& arc : transition.pre)
    {
      map.emplace(arc.first, identifier_map_type{});
    }
    for (const auto& arc : transition.post)
    {
      map.emplace(arc.first, identifier_map_type{});
    }
  }

  const auto add_fn = [&](const auto& arc, const auto& tid, const auto& fn)
  {
    const auto search = map.find(arc.first); // search variable
    assert(search != end(map));
    auto& identifier_map = search->second;
    const auto transition_search = identifier_map.find(tid);
    if (transition_search == end(identifier_map))
    {
      identifier_map.emplace_hint(end(identifier_map), tid, std::vector<fn_type>{fn});
    }
    else
    {
      transition_search->second.emplace_back(fn);
    }
  };

  for (const auto& transition : net.transitions())
  {
    for (const auto& arc : transition.pre)
    {
      const auto fn = [&]
      {
        switch (arc.second.kind)
        {
          case pn::arc::type::normal:
            return pre{arc.second.weight};

          default:
            throw std::runtime_error(__PRETTY_FUNCTION__);
        }
      }();
      add_fn(arc, transition.name, fn);
    }

    for (const auto& arc : transition.post)
    {
      const auto fn = [&]
      {
        switch (arc.second.kind)
        {
          case pn::arc::type::normal:
            return post{arc.second.weight};

          default:
            throw std::runtime_error(__PRETTY_FUNCTION__);
        }
      }();
      add_fn(arc, transition.name, fn);
    }
  }

  auto res = functions_type{};
  for (const auto& id : identifiers)
  {
    res.emplace_back(std::move(map.at(id)));
  }
  return res;
}


/*------------------------------------------------------------------------------------------------*/

struct visitor
{
  const std::string& transition;

  SDD
  operator()( const sdd::hierarchical_node<sdd_conf>&, state_type::const_iterator
            , functions_type::const_iterator)
  const
  {
    throw std::runtime_error(__PRETTY_FUNCTION__);
  }

  SDD
  operator()( const sdd::flat_node<sdd_conf>& n, state_type::const_iterator state_it
            , functions_type::const_iterator functions_it)
  const
  {
    const auto& operations_search = functions_it->find(transition);
    if (operations_search == (*functions_it).cend())
    {
      for (const auto& arc : n)
      {
        if (arc.valuation().find(*state_it) == arc.valuation().end())
        {
          continue; // to next arc
        }
        const auto rec = visit( *this, arc.successor(), std::next(state_it)
                               , std::next(functions_it));
        if (rec != zero())
        {
          return {n.variable(), arc.valuation(), rec}; // Hit!
        }
        else
        {
          continue; // to next arc
        }
      }
    }
    else // transition is applied on the current level
    {
      for (const auto& arc : n)
      {
        for (const auto v : arc.valuation())
        {
          auto values = flat_set{v};
          for (const auto& operation : operations_search->second)
          {
            values = operation(values);
            if (values.empty()) {break;} // No need to apply more operations
          }
          if (values.empty())
          {
            continue; // to next valuation
          }
          else
          {
            if (*values.cbegin() == *state_it)
            {
              const auto rec = visit( *this, arc.successor(), std::next(state_it)
                                    , std::next(functions_it));
              if (rec != zero())
              {
                return {n.variable(), arc.valuation(), rec}; // Hit!
              }
            }
          }
        }
      }
    }
    return zero();
  }

  SDD
  operator()( const sdd::one_terminal<sdd_conf>&, state_type::const_iterator
            , functions_type::const_iterator)
  const
  {
    return one();
  }

  SDD
  operator()( const sdd::zero_terminal<sdd_conf>&, state_type::const_iterator
            , functions_type::const_iterator)
  const
  {
    throw std::runtime_error(__PRETTY_FUNCTION__);
  }
};

/*------------------------------------------------------------------------------------------------*/

std::deque<std::pair<std::string, SDD>>
shortest_path( const order& o, const SDD& initial, const SDD& targets, const pn::net& net
             , const std::multimap<homomorphism, std::string>& operands)
{
  auto identifiers = std::deque<std::string>{};
  o.flat(std::back_inserter(identifiers));

  auto top_bottom = [&]
  {
    const auto key = [](const auto& kv){return kv.first;};
    const auto firing_rule = rewrite(o, sum( o
                                           , make_transform_iterator(begin(operands), key)
                                           , make_transform_iterator(end(operands), key)));

    auto current_layer = initial;
    auto acc = current_layer; // Keep all computed states to avoid to recompute states.

    auto res = std::deque<SDD>{};
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

  std::random_device rd;
  std::mt19937 g(rd());

  const auto id = [](const auto& transition){return transition.name;};
  auto transitions = std::vector<std::string>{ make_transform_iterator(begin(net.transitions()), id)
                                             , make_transform_iterator(end(net.transitions()), id)};

  const auto functions = mk_functions(identifiers, net);
  auto trace = std::deque<std::pair<std::string, SDD>>{};
  trace.emplace_front("", *top_bottom.rbegin());
  for (auto rcit = std::next(top_bottom.rbegin()); rcit != top_bottom.rend(); ++rcit)
  {
    std::shuffle(begin(transitions), end(transitions), g);

    const auto next_states = *std::prev(rcit);
    const auto a_state = one_state_of(next_states);
    
    for (const auto transition : transitions)
    {
      const auto res = visit( visitor{transition}
                            , *rcit            // current states to filter
                            , a_state.cbegin() // next state target
                            , functions.cbegin());
      if (res != zero())
      {
        *rcit = res;
        trace.front().first = transition;
        trace.emplace_front("", *rcit);
        break;
      }
    }
  }
  return trace;
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
