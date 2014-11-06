#include <algorithm> // copy, shuffle, transform
#include <cassert>
#include <deque>
#include <chrono>
#include <random>
#include <set>
#include <sstream>

#include <boost/filesystem/fstream.hpp>

#include <sdd/order/order.hh>
#include <sdd/order/strategies/force.hh>
#include <sdd/order/strategies/identifiers_per_hierarchy.hh>
#include <sdd/tools/order.hh>

#include "mc/classic/make_order.hh"
#include "mc/shared/dump.hh"
#include "shared/util/paths.hh"
#include "shared/util/timer.hh"

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
make_order(const conf::configuration& conf, shared::statistics& stats, const pn::net& net)
{
  for (const auto& place : net.places())
  {
    if (not place.connected())
    {
      std::cerr << "Warning: place " << place.id << " is not connected";
      if (place.marking > 0)
      {
        std::cerr << " and has an initial marking";
      }
      std::cerr << "." << std::endl;
    }
  }

  // Load a pre-computed order from a JSON file and check if it matches the Petri net.
  // We don't try to apply any heuristic on this order.
  if (conf.order_file)
  {
    boost::filesystem::ifstream file(*conf.order_file);
    if (file.is_open())
    {
      sdd::order<sdd_conf> o = sdd::tools::load_order<sdd_conf>(file);

      // Check if loaded order corresponds to the Petri net.
      boost::container::flat_set<std::string> order_identifiers;
      o.flat(std::inserter(order_identifiers, order_identifiers.end()));

      boost::container::flat_set<std::string> pn_identifiers;
      std::transform( net.places_by_id().cbegin(), net.places_by_id().cend()
                    , std::inserter(pn_identifiers, pn_identifiers.end())
                    , [](const pn::place& p){return p.id;});

      if (net.timed())
      {
        for (const auto& t : net.transitions())
        {
          if (t.timed())
          {
            pn_identifiers.insert(t.id);
          }
        }
      }

      std::vector<std::string> diff;
      diff.reserve(order_identifiers.size());

      std::set_difference( order_identifiers.cbegin(), order_identifiers.cend()
                         , pn_identifiers.cbegin(), pn_identifiers.cend()
                         , std::back_inserter(diff));

      if (not diff.empty())
      {
        std::stringstream ss;
        ss << "The following identifiers from " << conf.order_file->string()
           << " don't exist in PN: ";
        std::copy( diff.cbegin(), std::prev(diff.cend())
                 , std::ostream_iterator<std::string>(ss, ", "));
        ss << diff.back();
        throw std::runtime_error(ss.str());
      }

      std::set_difference( pn_identifiers.cbegin(), pn_identifiers.cend()
                         , order_identifiers.cbegin(), order_identifiers.cend()
                         , std::back_inserter(diff));

      if (not diff.empty())
      {
        std::stringstream ss;
        ss << "The following identifiers from PN don't exist in " << conf.order_file->string()
           << ": ";
        std::copy( diff.cbegin(), std::prev(diff.cend())
                  , std::ostream_iterator<std::string>(ss, ", "));
        ss << diff.back();
        throw std::runtime_error(ss.str());
      }

      return o;
    }
    else
    {
      throw std::runtime_error("Can't open JSON order file " + conf.order_file->string());
    }
  }

  // FORCE heuristic.
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

    // Add clocks of timed transitions.
    for (const auto& transition : net.transitions())
    {
      if (transition.timed())
      {
        identifiers.emplace_back(transition.id);
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

      if (transition.timed())
      {
        identifiers.emplace_back(transition.id);
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
    shared::dump_hypergraph_dot(conf, graph);
  }
  // Use model's hierarchy, if any.
  else if (not conf.order_flat and net.modules)
  {
    if (net.timed())
    {
      throw std::invalid_argument("Hierarchical order for timed PN is not supported yet");
    }
    ob = boost::apply_visitor(mk_order_visitor(), *net.modules).nested();
  }
  // Random order, mostly used for developpement purposes.
  else if (conf.order_random)
  {
    if (net.timed())
    {
      throw std::invalid_argument("Random order for timed PN is not supported yet");
    }

    std::vector<std::string> tmp;
    tmp.reserve(net.places().size());
    for (const auto& place : net.places())
    {
      if (place.connected())
      {
          tmp.emplace_back(place.id);
      }
    }

    // Add clocks of timed transitions.
    for (const auto& transition : net.transitions())
    {
      if (transition.timed())
      {
        tmp.emplace_back(transition.id);
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
  // Flat reversed order.
  else if (conf.order_reverse)
  {
    for (auto rcit = net.places().rbegin(); rcit != net.places().rend(); ++rcit)
    {
      if (rcit->connected())
      {
        ob.push(rcit->id);
      }
    }

    if (net.timed())
    {
      for (const auto& transition : net.transitions())
      {
        if (transition.timed())
        {
          ob.push(transition.id);
        }
      }
    }
  }
  // Flat order.
  else
  {
    for (const auto& place : net.places())
    {
      if (place.connected())
      {
        ob.push(place.id);
      }
    }

    if (net.timed())
    {
      for (const auto& transition : net.transitions())
      {
        if (transition.timed())
        {
          ob.push(transition.id);
        }
      }
    }
  }

  // Constrain hierarchy with a given number of variables per level.
  if (conf.order_id_per_hierarchy > 0 and not conf.order_flat)
  {
    ob = sdd::identifiers_per_hierarchy<sdd_conf>(conf.order_id_per_hierarchy)(ob);
  }
  return sdd::order<sdd_conf>(ob);
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
