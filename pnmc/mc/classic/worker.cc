#include <algorithm> // for_each, sort, transform
#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <set>

#include <sdd/tools/size.hh>

#include "mc/classic/count_tokens.hh"
#include "mc/classic/dead_states.hh"
#include "mc/classic/firing_rule.hh"
#include "mc/classic/make_order.hh"
#include "mc/classic/path_to.hh"
#include "mc/classic/sdd.hh"
#include "mc/classic/sharp_output.hh"
#include "mc/classic/threads.hh"
#include "mc/classic/worker.hh"
#include "mc/shared/dump.hh"
#include "mc/shared/results.hh"
#include "mc/shared/statistics.hh"
#include "mc/shared/exceptions.hh"
#include "shared/util/timer.hh"

namespace pnmc { namespace mc { namespace classic {

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

  return SDD( order
            , [&](const std::string& id) -> flat_set
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

homomorphism
rewrite( const conf::configuration&, const order& o, const homomorphism& h
       , shared::statistics& stats)
{
  util::timer timer;
  const auto res = sdd::rewrite(o, h);
  stats.rewrite_duration = timer.duration();
  return res;
}

/*------------------------------------------------------------------------------------------------*/

SDD
state_space( const conf::configuration& conf, const order& o, SDD m
           , homomorphism h, shared::statistics& stats, bool& stop
           , const sdd::manager<sdd_conf>& manager)
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
  sconf.sdd_unique_table_size = conf.sdd_ut_size;
  sconf.sdd_difference_cache_size = conf.sdd_diff_cache_size;
  sconf.sdd_intersection_cache_size = conf.sdd_inter_cache_size;
  sconf.sdd_sum_cache_size = conf.sdd_sum_cache_size;
  sconf.hom_unique_table_size = conf.hom_ut_size;
  sconf.hom_cache_size = conf.hom_cache_size;
  auto manager_ptr = std::make_unique<sdd::manager<sdd_conf>>(sdd::init(sconf));
  auto& manager = *manager_ptr;

  shared::statistics stats(conf);
  shared::results res(conf);

  // Used in limited time mode.
  bool stop = false;

  // Build the order.
  sdd::order<sdd_conf> o = make_order(conf, stats, net);
  assert(not o.empty() && "Empty order");
  if (conf.order_show)
  {
    std::cout << o << '\n';
  }

  // Get the initial state.
  const SDD m0 = initial_state(o, net);

  // Map of live transitions.
  boost::dynamic_bitset<> transitions_bitset(net.transitions().size());

  // Compute the transition relation.
  const auto h_operands = firing_rule(conf, o, net, transitions_bitset, stats, stop);
  const auto h_classic = fixpoint(sum(o, begin(h_operands), end(h_operands)));
  if (conf.show_relation)
  {
    std::cout << h_classic << '\n';
  }

  // Rewrite the transition relation.
  const auto h = rewrite(conf, o, h_classic, stats);
  if (conf.show_relation)
  {
    std::cout << h << '\n';
  }

  if (conf.order_only)
  {
    dump_json(conf, stats, manager, zero(), net);
    shared::dump_hom(conf, h_classic, h);
    return;
  }

  // Compute the state space.
  auto m = zero();
  try
  {
    m = state_space(conf, o, m0, h, stats, stop, manager);
  }
  catch (const shared::bound_error& e)
  {
    std::cout << "Marking (" << conf.marking_bound << ") reached for place " << e.place << ".\n";
    stats.interrupted = true;
    dump_json(conf, stats, manager, m, net);
    shared::dump_hom(conf, h_classic, h);
    return;
  }
  catch (const shared::interrupted&)
  {
    std::cout << "State space computation interrupted after " << stats.state_space_duration.count()
              << "s.\n";
    stats.interrupted = true;
    dump_json(conf, stats, manager, m, net);
    shared::dump_hom(conf, h_classic, h);
    return;
  }

  if (conf.count_tokens)
  {
    util::timer tokens_start;
    count_tokens(res, m, net);
    stats.tokens_duration = tokens_start.duration();
    std::cout << "maximal number of tokens per marking : " << res.max_token_markings << "\n"
              << "maximal number of tokens in a place : " << res.max_token_places << "\n";
  }

  res.nb_states = m.size();
  stats.nb_states = res.nb_states.template convert_to<long double>();
  std::cout << stats.nb_states << " states\n";

  if (conf.compute_dead_transitions)
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
      std::cout << *std::prev(dead_transitions.cend()) << '\n';
    }
    else
    {
      std::cout << "No dead transitions\n";
    }
  }

  if (conf.compute_dead_states and net.timed())
  {
    std::cerr << "Computation of dead states for Time Petri Nets is not supported yet.\n";
  }
  else if (conf.compute_dead_states)
  {
    const auto deads = dead_states(o, net, m, stats);
    if (deads.empty())
    {
      std::cout << "No dead states\n";
    }
    else
    {
      std::cout << deads.size().template convert_to<long double>() << " dead state(s).\n";
      std::cout << "Paths from the initial marking to the nearest dead states:\n";

      const auto paths = path_to(o, m0, deads, h_operands, stats);

      // Get the identifier of each level (SDD::paths() doesn't give this information).
      std::deque<std::reference_wrapper<const std::string>> identifiers;
      o.flat(std::back_inserter(identifiers));

      std::cout << "---------------------\n";
      for (const auto& x : paths)
      {
        // We can't use the range-based for loop as it produces an ambiguity with clang when
        // using Boost 1.56.
        auto path_generator = x.paths();
        while (path_generator)
        {
          const auto& path = path_generator.get();
          path_generator(); // advance generator
          auto id_cit = identifiers.cbegin();
          auto path_cit = path.cbegin();
          for (; path_cit != std::prev(path.cend()); ++path_cit, ++id_cit)
          {
            std::cout << id_cit->get() << " : " << *path_cit << ", ";
          }
          std::cout << id_cit->get() << " : " << *path_cit << '\n';
        }
        std::cout << "---------------------\n";
      }
    }
  }

  const auto total = stats.relation_duration + stats.rewrite_duration
                   + stats.state_space_duration + stats.dead_states_duration;
  std::cout << total.count() << "s\n";

  if (conf.show_time)
  {
    std::cout << "Relation             : " << stats.relation_duration.count() << "s\n"
              << "Rewrite              : " << stats.rewrite_duration.count() << "s\n"
              << "State space          : " << stats.state_space_duration.count() << "s\n";
    if (conf.compute_dead_states)
    {
      std::cout << "Dead states relation : " << stats.dead_states_relation_duration.count() << "s\n"
                << "Dead states rewrite  : " << stats.dead_states_rewrite_duration.count() << "s\n"
                << "Dead states          : " << stats.dead_states_duration.count() << "s\n";
    }
    if (conf.order_ordering_force)
    {
      std::cout << "FORCE                : " << stats.force_duration.count() << "s\n";
    }
  }

  if (conf.show_final_sdd_bytes)
  {
    std::cout << "Final SDD size: " << sdd::tools::size(m) << " bytes\n";
  }

  stats.total_duration = total_timer.duration();

  shared::dump_sdd_dot(conf, m, o);
  shared::dump_json(conf, stats, manager, m, net);
  shared::dump_results(conf, res);
  shared::dump_hom(conf, h_classic, h);

  if (conf.fast_exit)
  {
    manager_ptr.release();
  }
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
