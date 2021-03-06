/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

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
#include "mc/classic/threads.hh"
#include "classic.hh"
#include "mc/shared/exceptions.hh"
#include "mc/shared/export.hh"
#include "mc/shared/results.hh"
#include "mc/shared/statistics.hh"
#include "mc/shared/step.hh"
#include "support/pn/constants.hh"
#include "support/util/timer.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

namespace /* unnamed */ {

SDD
initial_state(const sdd::order<sdd_conf>& order, const pn::net& net)
{
  auto timed = std::map<std::string, std::reference_wrapper<const pn::transition>>{};
  boost::range::for_each( net.transitions()
                        , [&](const auto& t){if (t.timed()) timed.emplace(t.name, t);});

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
                        return net.enabled(t_cit->second.get().name)
                             ? flat_set{0}
                             : flat_set{pn::sharp};
                      }}};
}

/*------------------------------------------------------------------------------------------------*/

void
export_results(const conf::configuration& conf, homomorphism h, homomorphism h_opt,
               const results& res, const statistics& stats)
{
  std::cout << "\n\n-- Results\n";
  std::cout << res;

  using conf::filename;
  using dot_sdd = shared::dot_sdd<sdd_conf>;

  shared::export_dot(conf, filename::dot_h, h, filename::dot_h_opt, h_opt);
  shared::export_dot(conf, filename::dot_m0, dot_sdd{*res.m0, *res.order});
  shared::export_dot(conf, filename::dot_final, dot_sdd{*res.states, *res.order});
  shared::export_json(conf, filename::json_stats, stats);
  shared::export_json(conf, filename::json_results, res);
  shared::export_json(conf, filename::json_h, h, filename::json_h_opt, h_opt);
}

} // namespace unnamed

/*------------------------------------------------------------------------------------------------*/

void
classic::operator()(const pn::net& net, const properties::formulae& formulae)
{
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

  if (net.places().empty() and net.transitions().empty())
  {
    auto res = results{};
    auto stats = statistics{};
    res.m0 = zero();
    res.states = zero();
    res.order = make_order(conf, stats, net);
    std::cout << "Empty model.\n";
    export_results(conf, id<sdd_conf>(), id<sdd_conf>(), res, stats);
    return;
  }

  if (not formulae.fireable_transitions.empty())
  {
    conf.compute_dead_transitions = true;
  }
  if (formulae.compute_deadlock)
  {
    conf.compute_dead_states = true;
  }
  if (conf.trace and net.timed())
  {
    std::cerr << "Computation of a trace for Time Petri Nets is not supported yet.\n";
    conf.trace = false;
  }

  using conf::filename;

  auto total_timer = util::timer{};

  auto stats = statistics{};
  stats.max_time = conf.max_time;
  if (conf.stats_conf.count(shared::stats::pn))
  {
    stats.pn_statistics = pn::statistics(net);
  }
  auto res = results{};

  // Set to true by an asynchrous thread when the time limit is reached.
  auto stop_flag = false;

  // Build the order.
  res.order = make_order(conf, stats, net);
  if (res.order->empty())
  {
    throw std::runtime_error("Empty order");
  }
  shared::export_json(conf, filename::json_order, *res.order);

  if (conf.order_only)
  {
    return;
  }

  // Get the initial state.
  res.m0 = initial_state(*res.order, net);

  // Map of live transitions.
  auto live_transitions = boost::dynamic_bitset<>{net.transitions().size()};

  // Compute the transition relation.
  const auto h_operands = [&]
  {
    auto step = shared::step{"firing rule", &stats.relation_duration};
    return firing_rule(conf, *res.order, net, live_transitions, stop_flag);
  }();
  const auto key = [](const auto& kv){return kv.first;}; // Extract keys of a map
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
    auto step = shared::step{"state space", &stats.state_space_duration};
    // Threads will be stopped at scope exit.
    threads _{conf, stats, stop_flag, manager, step.timer};
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
    auto step = shared::step{"count tokens", &*stats.tokens_duration};
    count_tokens(res, *res.states, net);
  }

  if (conf.compute_dead_transitions)
  {
    auto step = shared::step{"dead transitions"};
    res.dead_transitions.emplace();
    for (const auto& t : net.transitions())
    {
      if (not live_transitions[t.uid])
      {
        res.dead_transitions->emplace(t.name);
      }
    }
  }

  if (conf.compute_dead_states)
  {
    {
      stats.dead_states_duration.emplace();
      auto step = shared::step{"dead states", &*stats.dead_states_duration};
      res.dead_states = dead_states(*res.order, net, *res.states);
    }
    if (conf.trace)
    {
      stats.trace_duration.emplace();
      auto step = shared::step{"trace", &*stats.trace_duration};
      res.trace = shortest_path(*res.order, *res.m0, *res.dead_states, net, h_operands);
    }
  }

  if (not formulae.booleans.empty() or not formulae.integers.empty())
  {
    stats.reachability_duration.emplace();
    auto step = shared::step{"reachability", &*stats.reachability_duration};
    reachability(*res.order, formulae, res);
  }

  if (conf.stats_conf.count(shared::stats::final_sdd))
  {
    stats.sdd_statistics = sdd::tools::statistics(*res.states);
  }
  stats.manager_statistics = sdd::tools::statistics(manager);

  stats.total_duration = total_timer.duration();
  std::cout << "total" << std::setw(15) << ": " << stats.total_duration.count() << "s";

  export_results(conf, h, h_opt, res, stats);
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

#pragma GCC diagnostic pop
