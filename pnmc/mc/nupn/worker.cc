#include <cassert>
#include <iostream>

#include <boost/dynamic_bitset.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/algorithm/for_each.hpp>

#include "mc/nupn/sdd.hh"
#include "mc/nupn/worker.hh"
#include "mc/shared/exceptions.hh"
#include "mc/shared/export.hh"
#include "mc/shared/results.hh"
#include "mc/shared/statistics.hh"
#include "mc/shared/step.hh"
#include "support/util/timer.hh"

namespace pnmc { namespace mc { namespace nupn {

/*------------------------------------------------------------------------------------------------*/

SDD
initial_state(const sdd::order<sdd_conf>&, const pn::net&)
{
  return zero();
}

/*------------------------------------------------------------------------------------------------*/

void
worker::operator()(const pn::net& net, const properties::formulae& formulae)
{
  assert(not net.timed() && "NUPN should not be timed");
  if (not formulae.integers.empty() or not formulae.booleans.empty())
  {
    std::cerr << "Verification of properties for NUPN is not supported yet.\n";
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
//  res.order = make_order(conf, stats, net);
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
//  const auto h_operands = [&]
//  {
//    shared::step step("firing rule", &stats.relation_duration);
//    return firing_rule(conf, *res.order, net, live_transitions, stop_flag);
//  }();
//  const auto key = [](const auto& kv){return kv.first;};
//  const auto h = fixpoint(sum( *res.order
//                             , boost::make_transform_iterator(begin(h_operands), key)
//                             , boost::make_transform_iterator(end(h_operands), key)));

  // Rewrite the transition relation.
//  const auto h_opt = [&]
//  {
//    shared::step step("rewrite", &stats.rewrite_duration);
//    return sdd::rewrite(*res.order, h);
//  }();

//  // Compute the state space.
//  {
//    shared::step step{"state space", &stats.state_space_duration};
//    threads _{conf, stats, stop_flag, manager, step.timer}; // threads will be stopped at scope exit
//    try
//    {
//      res.states = h_opt(*res.order, *res.m0);
//    }
//    catch (const shared::bound_error& e)
//    {
//      stats.interrupted = true;
//      std::cout << "Place " << e.place << " marking >=" << conf.marking_bound << '\n';
//    }
//    catch (const shared::interrupted&)
//    {
//      stats.interrupted = true;
//      std::cout << "Computation interrupted after " << step.timer.duration().count() << "s\n";
//    }
//    if (stats.interrupted)
//    {
//      shared::export_json(conf, filename::json_stats, stats);
//      shared::export_dot(conf, filename::dot_h, h, filename::dot_h_opt, h_opt);
//      return;
//    }
//  }

//  if (conf.stats_conf.count(shared::stats::final_sdd))
//  {
//    stats.sdd_statistics = sdd::tools::statistics(*res.states);
//  }
//  stats.manager_statistics = sdd::tools::statistics(manager);
//
//  stats.total_duration = total_timer.duration();
//  std::cout << "total" << std::setw(15) << ": " << stats.total_duration.count() << "s";
//  std::cout << "\n\n-- Results\n";
//  std::cout << res;
//
//  shared::export_dot(conf, filename::dot_h, h, filename::dot_h_opt, h_opt);
//  shared::export_dot(conf, filename::dot_m0, dot_sdd{*res.m0, *res.order});
//  shared::export_dot(conf, filename::dot_final, dot_sdd{*res.states, *res.order});
//  shared::export_json(conf, filename::json_stats, stats);
//  shared::export_json(conf, filename::json_results, res);
//  shared::export_json(conf, filename::json_h, h, filename::json_h_opt, h_opt);
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::nupn
