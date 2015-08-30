/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <algorithm>  // any_of
#include <functional> // reference_wrapper
#include <stdexcept>
#include <string>
#include <vector>

#include "mc/classic/advance.hh"
#include "mc/classic/advance_capped.hh"
#include "mc/classic/bounded_post.hh"
#include "mc/classic/enabled.hh"
#include "mc/classic/enabled_inhibitor.hh"
#include "mc/classic/filter_ge.hh"
#include "mc/classic/filter_lt.hh"
#include "mc/classic/firing_rule.hh"
#include "mc/classic/post.hh"
#include "mc/classic/pre.hh"
#include "mc/classic/pre_clock.hh"
#include "mc/classic/sdd.hh"
#include "mc/classic/set.hh"
#include "mc/shared/interruptible.hh"
#include "mc/shared/live.hh"
#include "support/pn/constants.hh"

namespace pnmc { namespace mc { namespace classic { namespace /* unnamed */ {

/*------------------------------------------------------------------------------------------------*/

std::string
unique_label(const pn::net& net, const std::string& base_name)
{
  auto name = base_name;
  auto index = 0ul;
  while (net.transitions().find(name) != net.transitions().end())
  {
    name = base_name + std::to_string(index++);
  }
  return name;
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Compute the transition relation corresponding to a petri net.
std::multimap<homomorphism, std::string>
untimed( const conf::configuration& conf, const order& o, const pn::net& net
       , boost::dynamic_bitset<>& transitions_bitset, const bool& stop)
{
  // Store created operations and associate them with a label.
  auto operands = std::multimap<homomorphism, std::string>{};

  // An operation will be created for Id. We thus have to generate a name that is not
  // the one of a transition.
  const auto id_name = unique_label(net, "id");

  // The id operation for the transitive closure.
  operands.emplace(sdd::id<sdd_conf>(), id_name);

  using target_arc_type = pn::transition::arcs_type::value_type;

  // Temporary storage to sort post and pre arcs.
  auto arcs = std::vector<std::reference_wrapper<const target_arc_type>>{};
  arcs.reserve(128);

  for (const pn::transition& transition : net.transitions())
  {
    if (transition.pre.empty() and transition.post.empty())
    {
      continue; // A transition with no pre or post places, no need to keep it.
    }

    // Sort post arcs using the variable order.
    arcs.clear();
    std::copy(begin(transition.post), end(transition.post), std::back_inserter(arcs));
    std::sort( arcs.begin(), arcs.end()
             , [&](const target_arc_type& lhs, const target_arc_type& rhs)
                  {
                    return o.node(rhs.first) < o.node(lhs.first);
                  });

    // compose from left to right
    auto h_t = sdd::id<sdd_conf>();

    // Post actions.
    for (const target_arc_type& arc : arcs)
    {
      // Is the maximal marking limited?
      const auto f = conf.marking_bound == 0
                   ? function(o, arc.first, post{arc.second.weight})
                   : function(o, arc.first, bounded_post{ arc.second.weight, conf.marking_bound
                                                        , arc.first});
      h_t = composition(h_t, f);
    }

    // Add a "canary" to detect live transitions. It will be triggered if all pre are fired.
    if (conf.compute_dead_transitions)
    {
      if (transition.pre.empty())
      {
        // t is always enabled, it's useless to test if it's live.
        transitions_bitset[transition.uid] = true;
      }
      else
      {
        // Target the same variable as the last pre or post to be fired to avoid evaluations.
        const auto var = transition.pre.cbegin()->first;
        const auto f = function(o, var, shared::live{transition.uid, transitions_bitset});
        h_t = composition(h_t, f);
      }
    }

    // Sort pre arcs using the variable order.
    arcs.clear();
    std::copy(begin(transition.pre), end(transition.pre), std::back_inserter(arcs));
    std::sort( arcs.begin(), arcs.end()
             , [&](const target_arc_type& lhs, const target_arc_type& rhs)
                  {
                    return o.node(rhs.first) < o.node(lhs.first);
                  });

    // Reset arcs.
    for (const target_arc_type& arc : arcs)
    {
      if (arc.second.kind == pn::arc::type::reset)
      {
        const auto f = function(o, arc.first, set{0});
        h_t = composition(h_t, f);
      }
    }

    // Pre actions.
    for (auto cit = begin(arcs); cit != end(arcs); ++cit)
    {
      const target_arc_type& arc = *cit;
      if (arc.second.kind == pn::arc::type::reset)
      {
        continue;
      }

      const auto weight = arc.second.weight;
      const auto& id = arc.first;
      const auto f = [&]
      {
        // Make last pre arc interruptible if required by the configuration.
        if (std::next(cit) == end(arcs) and conf.max_time > std::chrono::duration<double>(0))
        {
          switch (arc.second.kind)
          {
            case pn::arc::type::normal:
              return function(o, id, shared::interruptible<sdd_conf, pre>(stop, weight));

            case pn::arc::type::inhibitor:
              return function(o, id, shared::interruptible<sdd_conf, filter_lt>(stop, weight));

            case pn::arc::type::read:
              return function(o, id, shared::interruptible<sdd_conf, filter_ge>(stop, weight));

            default:
              __builtin_unreachable();
          }
        }
        else
        {
          switch (arc.second.kind)
          {
            case pn::arc::type::normal:
              return function(o, id, pre{weight});

            case pn::arc::type::inhibitor:
              return function(o, id, filter_lt{weight});

            case pn::arc::type::read:
              return function(o, id, filter_ge{weight});

            default:
              __builtin_unreachable();
          }
        }
      }();
      h_t = composition(h_t, f);
    }

    operands.emplace(h_t, transition.name);
  }
  return operands;
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Compute the transition relation corresponding to a petri net.
std::multimap<homomorphism, std::string>
timed( const conf::configuration& conf, const order& o, const pn::net& net
     , boost::dynamic_bitset<>& transitions_bitset, const bool& stop)
{
  // Store created operations and associate them with a label.
  auto operands = std::multimap<homomorphism, std::string>{};

  // An operation will be created to advance time. We thus have to generate a name that is not
  // the one of a transition.
  const auto advance_time_name = unique_label(net, "advance_time");

  // An operation will be created for Id. We thus have to generate a name that is not
  // the one of a transition.
  const auto id_name = unique_label(net, "id");

  // The id operation for the transitive closure.
  operands.emplace(sdd::id<sdd_conf>(), id_name);

  using target_arc_type = pn::transition::arcs_type::value_type;

  // Temporary storage to sort post and pre arcs.
  auto arcs = std::vector<std::reference_wrapper<const target_arc_type>>{};
  arcs.reserve(128);

  for (const pn::transition& t : net.transitions())
  {
    // Compose from right to left into this homomorphism.
    auto h_t = sdd::id<sdd_conf>();

    // Check if the clock of t has reached the low time requirement.
    // An untimed transition doesn't have a clock.
    if (t.timed())
    {
      h_t = function(o, t.name, pre_clock{t.low});
    }

    // Sort pre arcs using the variable order.
    arcs.clear();
    std::copy(begin(t.pre), end(t.pre), std::back_inserter(arcs));
    std::sort( arcs.begin(), arcs.end()
             , [&](const target_arc_type& lhs, const target_arc_type& rhs)
                  {
                    return o.node(rhs.first) < o.node(lhs.first);
                  });

    // Pre actions.
    for (auto cit = begin(arcs); cit != end(arcs); ++cit)
    {
      const target_arc_type& arc = *cit;
      if (arc.second.kind == pn::arc::type::reset)
      {
        continue;
      }

      const auto weight = arc.second.weight;
      const auto& id = arc.first;
      const auto f = [&]
      {
        // Make last pre arc interruptible if required by the configuration.
        if (std::next(cit) == end(arcs) and conf.max_time > std::chrono::duration<double>(0))
        {
          switch (arc.second.kind)
          {
            case pn::arc::type::normal:
              return function(o, id, shared::interruptible<sdd_conf, pre>(stop, weight));

            case pn::arc::type::inhibitor:
              return function(o, id, shared::interruptible<sdd_conf, filter_lt>(stop, weight));

            case pn::arc::type::read:
              return function(o, id, shared::interruptible<sdd_conf, filter_ge>(stop, weight));

            default:
              __builtin_unreachable();
          }
        }
        else
        {
          switch (arc.second.kind)
          {
            case pn::arc::type::normal:
              return function(o, id, pre{weight});

            case pn::arc::type::inhibitor:
              return function(o, id, filter_lt{weight});

            case pn::arc::type::read:
              return function(o, id, filter_ge{weight});

            default:
              __builtin_unreachable();
          }
        }
      }();
      h_t = composition(f, h_t);
    }

    // Reset arcs.
    for (const target_arc_type& arc : arcs)
    {
      if (arc.second.kind == pn::arc::type::reset)
      {
        const auto f = function(o, arc.first, set{0});
        h_t = composition(f, h_t);
      }
    }

    // Add a "canary" to detect live transitions. It will be triggered if all pre are fired.
    if (conf.compute_dead_transitions)
    {

      if (t.pre.empty())
      {
        // t is always enabled, it's useless to test if it's live.
        transitions_bitset[t.uid] = true;
      }
      else
      {
        // Target the same variable as the last pre to be fired to avoid useless evaluations.
        const auto var = t.pre.crbegin()->first;
        const auto f = function(o, var, shared::live{t.uid, transitions_bitset});
        h_t = composition(f, h_t);
      }
    }

    const bool has_impact_on_time
      = t.timed()
        // The transition t is untimed, but we should check if the places it marks are pre of
        // some timed transition.
        or
        std::any_of( t.post.cbegin(), t.post.cend()
                   , [&](const auto& arc)
                     {
                       const auto& p = *net.places().find(arc.first);
                       return std::any_of( p.post.cbegin(), p.post.cend()
                                         , [&](const auto& arc2)
                                           {
                                             const auto& u = *net.transitions().find(arc2.first);
                                             return u.timed();
                                           });
                     })
        // The transition t is untimed, but we should check if the places it take tokens from are
        // pre of some timed transition.
        or
        std::any_of( t.pre.cbegin(), t.pre.cend()
                   , [&](const auto& arc)
                     {
                       const auto& p = *net.places().find(arc.first);
                       return std::any_of( p.post.cbegin(), p.post.cend()
                                         , [&](const auto& arc2)
                                           {
                                             const auto& u = *net.transitions().find(arc2.first);
                                             return u.timed();
                                           });
                     });

    if (not has_impact_on_time)
    {
      goto post_and_advance_time;
    }

    // Update clocks of all other timed transitions.
    for (const pn::transition& u : net.transitions())
    {
      if (not u.timed())
      {
        // We don't need to check if firing t enables u and to set clocks accordingly.
        continue;
      }

      // Indicates that u has an inhibitor arc to a place shared with t.
      auto u_has_inhibitor = false;

      // Indicates that t has a read arc to place shared with u.
      auto t_has_read = false;

      // Pre places of u which are post of t.
      auto shared_pre_post = std::vector<std::string>{};

      // Pre places of u which are not post of t.
      auto unshared_pre_post = std::vector<std::string>{};

      // Pre places of u which are also pre of t.
      auto shared_pre = std::vector<std::string>{};

      for (const auto& u_pre_arc : u.pre)
      {
        const auto& u_pre_place = u_pre_arc.first;

        if (t.post.find(u_pre_place) != end(t.post)) // pre of u exists in t.post
        {
          shared_pre_post.emplace_back(u_pre_place);
        }
        else
        {
          unshared_pre_post.emplace_back(u_pre_place);
        }

        const auto t_pre_find = t.pre.find(u_pre_place);
        if (t_pre_find != t.pre.cend()) // pre of u exists in t.pre
        {
          shared_pre.emplace_back(u_pre_place);
          if (t_pre_find->second.kind == pn::arc::type::read)
          {
            t_has_read = true;
          }

          if (u_pre_arc.second.kind == pn::arc::type::inhibitor)
          {
            u_has_inhibitor = true;
          }
        }
      }

      if (shared_pre_post.empty() and shared_pre.empty())
      {
        // Major optimization.
        // Transitions t and u don't share any places. Then firing t won't have any impact on the
        // clock of u. Thus, we don't have to test its status and update its clock if necessary.
        continue; // to next transition u
      }

      // If the Petri net is 1-safe and if t cannot mark a pre place of u, then u can't be
      // newly enabled neither persistent, as t will consume tokens of some pre places of u.
      if (conf.one_safe and shared_pre_post.empty() and not u_has_inhibitor and not t_has_read)
      {
        h_t = composition(function(o, u.name, set{pn::sharp}), h_t);
        continue; // to next u
      }

      const auto u_is_enabled = [&]
      {
        auto enabled_predicate = sdd::id<sdd_conf>();

        // Check if existing marking of pre (unshared with t) places of u is sufficient.
        for (const auto& pre_place_id : unshared_pre_post)
        {
          const auto& u_arc = *u.pre.find(pre_place_id);
          const auto e = [&]
          {
            switch (u_arc.second.kind)
            {
              case pn::arc::type::normal:
              case pn::arc::type::read:
                return function(o, u_arc.first, filter_ge{u_arc.second.weight});

              case pn::arc::type::inhibitor:
                return function(o, u_arc.first, filter_lt{u_arc.second.weight});

              default:
                throw std::runtime_error("Unsupported arc type.");
            }
          }();
          enabled_predicate = composition(e, enabled_predicate);
        }

        // Check if pre places (of u) potentially marked by t enable u, by "simulating" Post(t).
        for (const auto& pid : shared_pre_post)
        {
          const auto& t_arc = *t.post.find(pid);
          const auto& u_arc = *u.pre.find(pid);
          const auto e = [&]
          {
            switch (u_arc.second.kind)
            {
              case pn::arc::type::normal:
              case pn::arc::type::read:
                return function( o, u_arc.first
                               , enabled{u_arc.second.weight, t_arc.second.weight});

              case pn::arc::type::inhibitor:
                return function( o, u_arc.first
                               , enabled_inhibitor{u_arc.second.weight, t_arc.second.weight});

              default:
                throw std::runtime_error("Unsupported arc type.");
            }
          }();
          enabled_predicate = composition(e, enabled_predicate);
        }

        return enabled_predicate;
      }();

      const auto u_persistence_possible = [&]
      {
        if (t.uid == u.uid) // by convention, t cannot be persistent vs itself.
        {
          return false;
        }

        for (const auto& t_arc : t.pre)
        {
          const auto u_search = u.pre.find(t_arc.first);
          if (   u_search != u.pre.cend()
             and u_search->second.kind == pn::arc::type::inhibitor
             and t_arc.second.kind != pn::arc::type::inhibitor)
          {
            // If t_arc is not inhibitor and u_arc is inhibitor and if t_arc needs more tokens
            // than u_arc permits, then u can never be persistent vs t. Indeed, to be enabled, t
            // will always need more tokens than the number required by u to be enabled.
            if (t_arc.second.weight >= u_search->second.weight)
            {
              return false;
            }
          }
        }

        return true;
      }();

      const auto enabled_branch = [&]
      {
        if (u_persistence_possible)
        {
          // The predicate that checks if u is persistent vs t (pre of t have already been fired).
          const auto u_is_persistent = [&]
          {
            // Transition u might be always persistent.
            if (shared_pre_post.empty())
            {
              bool u_necessarily_persistent = true;
              for (const auto& shared_pre_place_id : shared_pre)
              {
                const auto u_search = u.pre.find(shared_pre_place_id);
                assert(u_search != u.pre.cend());
                if (u_search->second.kind == pn::arc::type::inhibitor)
                {
                  u_necessarily_persistent = false;
                  break;
                }
              }
              if (u_necessarily_persistent)
              {
                // Transition u is enabled at m' and t doesn't mark a pre place of u. Thus, u is
                // necessarily persistent.
                return sdd::id<sdd_conf>();
              }
            }

            auto persistence = sdd::id<sdd_conf>();

            for (const auto& u_arc : u.pre)
            {
              const auto f = [&]
              {
                switch (u_arc.second.kind)
                {
                  // Pre(t) has already been fired. As we are in the enabled branch of u, it means
                  // that u was already enabled before Pre(t).
                  case pn::arc::type::normal:
                  case pn::arc::type::read:
                    return function(o, u_arc.first, filter_ge{u_arc.second.weight});

                  case pn::arc::type::inhibitor:
                  {
                    const auto t_pre_search = t.pre.find(u_arc.first);
                    if (t_pre_search == t.pre.cend()) // not a common pre place of t and u
                    {
                      return function(o, u_arc.first, filter_lt{u_arc.second.weight});
                    }
                    else
                    {
                      const auto& t_arc = *t_pre_search;
                      if (t_arc.second.kind == pn::arc::type::inhibitor)
                      {
                        // If t_arc is also an inhibitor, we don't have to "put back" tokens.
                        return function(o, u_arc.first, filter_lt{u_arc.second.weight});
                      }
                      else
                      {
                        return function( o, u_arc.first, enabled_inhibitor{ u_arc.second.weight
                                                                          , t_arc.second.weight});
                      }
                    }
                  }

                  default:
                    __builtin_unreachable();
                }
              }();
              persistence = composition(f, persistence);
            } // for (const auto& u_arc : u.pre)
            return persistence;
          }();

          return if_then_else(u_is_persistent, sdd::id<sdd_conf>(), function(o, u.name, set{0}));
        }
        else // not u_persistence_possible
        {
          return function(o, u.name, set{0});
        }
      }(); // enabled_branch

      const auto ite_u
        = if_then_else(u_is_enabled, enabled_branch, function(o, u.name, set{pn::sharp}));

      h_t = composition(ite_u, h_t);

    } // for (const pn::transition& u : net.transitions())

post_and_advance_time:

    // Sort post arcs using the variable order.
    arcs.clear();
    std::copy(begin(t.post), end(t.post), std::back_inserter(arcs));
    std::sort( arcs.begin(), arcs.end()
             , [&](const target_arc_type& lhs, const target_arc_type& rhs)
                  {
                    return o.node(rhs.first) < o.node(lhs.first);
                  });

    // Finally, apply the post operations.
    auto post_t = sdd::id<sdd_conf>();
    for (const target_arc_type& arc : arcs)
    {
      // Is the maximal marking limited?
      const auto p = conf.marking_bound == 0
                   ? function(o, arc.first, post{arc.second.weight})
                   : function(o, arc.first, bounded_post{ arc.second.weight, conf.marking_bound
                                                        , arc.first});
      post_t = composition(p, post_t);
    }
    h_t = composition(post_t, h_t);

    // The operation for transition t is ready.
    operands.emplace(h_t, t.name);
  } // for (const pn::transition& t : net.transitions())

  const auto advance_time = [&]
  {
    auto res = sdd::id<sdd_conf>();
    for (const pn::transition& t : net.transitions())
    {
      if (t.timed())
      {
        const auto f = t.high == pn::infinity
                     ? function(o, t.name, advance_capped{t.low, t.high})
                     : function(o, t.name, advance{t.high});
        res = composition(f, res);
      }
    }
    return res;
  }();
  operands.emplace(advance_time, advance_time_name);

  return operands;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace unnamed

/*------------------------------------------------------------------------------------------------*/

std::multimap<homomorphism, std::string>
firing_rule( const conf::configuration& conf, const order& o, const pn::net& net
           , boost::dynamic_bitset<>& transitions_bitset, const bool& stop)
{
  return net.timed()
       ? timed(conf, o, net, transitions_bitset, stop)
       : untimed(conf, o, net, transitions_bitset, stop);
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
