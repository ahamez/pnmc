#include <cassert>
#include <iostream>

#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/algorithm/for_each.hpp>

#include "mc/classic/count_tokens.hh"
#include "mc/classic/dead_states.hh"
#include "mc/classic/firing_rule.hh"
#include "mc/classic/make_order.hh"
#include "mc/classic/path_to.hh"
#include "mc/classic/reachability.hh"
#include "mc/classic/sdd.hh"
#include "mc/classic/sharp_output.hh"
#include "mc/classic/threads.hh"
#include "mc/classic/worker.hh"
#include "mc/shared/exceptions.hh"
#include "mc/shared/export.hh"
#include "mc/shared/results.hh"
#include "mc/shared/statistics.hh"
#include "mc/shared/step.hh"
#include "support/util/timer.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

SDD
initial_state(const sdd::order<sdd_conf>& order, const pn::net& net)
{
  std::map<std::string, std::reference_wrapper<const pn::transition>> timed;
  boost::range::for_each( net.transitions()
                        , [&](const auto& t){if (t.timed()) timed.emplace(t.id, t);});

  return {order, [&](const auto& id)
                    {
                      const auto cit = net.places().find(id);
                      if (cit != end(net.places()))
                      {
                        return flat_set{cit->marking};
                      }
                      else
                      {
                        const auto t_cit = timed.find(id);
                        assert(t_cit != timed.end());
                        return net.enabled(t_cit->second.get().id)
                             ? flat_set{0}
                             : flat_set{pn::sharp};
                      }}};
}

/*------------------------------------------------------------------------------------------------*/

void
worker::operator()(const pn::net& net, const properties::formulae& formulae)
{
  if (not formulae.fireable_transitions.empty())
  {
    conf.compute_dead_transitions = true;
  }
  if (formulae.compute_deadlock)
  {
    conf.compute_dead_states = true;
  }
  if (conf.compute_dead_states and net.timed())
  {
    std::cerr << "Computation of dead states for Time Petri Nets is not supported yet.\n";
    conf.compute_dead_states = false;
  }
  if (conf.trace and net.timed())
  {
    std::cerr << "Computation of a trace for Time Petri Nets is not supported yet.\n";
    conf.trace = false;
  }

  using conf::filename;
  using dot_sdd = shared::dot_sdd<sdd_conf>;

  util::timer total_timer;

  // Configure libsdd.
  sdd_conf sconf;
  sconf.sdd_unique_table_size = conf.ut_sizes.at("sdd");
  sconf.sdd_difference_cache_size = conf.cache_sizes.at("diff");
  sconf.sdd_intersection_cache_size = conf.cache_sizes.at("inter");
  sconf.sdd_intersection_cache_size = conf.cache_sizes.at("sum");
  sconf.hom_unique_table_size = conf.ut_sizes.at("hom");
  sconf.hom_cache_size = conf.cache_sizes.at("hom");

  // This pointer WILL NOT be deleted to avoid a too long shutdown time due to big hash table
  // cleanups.
  auto manager_ptr = new sdd::manager<sdd_conf>{sdd::init(sconf)};
  auto& manager = *manager_ptr;

  auto stats = statistics{};
  stats.max_time = conf.max_time;
  if (conf.stats_conf.count(shared::stats::pn))
  {
    stats.pn_statistics = pn::statistics(net);
  }
  auto res = results{};

  // Set to true by an asynchrous thread when the time limit is reached.
  bool stop_flag = false;

  std::cout << "\n-- Steps\n";

  // Build the order.
  res.order = make_order(conf, stats, net);
  if (res.order->empty())
  {
    throw std::runtime_error("Empty order");
  }
  shared::export_json(conf, filename::json_order, *res.order);

  // Get the initial state.
  res.m0 = initial_state(*res.order, net);

  // Map of live transitions.
  boost::dynamic_bitset<> live_transitions(net.transitions().size());

  // Compute the transition relation.
  const auto h_operands = [&]
  {
    shared::step step("firing rule", &stats.relation_duration);
    return firing_rule(conf, *res.order, net, live_transitions, stop_flag);
  }();
  const auto key = [](const auto& kv){return kv.first;};
  const auto h = fixpoint(sum( *res.order
                             , boost::make_transform_iterator(begin(h_operands), key)
                             , boost::make_transform_iterator(end(h_operands), key)));

  // Rewrite the transition relation.
  const auto h_opt = [&]
  {
    shared::step step("rewrite", &stats.rewrite_duration);
    return sdd::rewrite(*res.order, h);
  }();

  // Compute the state space.
  {
    shared::step step{"state space", &stats.state_space_duration};
    threads _{conf, stats, stop_flag, manager, step.timer}; // threads will be stopped at scope exit
    try
    {
      res.states = h_opt(*res.order, *res.m0);
    }
    catch (const shared::bound_error& e)
    {
      stats.interrupted = true;
      std::cout << "Place " << e.place << " marking >=" << conf.marking_bound << '\n';
    }
    catch (const shared::interrupted&)
    {
      stats.interrupted = true;
      std::cout << "Computation interrupted after " << step.timer.duration().count() << "s\n";
    }
    if (stats.interrupted)
    {
      shared::export_json(conf, filename::json_stats, stats);
      shared::export_dot(conf, filename::dot_h, h, filename::dot_h_opt, h_opt);
      return;
    }
  }

  if (conf.count_tokens)
  {
    stats.tokens_duration.emplace();
    shared::step step{"count tokens", &*stats.tokens_duration};
    count_tokens(res, *res.states, net);
  }

  if (conf.compute_dead_transitions)
  {
    shared::step step{"dead transitions"};
    res.dead_transitions.emplace();
    for (std::size_t i = 0; i < net.transitions().size(); ++i)
    {
      if (not live_transitions[i])
      {
        res.dead_transitions->emplace(net.get_transition_by_index(i).id);
      }
    }
  }

  if (conf.compute_dead_states)
  {
    {
      stats.dead_states_duration.emplace();
      shared::step step{"dead states", &*stats.dead_states_duration};
      res.dead_states = dead_states(*res.order, net, *res.states);
    }
    if (conf.trace)
    {
      stats.trace_duration.emplace();
      shared::step step{"trace", &*stats.trace_duration};
      res.trace = shortest_path(*res.order, *res.m0, *res.dead_states, net, h_operands);
    }
  }

  if (not formulae.booleans.empty() or not formulae.integers.empty())
  {
    stats.reachability_duration.emplace();
    shared::step step{"reachability", &*stats.reachability_duration};
    reachability(*res.order, formulae, res);
  }

  if (conf.stats_conf.count(shared::stats::final_sdd))
  {
    stats.sdd_statistics = sdd::tools::statistics(*res.states);
  }
  stats.manager_statistics = sdd::tools::statistics(manager);

  stats.total_duration = total_timer.duration();
  std::cout << "total" << std::setw(15) << ": " << stats.total_duration.count() << "s";
  std::cout << "\n\n-- Results\n";
  std::cout << res;

  shared::export_dot(conf, filename::dot_h, h, filename::dot_h_opt, h_opt);
  shared::export_dot(conf, filename::dot_m0, dot_sdd{*res.m0, *res.order});
  shared::export_dot(conf, filename::dot_final, dot_sdd{*res.states, *res.order});
  shared::export_json(conf, filename::json_stats, stats);
  shared::export_json(conf, filename::json_results, res);
  shared::export_json(conf, filename::json_h, h, filename::json_h_opt, h_opt);
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

#pragma GCC diagnostic pop
