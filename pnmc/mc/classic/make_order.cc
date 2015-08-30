/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <algorithm> // copy, shuffle, sort, transform
#include <cassert>
#include <deque>
#include <chrono>
#include <set>
#include <sstream>
#include <vector>

#include <boost/filesystem/fstream.hpp>

#include <sdd/order/strategies/force.hh>
#include <sdd/order/strategies/identifiers_per_hierarchy.hh>
#include <sdd/tools/order.hh>

#include "mc/classic/make_order.hh"
#include "mc/shared/export.hh"
#include "mc/shared/step.hh"
#include "support/util/paths.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

order_builder
make_hierarchical_order(const std::vector<pn::module>& modules)
{
  struct mk_order_visitor
    : public boost::static_visitor<order_builder>
  {
    using result_type = order_builder;

    // Place: base case of the recursion, there's no more possible nested hierarchies.
    result_type
    operator()(const pn::place& p)
    const
    {
      return p.connected() ? order_builder(p.name) : order_builder();
    }

    // Hierarchy.
    result_type
    operator()(const pn::module_node& m)
    const
    {
      assert(not m.all().empty());
      auto local_ob = order_builder{};
      if (m.all().size() == 1)
      {
        return boost::apply_visitor(*this, m.all().front().variant());
      }
      else
      {
        for (const auto& nested_module : m.all())
        {
          local_ob << boost::apply_visitor(*this, nested_module.variant());
        }
        return order_builder(m.id(), local_ob);
      }
    }
  };

  order_builder current;
  for (const auto& sub : modules)
  {
    current << boost::apply_visitor(mk_order_visitor{}, sub.variant());
  }
  return current;
}

/*------------------------------------------------------------------------------------------------*/

order
make_order(const conf::configuration& conf, statistics& stats, const pn::net& net)
{
  for (const auto& place : net.places_by_insertion())
  {
    if (not place.connected())
    {
      std::cerr << "Warning: place " << place.name << " is not connected";
      if (place.marking > 0)
      {
        std::cerr << " and has an initial marking";
      }
      std::cerr << ".\n";
    }
  }

  using identifier_type = sdd_conf::Identifier;

  // Load a pre-computed order from a JSON file and check if it matches the Petri net.
  // We don't try to apply any heuristic on this order.
  if (conf.order_file)
  {
    boost::filesystem::ifstream file(*conf.order_file);
    if (not file.is_open())
    {
      throw std::runtime_error("Can't open JSON order file " + conf.order_file->string());
    }

    const auto ob = sdd::tools::load_order<sdd_conf>(file);
    if (not ob)
    {
      throw std::runtime_error("Empty JSON order file " + conf.order_file->string());
    }
    auto o = order{*ob};

    // Check if loaded order corresponds to the Petri net.
    auto order_identifiers = std::set<std::string>{};
    o.flat(std::inserter(order_identifiers, order_identifiers.end()));

    auto pn_identifiers = boost::container::flat_set<std::string>{};
    std::transform( begin(net.places()), end(net.places())
                  , std::inserter(pn_identifiers, pn_identifiers.end())
                  , [](const pn::place& p){return p.name;});

    if (net.timed())
    {
      for (const auto& t : net.transitions())
      {
        if (t.timed())
        {
          pn_identifiers.insert(t.name);
        }
      }
    }

    auto diff = std::vector<std::string>{};
    diff.reserve(order_identifiers.size());

    std::set_difference( order_identifiers.cbegin(), order_identifiers.cend()
                       , pn_identifiers.cbegin(), pn_identifiers.cend()
                       , std::back_inserter(diff));

    if (not diff.empty())
    {
      std::stringstream ss{};
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
      std::stringstream ss{};
      ss << "The following identifiers from PN don't exist in " << conf.order_file->string()
         << ": ";
      std::copy( diff.cbegin(), std::prev(diff.cend())
                , std::ostream_iterator<std::string>(ss, ", "));
      ss << diff.back();
      throw std::runtime_error(ss.str());
    }

    return o;
  }

  // FORCE heuristic.
  auto ob = order_builder{};
  if (conf.order_ordering_force)
  {
    stats.force_duration.emplace();
    auto s = shared::step{"force", &*stats.force_duration};

    // Temporary placeholder for identifiers.
    auto identifiers = std::vector<identifier_type>{};

    // Collect identifiers.
    identifiers.reserve(net.places_by_insertion().size());
    for (const auto& place : net.places_by_insertion())
    {
      if (place.connected())
      {
        identifiers.emplace_back(place.name);
      }
    }

    // Add clocks of timed transitions.
    for (const auto& transition : net.transitions())
    {
      if (transition.timed())
      {
        identifiers.emplace_back(transition.name);
      }
    }

    // The hypergraph that stores connections between the places of the Petri net.
    auto graph = sdd::force::hypergraph<sdd_conf>{identifiers.cbegin(), identifiers.cend()};

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
        identifiers.emplace_back(transition.name);
      }

      graph.add_hyperedge(identifiers.cbegin(), identifiers.cend());

      // We use this container again in the next loop.
      identifiers.clear();
    }
    // Apply the FORCE ordering strategy.
    auto force = sdd::force::worker<sdd_conf>(graph, conf.order_reverse);
    ob = force(conf.order_force_iterations);
    stats.force_spans = force.spans();

    // Dump the hypergraph to a DOT file if required by the configuration.
    shared::export_dot(conf, conf::filename::dot_force, graph);
  }
  // Lexical order
  else if (conf.order_lexical)
  {
    auto to_sort = std::vector<identifier_type>{};

    for (const auto& place : net.places())
    {
      to_sort.push_back(place.name);
    }

    if (net.timed())
    {
      for (const auto& transition : net.transitions())
      {
        if (transition.timed())
        {
          to_sort.push_back(transition.name);
        }
      }
    }

    if (conf.order_reverse)
    {
      std::sort(to_sort.begin(), to_sort.end());
    }
    else
    {
      std::sort( to_sort.begin(), to_sort.end()
               , [](const auto& lhs, const auto& rhs)
                   {
                     return lhs > rhs;
                   });
    }

    for (const auto& id : to_sort)
    {
      ob.push(id);
    }
  }
  // Use model's hierarchy, if any.
  else if (not conf.order_flat and not net.modules.empty())
  {
    if (net.timed())
    {
      throw std::runtime_error("Hierarchical order for timed PN is not supported yet");
    }
    ob = make_hierarchical_order(net.root_modules);
  }
  // Flat reversed order.
  else if (conf.order_reverse)
  {
    for ( auto rcit = net.places_by_insertion().rbegin(); rcit != net.places_by_insertion().rend()
        ; ++rcit)
    {
      if (rcit->connected())
      {
        ob.push(rcit->name);
      }
    }

    if (net.timed())
    {
      for (const auto& transition : net.transitions())
      {
        if (transition.timed())
        {
          ob.push(transition.name);
        }
      }
    }
  }
  // Flat order.
  else
  {
    for (const auto& place : net.places_by_insertion())
    {
      if (place.connected())
      {
        ob.push(place.name);
      }
    }

    if (net.timed())
    {
      for (const auto& transition : net.transitions())
      {
        if (transition.timed())
        {
          ob.push(transition.name);
        }
      }
    }
  }

  // Constrain hierarchy with a given number of variables per level.
  if (conf.order_id_per_hierarchy > 0 and not conf.order_flat)
  {
    ob = sdd::identifiers_per_hierarchy<sdd_conf>(conf.order_id_per_hierarchy)(ob);
  }
  return order{ob};
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
