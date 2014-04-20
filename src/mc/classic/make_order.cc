#include <algorithm> // random_shuffle, transform
#include <cassert>
#include <deque>
#include <chrono>
#include <fstream>
#include <random>

#include <sdd/order/order.hh>
#include <sdd/order/strategies/force.hh>
#include <sdd/order/strategies/identifiers_per_hierarchy.hh>
#include <sdd/tools/load_order.hh>

#include "mc/classic/dump.hh"
#include "mc/classic/make_order.hh"
#include "util/timer.hh"

namespace pnmc { namespace mc { namespace classic {

using sdd_conf = sdd::conf1;

/*------------------------------------------------------------------------------------------------*/

struct mk_order_visitor
  : public boost::static_visitor<sdd::order_builder<sdd_conf>>
{
  using order_builder = sdd::order_builder<sdd_conf>;
  using result_type = order_builder;

  // Place: base case of the recursion, there's no more possible nested hierarchies.
  result_type
  operator()(const pn::place* p)
  const
  {
    return p->connected() ? order_builder(p->id) : order_builder();
  }

  // Hierarchy.
  result_type
  operator()(const pn::module_node& m)
  const
  {
    assert(not m.nested.empty());
    order_builder local_ob;
    if (m.nested.size() == 1)
    {
      return boost::apply_visitor(*this, *m.nested.front());
    }
    else
    {
      for (const auto& nested_module : m.nested)
      {
        local_ob = local_ob << boost::apply_visitor(*this, *nested_module);
      }
      return order_builder(m.id, local_ob);
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

sdd::order<sdd_conf>
make_order(const conf::configuration& conf, statistics& stats, const pn::net& net)
{
  for (const auto& place : net.places())
  {
    if (not place.connected())
    {
      std::cerr << "Warning: place " << place.id << " is not connected." << std::endl;
    }
  }

  if (conf.load_order_file)
  {
    std::fstream file(*conf.load_order_file);
    if (file.is_open())
    {
      return sdd::tools::load_order<sdd_conf>(file);
    }
    else
    {
      std::cerr << "Can't open JSON order file " << *conf.load_order_file << std::endl;
    }
  }

  sdd::order_builder<sdd_conf> ob;
  if (conf.order_ordering_force)
  {
    util::timer timer;
    using identifier_type = sdd_conf::Identifier;

    // Temporary placeholder for identifiers.
    std::vector<identifier_type> identifiers;

    // Collect identifiers.
    identifiers.reserve(net.places().size());
    for (const auto& place : net.places())
    {
      if (place.connected())
      {
        identifiers.emplace_back(place.id);
      }
    }

    // The hypergraph that stores connections between the places of the Petri net.
    sdd::force::hypergraph<sdd_conf> graph(identifiers.cbegin(), identifiers.cend());

    // This container will be used again.
    identifiers.clear();

    // A new connection is created for each transition of the Petri net.
    for (const auto& transition : net.transitions())
    {
      for (const auto& arc : transition.pre)
      {
        identifiers.emplace_back(arc.first);
      }

      for (const auto& arc : transition.post)
      {
        identifiers.emplace_back(arc.first);
      }
      graph.add_hyperedge(identifiers.cbegin(), identifiers.cend());

      // We use this container again in the next loop.
      identifiers.clear();
    }
    // Apply the FORCE ordering strategy.
    auto force = sdd::force::worker<sdd_conf>(graph, conf.order_reverse);
    ob = force(conf.order_force_iterations);
    stats.force_duration = timer.duration();
    stats.force_spans = force.spans();

    // Dump the hypergraph to a DOT file if required by the configuration.
    dump_hypergraph_dot(conf, graph);
  }
  else if (not conf.order_flat and net.modules)
  {
    ob = boost::apply_visitor(mk_order_visitor(), *net.modules).nested();
  }
  else if (conf.order_random)
  {
    std::vector<std::string> tmp;
    tmp.reserve(net.places().size());
    for (const auto& place : net.places())
    {
      if (place.connected())
      {
          tmp.emplace_back(place.id);
      }
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(tmp.begin(), tmp.end(), g);
    for (const auto& id : tmp)
    {
      ob.push(id);
    }
  }
  else if (conf.order_reverse)
  {
    for (auto rcit = net.places().rbegin(); rcit != net.places().rend(); ++rcit)
    {
      if (rcit->connected())
      {
        ob.push(rcit->id);
      }
    }
  }
  else
  {
    for (const auto& place : net.places())
    {
      if (place.connected())
      {
        ob.push(place.id);
      }
    }
  }

  if (conf.order_id_per_hierarchy > 0 and not conf.order_flat)
  {
    ob = sdd::identifiers_per_hierarchy<sdd_conf>(conf.order_id_per_hierarchy)(ob);
  }
  return sdd::order<sdd_conf>(ob);
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
