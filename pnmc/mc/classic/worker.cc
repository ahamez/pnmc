#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <set>

#include <boost/range/algorithm/for_each.hpp>

#include "mc/classic/count_tokens.hh"
#include "mc/classic/dead.hh"
#include "mc/classic/firing_rule.hh"
#include "mc/classic/make_order.hh"
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
                      const auto cit = net.places_by_id().find(id);
                      if (cit != end(net.places_by_id()))
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

SDD
dead_states(const conf::configuration&, const order& o, const pn::net& net, const SDD& state_space)
{
  std::set<homomorphism> and_operands;
  std::set<homomorphism> or_operands;

  for (const auto& transition : net.transitions())
  {
    // We are only interested in pre actions.
    for (const auto& arc : transition.pre)
    {
      or_operands.insert(function(o, arc.first, dead{arc.second.weight}));
    }

    and_operands.insert(sum(o, or_operands.cbegin(), or_operands.cend()));
    or_operands.clear();
  }
  const auto tmp = intersection(o, and_operands.cbegin(), and_operands.cend());

  return sdd::rewrite(o, tmp)(o, state_space); // compute dead states
}

/*------------------------------------------------------------------------------------------------*/

void
worker::operator()(const pn::net& net)
const
{
  if (conf.compute_dead_states and net.timed())
  {
    std::cerr << "Computation of dead states for Time Petri Nets is not supported yet.\n";
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

  // Initialize libsdd.
  auto manager_ptr = std::make_unique<sdd::manager<sdd_conf>>(sdd::init(sconf));
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
  assert(not res.order->empty() && "Empty order");
  shared::export_json(conf, filename::json_order, *res.order);

  // Get the initial state.
  const SDD m0 = initial_state(*res.order, net);

  // Map of live transitions.
  boost::dynamic_bitset<> transitions_bitset(net.transitions().size());

  // Compute the transition relation.
  const auto h_classic = [&]
  {
    shared::step("firing rule", &stats.relation_duration);
    return firing_rule(conf, *res.order, net, transitions_bitset, stop_flag);
  }();

  // Rewrite the transition relation.
  const auto h = [&]
  {
    shared::step("rewrite", &stats.rewrite_duration);
    return sdd::rewrite(*res.order, h_classic);
  }();

  // Compute the state space.
  auto m = sdd::zero<sdd_conf>();
  {
    shared::step s{"state space", &stats.state_space_duration};
    threads _{conf, stats, stop_flag, manager, s.timer}; // threads will be stopped at scope exit
    try
    {
      m = h(*res.order, m0);
      res.states = m;
    }
    catch (const shared::bound_error& e)
    {
      stats.interrupted = true;
      std::cout << "Place " << e.place << " marking >=" << conf.marking_bound << '\n';
    }
    catch (const shared::interrupted&)
    {
      stats.interrupted = true;
      std::cout << "Computation interrupted after " << s.timer.duration().count() << "s\n";
    }
    if (stats.interrupted)
    {
      shared::export_json(conf, filename::json_stats, stats);
      shared::export_dot(conf, filename::dot_hclassic, h_classic, filename::dot_hrewritten, h);
      return;
    }
  }

  if (conf.count_tokens)
  {
    stats.tokens_duration.emplace();
    shared::step s{"count tokens", &*stats.tokens_duration};
    count_tokens(res, m, net);
  }

  if (conf.compute_dead_transitions)
  {
    shared::step{"dead transitions"};
    res.dead_transitions.emplace();
    for (std::size_t i = 0; i < net.transitions().size(); ++i)
    {
      if (not transitions_bitset[i])
      {
        res.dead_transitions->push_back(net.get_transition_by_index(i).id);
      }
    }
  }

  if (conf.compute_dead_states and not net.timed())
  {
    stats.dead_states_duration.emplace();
    shared::step s{"dead states", &*stats.dead_states_duration};
    res.dead_states = dead_states(conf, *res.order, net, m);
  }

  if (conf.stats_conf.count(shared::stats::final_sdd))
  {
    stats.sdd_statistics = sdd::tools::statistics(m);
  }
  stats.manager_statistics = sdd::tools::statistics(manager);

  stats.total_duration = total_timer.duration();
  std::cout << "total" << std::setw(15) << ": " << stats.total_duration.count() << "s";
  std::cout << "\n\n-- Results\n";
  std::cout << res;

  shared::export_dot(conf, filename::dot_hclassic, h_classic, filename::dot_hrewritten, h);
  shared::export_dot(conf, filename::dot_m0, dot_sdd{m0, *res.order});
  shared::export_dot(conf, filename::dot_final, dot_sdd{m, *res.order});
  shared::export_json(conf, filename::json_stats, stats);
  shared::export_json(conf, filename::json_results, res);
  shared::export_json(conf, filename::json_hclassic, h_classic, filename::json_hrewritten, h);

  if (conf.fast_exit)
  {
    manager_ptr.release(); // manager's destructor won't be called
  }
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

#pragma GCC diagnostic pop
