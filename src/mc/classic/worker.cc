#include <algorithm> // for_each
#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <set>
#include <thread>

#include <boost/dynamic_bitset.hpp>

#include <sdd/sdd.hh>
#include <sdd/tools/size.hh>

#include "mc/classic/advance.hh"
#include "mc/classic/advance_capped.hh"
#include "mc/classic/bounded_post.hh"
#include "mc/classic/count_tokens.hh"
#include "mc/classic/dead.hh"
#include "mc/classic/dump.hh"
#include "mc/classic/enabled.hh"
#include "mc/classic/exceptions.hh"
#include "mc/classic/filter.hh"
#include "mc/classic/inhibitor.hh"
#include "mc/classic/interruptible.hh"
#include "mc/classic/live.hh"
#include "mc/classic/make_order.hh"
#include "mc/classic/post.hh"
#include "mc/classic/pre.hh"
#include "mc/classic/pre_clock.hh"
#include "mc/classic/results.hh"
#include "mc/classic/set.hh"
#include "mc/classic/statistics.hh"
#include "mc/classic/worker.hh"
#include "util/timer.hh"

namespace pnmc { namespace mc { namespace classic {

namespace chrono = std::chrono;

using sdd_conf = sdd::conf1 ;
using SDD = sdd::SDD<sdd_conf>;
using homomorphism = sdd::homomorphism<sdd_conf>;

using sdd::composition;
using sdd::fixpoint;
using sdd::intersection;
using sdd::sum;
using sdd::function;

/*------------------------------------------------------------------------------------------------*/

SDD
initial_state(const sdd::order<sdd_conf>& order, const pn::net& net)
{
  std::map<std::string, std::reference_wrapper<const pn::transition>> timed;
  for (const auto& t : net.transitions())
  {
    if (t.timed())
    {
      timed.emplace(t.id, t);
    }
  }

  return SDD(order, [&](const std::string& id)
                        -> sdd::values::flat_set<unsigned int>
                       {
                         const auto cit = net.places_by_id().find(id);
                         if (cit != net.places_by_id().end())
                         {
                           return {cit->marking};
                         }
                         else
                         {
                           const auto t_cit = timed.find(id);
                           assert(t_cit != timed.end());
                           if (net.enabled(t_cit->second.get().id))
                           {
                             return {0};
                           }
                           else
                           {
                             return {pn::sharp};
                           }
                         }
                       });
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Create an interruptible function if required by the configuration, a normal function
/// otherwise.
template <typename Fun, typename... Args>
homomorphism
mk_fun( const conf::configuration& conf, const bool& stop, const sdd::order<sdd_conf>& o
      , const sdd_conf::Identifier& id, Args&&... args)
{
  if (conf.max_time > chrono::duration<double>(0))
  {
    return function(o, id, interruptible<sdd_conf, Fun>(stop, std::forward<Args>(args)...));
  }
  else
  {
    return function(o, id, Fun(std::forward<Args>(args)...));;
  }
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Compute the transition relation corresponding to a petri net.
homomorphism
transition_relation( const conf::configuration& conf, const sdd::order<sdd_conf>& o
                   , const pn::net& net, boost::dynamic_bitset<>& transitions_bitset
                   , statistics& stats, const bool& stop)
{
  util::timer timer;

  // Each transition will produce an operand.
  std::set<homomorphism> operands;
  operands.insert(sdd::id<sdd_conf>());

  if (not net.timed())
  {
    for (const pn::transition& transition : net.transitions())
    {
      if (transition.pre.empty() and transition.post.empty())
      {
        continue; // A transition with no pre or post places, no need to keep it.
      }

      // compose from left to right
      auto h_t = sdd::id<sdd_conf>();

      // Post actions.
      for (const auto& arc : transition.post)
      {
        // Is the maximal marking limited?
        homomorphism f = conf.marking_bound == 0
        ? mk_fun<post>(conf, stop, o, arc.first, arc.second.weight)
        : mk_fun<bounded_post<sdd_conf>>( conf, stop, o, arc.first, arc.second.weight
                                         , conf.marking_bound, arc.first);
        h_t = composition(h_t, sdd::carrier(o, arc.first, f));
      }

      // Add a "canary" to detect live transitions. It will be triggered if all pre are fired.
      if (conf.compute_dead_transitions)
      {
        // Target the same variable as the last pre or post to be fired to avoid evaluations.
        const auto var = transition.pre.empty()
        ? std::prev(transition.post.cend())->first
        : transition.pre.cbegin()->first;

        const auto f = mk_fun<live>(conf, stop, o, var, transition.index, transitions_bitset);
        h_t = composition(h_t, sdd::carrier(o, var, f));
      }

      // Pre actions.
      for (const auto& arc : transition.pre)
      {
        const homomorphism f = [&]{
          switch (arc.second.kind)
          {
            case pn::arc::type::normal:
              return mk_fun<pre>(conf, stop, o, arc.first, arc.second.weight);

            case pn::arc::type::inhibitor:
              return mk_fun<inhibitor>(conf, stop, o, arc.first, arc.second.weight);

            default:
              throw std::runtime_error("Unsupported arc type.");
          }
        }();
        h_t = composition(h_t, sdd::carrier(o, arc.first, f));
      }
      
      operands.insert(h_t);
    }
  }
  else // timed Petri net
  {
    // @todo Add canary to detect dead timed transitions

    for (const pn::transition& t : net.transitions())
    {
      // Compose from right to left into this homomorphism.
      auto h_t = sdd::id<sdd_conf>();

      // Check if the clock of t has reached the low time requirement.
      // An untimed transition doesn't have a clock.
      if (t.timed())
      {
        h_t = sdd::carrier(o, t.id, mk_fun<pre_clock>(conf, stop, o, t.id, t.low));
      }

      // All pre arcs.
      for (const auto& arc : t.pre)
      {
        h_t = composition( sdd::carrier( o, arc.first
                                        , function(o, arc.first, pre(arc.second.weight)))
                         , h_t);
      }

      const bool has_impact_on_time
        = t.timed() or
          // The transition t is untimed, but we should check if the places it marks are pre of
          // some timed transition.
          std::any_of( t.post.cbegin(), t.post.cend()
                     , [&](const pn::transition::arcs_type::value_type& arc)
                          {
                            const auto& p = *net.places_by_id().find(arc.first);
                            return std::any_of( p.post.cbegin(), p.post.cend()
                                              , [&](const pn::place::arcs_type::value_type& arc2)
                                                   {
                                                     const auto& u
                                                       = *net.transitions().find(arc2.first);
                                                       return u.timed();
                                                   });
                          });

      if (has_impact_on_time)
      {
        // Update clocks of all other timed transitions.
        for (const pn::transition& u : net.transitions())
        {
          if (not u.timed())
          {
            // We don't need to check if firing t enables u and to set clocks accordingly.
            continue;
          }

          // The operation to perform when u is not persistent.
          const homomorphism not_persistent = [&]
          {
            // Seperate places which are shared between t and u from those which are unshared.
            std::vector<std::string> shared_pre;
            std::vector<std::string> unshared_pre;
            for (const auto& arc : u.pre)
            {
              if (t.post.find(arc.first) != t.post.cend()) // arc.first exists in t.post
              {
                shared_pre.push_back(arc.first);
              }
              else
              {
                unshared_pre.push_back(arc.first);
              }
            }

            if (shared_pre.empty() and t.id != u.id)
            {
              // Transition u doesn't have a pre place that t can marks. Thus, for a given marking,
              // u is either persistent or disabled.
              return sdd::carrier(o, u.id, function(o, u.id, set(pn::sharp)));
            }
            else
            {
              // The predicate that selects markings where the transition u is enabled at m'.
              auto u_enabled = sdd::id<sdd_conf>();

              // Check if existing marking of pre (unshared) places of u is sufficient.
              for (const auto& pid : unshared_pre)
              {
                const auto& arc = *u.pre.find(pid);
                const auto e = function(o, arc.first, filter(arc.second.weight));
                u_enabled = composition(sdd::carrier(o, arc.first, e), u_enabled);
              }

              // Check if pre places (of u) potentially marked by t enable u.
              for (const auto& pid : shared_pre)
              {
                const auto& t_arc = *t.post.find(pid);
                const auto& u_arc = *u.pre.find(pid);
                const auto e = function( o, u_arc.first
                                       , enabled(u_arc.second.weight, t_arc.second.weight));
                u_enabled = composition(sdd::carrier(o, u_arc.first, e), u_enabled);
              }

              return sdd::if_then_else( u_enabled
                                      , sdd::carrier( o, u.id
                                                    , function(o, u.id, set(0)))
                                      , sdd::carrier( o, u.id
                                                    , function( o, u.id, set(pn::sharp))));
            }
          }();

          // Test persistence.
          if (t.id != u.id)
          {
            // The predicate that checks if u is persistent vs t (pre of t have already been fired).
            auto u_persistence = sdd::id<sdd_conf>();
            for (const auto& arc : u.pre)
            {
              const auto e = function(o, arc.first, filter(arc.second.weight));
              u_persistence = composition(sdd::carrier(o, arc.first, e), u_persistence);
            }

            // Finally, the whole operation for the transition u.
            const auto ite_u
              = sdd::if_then_else(u_persistence, sdd::id<sdd_conf>(), not_persistent);

            // Compose it with the operation of the previous transition u.
            h_t = composition(ite_u, h_t);
          }
          else // t.id == u.id, by convention, t cannot be persistent vs itself.
          {
            h_t = composition(not_persistent, h_t);
          }
        } // for (pn::transition& u : shared)
      } // if (has_impact_on_time)

      // Finally, apply the post operations.
      auto post_t = sdd::id<sdd_conf>();
      for (const auto& arc : t.post)
      {
        const auto p = function(o, arc.first, post(arc.second.weight));
        post_t = composition(sdd::carrier(o, arc.first, p), post_t);
      }
      h_t = composition(post_t, h_t);

      // The operation for transition t is ready.
      operands.insert(h_t);

    } // for (const pn::transition& t : net.transitions())

    const auto advance_time = [&]
    {
      auto res = sdd::id<sdd_conf>();
      for (const pn::transition& t : net.transitions())
      {
        if (t.timed())
        {
          const auto f = t.high == pn::inf
                       ? function(o, t.id, advance_capped(t.low, t.high))
                       : function(o, t.id, advance(t.high));
          res = composition(sdd::carrier(o, t.id, f), res);
        }
      }
      return res;
    }();

    operands.insert(advance_time);
  }

  stats.relation_duration = timer.duration();
  return fixpoint(sum(o, operands.cbegin(), operands.cend()));
}

/*------------------------------------------------------------------------------------------------*/

homomorphism
rewrite( const conf::configuration&, const sdd::order<sdd_conf>& o
       , const homomorphism& h, statistics& stats)
{
  util::timer timer;
  const auto res = sdd::rewrite(o, h);
  stats.rewrite_duration = timer.duration();
  return res;
}

/*------------------------------------------------------------------------------------------------*/

struct threads
{
  bool finished;
  std::thread clock;
  std::thread sdd_sampling;

  threads( const conf::configuration& conf, statistics& stats, bool& stop
         , const sdd::manager<sdd_conf>& manager, util::timer& beginnning)
    : finished(false)
    , clock()
    , sdd_sampling()
  {
    if (conf.max_time > chrono::duration<double>(0))
    {
      clock = std::thread([&]
              {
                while (not finished)
                {
                  std::this_thread::sleep_for(std::chrono::milliseconds(100));
                  if (beginnning.duration() >= conf.max_time)
                  {
                    stop = true;
                    break;
                  }
                }
              });
    }

    if (conf.sample_nb_sdd)
    {
      sdd_sampling = std::thread([&]
                     {
                       const auto sample_time = std::chrono::milliseconds(500);
                       auto last = std::chrono::system_clock::now();
                       while (not finished)
                       {
                         std::this_thread::sleep_for(std::chrono::milliseconds(100));
                         auto now = std::chrono::system_clock::now();
                         if ((now - last) >= sample_time)
                         {
                           stats.sdd_ut_size.emplace_back(manager.sdd_stats().size);
                           last = now;
                         }
                       }
                     });
    }
  }

  ~threads()
  {
    finished = true;
    if (clock.joinable())
    {
      clock.join();
    }
    if (sdd_sampling.joinable())
    {
      sdd_sampling.join();
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

SDD
state_space( const conf::configuration& conf, const sdd::order<sdd_conf>& o, SDD m
           , homomorphism h, statistics& stats, bool& stop, const sdd::manager<sdd_conf>& manager)
{
  SDD res;

  // The reference time;
  util::timer beginnning;

  threads stop_threads_on_scope_exit(conf, stats, stop, manager, beginnning);

  util::timer timer;
  try
  {
    res = h(o, m);
  }
  catch (const std::exception& e)
  {
    stats.state_space_duration = timer.duration();
    throw;
  }

  stats.state_space_duration = timer.duration();

  return res;
}

/*------------------------------------------------------------------------------------------------*/

SDD
dead_states( const conf::configuration&, const sdd::order<sdd_conf>& o, const pn::net& net
           , const SDD& state_space, statistics& stats)
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
      const auto h = function(o, arc.first, dead(arc.second.weight));
      or_operands.insert(sdd::carrier(o, arc.first, h));
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
  const auto res = h(o, state_space);
  stats.dead_states_duration = timer.duration();

  return res;
}

/*------------------------------------------------------------------------------------------------*/

worker::worker(const conf::configuration& c)
  : conf(c)
{}

/*------------------------------------------------------------------------------------------------*/

void
worker::operator()(const pn::net& net)
const
{
  util::timer total_timer;

  // Initialize the libsdd.
  sdd_conf sconf;
  sconf.final_cleanup = not conf.fast_exit;
  sconf.sdd_unique_table_size = conf.sdd_ut_size;
  sconf.sdd_difference_cache_size = conf.sdd_diff_cache_size;
  sconf.sdd_intersection_cache_size = conf.sdd_inter_cache_size;
  sconf.sdd_sum_cache_size = conf.sdd_sum_cache_size;
  sconf.hom_unique_table_size = conf.hom_ut_size;
  sconf.hom_cache_size = conf.hom_cache_size;
  auto manager = sdd::init(sconf);

  statistics stats(conf);
  results res(conf);

  // Used in limited time mode.
  bool stop = false;

  // Build the order.
  sdd::order<sdd_conf> o = make_order(conf, stats, net);
  assert(not o.empty() && "Empty order");
  if (conf.order_show)
  {
    std::cout << o << std::endl;
  }

  // Get the initial state.
  const SDD m0 = initial_state(o, net);

  // Map of live transitions.
  boost::dynamic_bitset<> transitions_bitset(net.transitions().size());

  // Compute the transition relation.
  const auto h_classic = transition_relation(conf, o, net, transitions_bitset, stats, stop);
  if (conf.show_relation)
  {
    std::cout << h_classic << std::endl;
  }

  // Rewrite the transition relation.
  const auto h = rewrite(conf, o, h_classic, stats);
  if (conf.show_relation)
  {
    std::cout << h << std::endl;
  }

  if (conf.order_only)
  {
    dump_json(conf, stats, manager, sdd::zero<sdd_conf>(), net);
    dump_hom_dot(conf, h_classic, h);
    return;
  }

  // Compute the state space.
  auto m = sdd::zero<sdd_conf>();
  try
  {
    m = state_space(conf, o, m0, h, stats, stop, manager);
  }
  catch (const bound_error<sdd_conf>& e)
  {
    std::cout << "Marking limit (" << conf.marking_bound << ") reached for place " << e.place << "."
              << std::endl;
    stats.interrupted = true;
    dump_json(conf, stats, manager, m, net);
    dump_hom_dot(conf, h_classic, h);
    return;
  }
  catch (const interrupted<sdd_conf>& e)
  {
    std::cout << "State space computation interrupted after " << stats.state_space_duration.count()
              << "s." << std::endl;
    stats.interrupted = true;
    dump_json(conf, stats, manager, m, net);
    dump_hom_dot(conf, h_classic, h);
    return;
  }

  if (conf.count_tokens)
  {
    util::timer tokens_start;
    count_tokens(res, m, net);
    stats.tokens_duration = tokens_start.duration();
    std::cout << "maximal number of tokens per marking : " << res.max_token_markings << std::endl
              << "maximal number of tokens in a place : " << res.max_token_places << std::endl;
  }

  res.nb_states = m.size();
  stats.nb_states = res.nb_states.template convert_to<long double>();
  std::cout << stats.nb_states << " states" << std::endl;

  if (conf.compute_dead_transitions and net.timed())
  {
    std::cerr << "Computation of dead transitions for Time Petri Nets is not supported yet."
              << std::endl;
  }
  else if (conf.compute_dead_transitions)
  {
    std::deque<std::string> dead_transitions;
    for (std::size_t i = 0; i < net.transitions().size(); ++i)
    {
      if (not transitions_bitset[i])
      {
        dead_transitions.push_back(net.get_transition_by_index(i).id);
      }
    }

    if (not dead_transitions.empty())
    {
      std::cout << dead_transitions.size() << " dead transition(s): ";
      std::copy( dead_transitions.cbegin(), std::prev(dead_transitions.cend())
               , std::ostream_iterator<std::string>(std::cout, ","));
      std::cout << *std::prev(dead_transitions.cend()) << std::endl;
    }
    else
    {
      std::cout << "No dead transitions" << std::endl;
    }
  }

  if (conf.compute_dead_states and net.timed())
  {
    std::cerr << "Computation of dead states for Time Petri Nets is not supported yet."
              << std::endl;
  }
  else if (conf.compute_dead_states)
  {
    const auto dead = dead_states(conf, o, net, m, stats);
    if (dead.empty())
    {
      std::cout << "No dead states" << std::endl;
    }
    else
    {
      std::cout << dead.size().template convert_to<long double>() << " dead state(s):"
                << std::endl;

      // Get the identifier of each level (SDD::paths() doesn't give this information).
      std::deque<std::reference_wrapper<const std::string>> identifiers;
      o.flat(std::back_inserter(identifiers));

      for (const auto& path : dead.paths())
      {
        auto id_cit = identifiers.cbegin();
        auto path_cit = path.cbegin();
        for (; path_cit != std::prev(path.cend()); ++path_cit, ++id_cit)
        {
          std::cout << id_cit->get() << " : " << *path_cit << ", ";
        }
        std::cout << id_cit->get() << " : " << *path_cit << std::endl;
      }
    }
  }

  const auto total = stats.relation_duration + stats.rewrite_duration
                   + stats.state_space_duration + stats.dead_states_duration;
  std::cout << total.count() << "s" << std::endl;

  if (conf.show_time)
  {
    std::cout << "Relation             : " << stats.relation_duration.count() << "s"
              << std::endl
              << "Rewrite              : " << stats.rewrite_duration.count() << "s"
              << std::endl
              << "State space          : " << stats.state_space_duration.count() << "s"
              << std::endl;
    if (conf.compute_dead_states)
    {
      std::cout << "Dead states relation : " << stats.dead_states_relation_duration.count()
                << "s" << std::endl
                << "Dead states rewrite  : " << stats.dead_states_rewrite_duration.count()
                << "s" << std::endl
                << "Dead states          : " << stats.dead_states_duration.count()
                << "s" << std::endl;
    }
    if (conf.order_ordering_force)
    {
      std::cout << "FORCE                : " << stats.force_duration.count() << "s" << std::endl;
    }
  }

  if (conf.show_final_sdd_bytes)
  {
    std::cout << "Final SDD size: " << sdd::tools::size(m) << " bytes" << std::endl;
  }

  stats.total_duration = total_timer.duration();

  dump_sdd_dot(conf, m, o);
  dump_lua(conf, m);
  dump_json(conf, stats, manager, m, net);
  dump_results(conf, res);
  dump_hom_dot(conf, h_classic, h);
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
