#include <algorithm> // random_shuffle, transform
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <unordered_map>
#include <utility>  // pair

#include <cereal/archives/json.hpp>

#include <sdd/sdd.hh>
#include <sdd/order/strategies/force.hh>
#include <sdd/order/strategies/force2.hh>
#include <sdd/tools/dot.hh>
#include <sdd/tools/lua.hh>
#include "sdd/tools/sdd_statistics.hh"
#include <sdd/tools/serialization.hh>
#include "sdd/tools/size.hh"

#include "mc/bound_error.hh"
#include "mc/bounded_post.hh"
#include "mc/dead.hh"
#include "mc/live.hh"
#include "mc/post.hh"
#include "mc/pre.hh"
#include "mc/statistics.hh"
#include "mc/timed.hh"
#include "mc/work.hh"

namespace pnmc { namespace mc {

namespace chrono = std::chrono;

typedef sdd::conf1 sdd_conf;
typedef sdd::SDD<sdd_conf> SDD;
typedef sdd::homomorphism<sdd_conf> homomorphism;

using sdd::composition;
using sdd::fixpoint;
using sdd::intersection;
using sdd::sum;
using sdd::function;

/*------------------------------------------------------------------------------------------------*/

struct mk_order_visitor
  : public boost::static_visitor<std::pair<std::string, sdd::order_builder<sdd_conf>>>
{
  using order_identifier = sdd::order_identifier<sdd_conf>;
  using result_type = std::pair<order_identifier, sdd::order_builder<sdd_conf>>;
  using order_builder = sdd::order_builder<sdd_conf>;

  const conf::pnmc_configuration& conf;
  mutable unsigned int artificial_id_counter;

  mk_order_visitor(const conf::pnmc_configuration& c)
    : conf(c), artificial_id_counter(0)
  {}

  // Place: base case of the recursion, there's no more possible nested hierarchies.
  result_type
  operator()(const pn::place* p)
  const noexcept
  {
    return std::make_pair(order_identifier(p->id), order_builder());
  }

  // Hierarchy.
  result_type
  operator()(const pn::module_node& m)
  const noexcept
  {
    assert(not m.nested.empty());

    std::deque<result_type> tmp;

    for (const auto& h : m.nested)
    {
      const auto res = boost::apply_visitor(*this, *h);
      tmp.push_back(res);
    }

    std::size_t height = 0;
    for (const auto& p : tmp)
    {
      if (not p.second.empty())
      {
        height += p.second.height();
      }
      else
      {
        height += 1; // place
      }
    }

    order_builder ob;
    if (height <= conf.order_min_height)
    {
      order_identifier id;
      for (const auto& p : tmp)
      {
        if (not p.second.empty())
        {
          ob = p.second << ob;
        }
        else // place
        {
          ob.push(p.first, p.second);
        }
      }
      return result_type(id, ob);
    }
    else
    {
      for (const auto& p : tmp)
      {
        ob.push(p.first, p.second);
      }
    }

    return std::make_pair(order_identifier(m.id) , ob);
  }
};

/*------------------------------------------------------------------------------------------------*/

sdd::order<sdd_conf>
mk_order(const conf::pnmc_configuration& conf, const pn::net& net)
{
  if (conf.order_ordering_force_pn)
  {
    using id_type = sdd_conf::Identifier;
    using vertex = sdd::force::vertex<id_type>;
    using hyperedge = sdd::force::hyperedge<id_type>;

    // Rapidly find the vertex associated to an identifier.
    std::unordered_map<id_type, vertex*> map;

    // Construct all vertices.
    std::vector<vertex> vertices;
    vertices.reserve(net.places().size());
    unsigned int location = 0;
    for (const auto& place : net.places())
    {
      vertices.emplace_back(place.id, location++);
      map.emplace(place.id, &vertices.back());
    }

    // Construct all hyperedges.
    std::vector<hyperedge> hyperedges;
    hyperedges.reserve(net.transitions().size());
    for (const auto& transition : net.transitions())
    {
      std::vector<vertex*> local_vertices;

      for (const auto& arc : transition.pre)
      {
        assert(map.find(arc.first) != map.end());
        local_vertices.emplace_back(map[arc.first]);
      }

      for (const auto& arc : transition.post)
      {
        assert(map.find(arc.first) != map.end());
        local_vertices.emplace_back(map[arc.first]);
      }

      // Create the new hyperedge.
      hyperedges.emplace_back(std::move(local_vertices));

      // Update vertices.
      for (auto v_ptr : hyperedges.back().vertices)
      {
        v_ptr->hyperedges.emplace_back(&hyperedges.back());
      }
    }
    return sdd::force_ordering2<sdd_conf>(std::move(vertices), std::move(hyperedges));
  } else
  if (not conf.order_force_flat and net.modules)
  {
    return sdd::order<sdd_conf>(boost::apply_visitor(mk_order_visitor(conf), *net.modules).second);
  }
  else
  {
    sdd::order_builder<sdd_conf> ob;
    if (conf.order_random)
    {
      std::vector<std::string> tmp;
      tmp.reserve(net.places().size());
      std::transform( net.places().cbegin(), net.places().cend(), std::back_inserter(tmp)
                    , [](const pn::place& p){return p.id;});
      std::random_device rd;
      std::mt19937 g(rd());
      std::shuffle(tmp.begin(), tmp.end(), g);
      for (const auto& id : tmp)
      {
        ob.push(id);
      }
    }
    else
    {
      for (const auto& place : net.places())
      {
        ob.push(place.id);
      }
    }
    return sdd::order<sdd_conf>(ob);
  }
}

/*------------------------------------------------------------------------------------------------*/

SDD
initial_state(const sdd::order<sdd_conf>& order, const pn::net& net)
{
  return SDD(order, [&](const std::string& id)
                        -> sdd::values::flat_set<unsigned int>
                       {
                         return {net.places_by_id().find(id)->marking};
                       });
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Create a timed function if required by the configuration, a normal function otherwise.
template <typename Fun, typename... Args>
homomorphism
mk_fun( const conf::pnmc_configuration& conf, const sdd::order<sdd_conf>& o
      , const sdd_conf::Identifier& id, Args&&... args)
{
  if (conf.max_time > chrono::duration<double>(0))
  {
    return function(o, id, timed<Fun>(conf, std::forward<Args>(args)...));
  }
  else
  {
    return function(o, id, Fun(std::forward<Args>(args)...));;
  }
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Compute the transition relation corresponding to a petri net.
homomorphism
transition_relation( const conf::pnmc_configuration& conf, const sdd::order<sdd_conf>& o
                   , const pn::net& net, boost::dynamic_bitset<>& transitions_bitset
                   , statistics& stats)
{
  chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();

  std::set<homomorphism> operands;
  operands.insert(sdd::id<sdd_conf>());

  for (const auto& transition : net.transitions())
  {
    homomorphism h_t = sdd::id<sdd_conf>();

    // Add a "canary" to detect live transitions.
    if (conf.compute_dead_transitions)
    {
      if (not transition.post.empty())
      {
        const auto f = mk_fun<live>( conf, o, transition.post.begin()->first, transition.index
                                   , transitions_bitset);
        h_t = sdd::carrier(o, transition.post.begin()->first, f);
      }
    }

    // Post actions.
    for (const auto& arc : transition.post)
    {
      homomorphism f = conf.marking_bound == 0
                     ? mk_fun<post>(conf, o, arc.first, arc.second)
                     : mk_fun<bounded_post>( conf, o, arc.first, arc.second, conf.marking_bound
                                           , arc.first);
      h_t = composition(h_t, sdd::carrier(o, arc.first, f));
    }

    // Pre actions.
    for (const auto& arc : transition.pre)
    {
      homomorphism f = mk_fun<pre>(conf, o, arc.first, arc.second);
      h_t = composition(h_t, sdd::carrier(o, arc.first, f));
    }

    operands.insert(h_t);
  }
  stats.relation_duration = chrono::system_clock::now() - start;
  return fixpoint(sum(o, operands.cbegin(), operands.cend()));
}

/*------------------------------------------------------------------------------------------------*/

homomorphism
rewrite( const conf::pnmc_configuration& conf, const sdd::order<sdd_conf>& o
       , const homomorphism& h, statistics& stats)
{
  chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
  const auto res = sdd::rewrite(o, h);
  stats.rewrite_duration = chrono::system_clock::now() - start;
  return res;
}

/*------------------------------------------------------------------------------------------------*/

SDD
state_space( conf::pnmc_configuration& conf, const sdd::order<sdd_conf>& o, SDD m
           , homomorphism h, statistics& stats)
{
  SDD res;
  conf.beginning = chrono::system_clock::now();
  chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
  try
  {
    res = h(o, m);
  }
  catch (const sdd::interrupt<SDD>& i)
  {
    std::cout << "Interrupted state space computation after " << conf.max_time.count() << "s"
               << std::endl;
    stats.interrupted = true;
    res = i.result();
  }
  stats.state_space_duration = chrono::system_clock::now() - start;
  return res;
}

/*------------------------------------------------------------------------------------------------*/

SDD
dead_states( const conf::pnmc_configuration& conf, const sdd::order<sdd_conf>& o, const pn::net& net
           , const SDD& state_space, statistics& stats)
{
  std::set<homomorphism> and_operands;
  std::set<homomorphism> or_operands;

  chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();

  // Get relation
  for (const auto& transition : net.transitions())
  {
    // We are only interested in pre actions.
    for (const auto& arc : transition.pre)
    {
      const auto h = function(o, arc.first, dead(arc.second));
      or_operands.insert(sdd::carrier(o, arc.first, h));
    }

    and_operands.insert(sum(o, or_operands.cbegin(), or_operands.cend()));
    or_operands.clear();
  }
  const auto tmp = intersection(o, and_operands.cbegin(), and_operands.cend());
  stats.dead_states_relation_duration = chrono::system_clock::now() - start;

  // Rewrite the relation
  start = chrono::system_clock::now();
  const auto h = sdd::rewrite(o, tmp);
  stats.dead_states_rewrite_duration = chrono::system_clock::now() - start;

  // Compute the dead states
  start = chrono::system_clock::now();
  const auto res = h(o, state_space);
  stats.dead_states_duration = chrono::system_clock::now() - start;

  return res;
}

/*------------------------------------------------------------------------------------------------*/

void
work(conf::pnmc_configuration& conf, const pn::net& net)
{
  auto manager = sdd::manager<sdd_conf>::init();

  statistics stats(conf);

  boost::dynamic_bitset<> transitions_bitset(net.transitions().size());

  sdd::order<sdd_conf> o = mk_order(conf, net);
  assert(not o.empty() && "Empty order");

  if (conf.order_show)
  {
    std::cout << o << std::endl;
  }

  homomorphism h = transition_relation(conf, o, net, transitions_bitset, stats);
  if (conf.show_relation)
  {
    std::cout << h << std::endl;
  }

  if (conf.order_ordering_force)
  {
    chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
    o = sdd::force_ordering(o, h);
    stats.force_duration = chrono::system_clock::now() - start;

    h = transition_relation(conf, o, net, transitions_bitset, stats);

    if (conf.order_show)
    {
      std::cout << o << std::endl;
    }
  }

  h = rewrite(conf, o, h, stats);

  const SDD m0 = initial_state(o, net);

  try
  {
    SDD m = sdd::zero<sdd_conf>();
    m = state_space(conf, o, m0, h, stats);
    stats.nb_states = m.size().template convert_to<long double>();
    std::cout << stats.nb_states << " states" << std::endl;

    if (conf.export_final_sdd_dot)
    {
      std::ofstream dot_file(conf.export_final_sdd_dot_file);
      if (dot_file.is_open())
      {
        dot_file << sdd::tools::dot(m) << std::endl;
      }
      else
      {
        std::cerr << "Can't export state space's SDD to " << conf.export_final_sdd_dot_file
                  << std::endl;
      }
    }

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
        std::cout << *std::prev(dead_transitions.cend()) << std::endl;
      }
      else
      {
        std::cout << "No dead transitions" << std::endl;
      }
    }

    if (conf.compute_dead_states)
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
      if (conf.order_ordering_force)
      {
        std::cout << "FORCE                : " << stats.force_duration.count() << "s" << std::endl;
      }
      if (conf.compute_dead_states)
      {
        std::cout << "Dead states relation : " << stats.dead_states_relation_duration.count()
                  << "s" << std::endl
                  << "Dead states rewrite  : " << stats.dead_states_rewrite_duration.count()
                  << "s" << std::endl
                  << "Dead states          ; " << stats.dead_states_duration.count()
                  << "s" << std::endl;
      }
    }

    if (conf.show_final_sdd_bytes)
    {
      std::cout << "Final SDD size: " << sdd::tools::size(m) << " bytes" << std::endl;
    }

    if (conf.export_to_lua)
    {
      std::ofstream lua_file(conf.export_to_lua_file);
      if (lua_file.is_open())
      {
        lua_file << sdd::tools::lua(m) << std::endl;
      }
    }

    if (conf.json)
    {
      std::ofstream file(conf.json_file);
      if (file.is_open())
      {
        const sdd::tools::sdd_statistics<sdd_conf> final_sdd_stats(m);

        cereal::JSONOutputArchive archive(file);
        if (not conf.read_stdin)
        {
          archive(cereal::make_nvp("file", conf.file_name));
        }
        archive(cereal::make_nvp("pnmc", stats));
        archive(cereal::make_nvp("libsdd", manager));
        archive(cereal::make_nvp("final sdd", final_sdd_stats));
      }
    }
  }
  catch (const bound_error& be)
  {
    std::cout << "Marking limit reached for place " << be.place << std::endl;
  }
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc
