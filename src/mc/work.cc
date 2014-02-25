#include <algorithm> // random_shuffle, transform
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <utility>  // pair

#include <sdd/sdd.hh>
#include <sdd/tools/dot.hh>
#include <sdd/tools/lua.hh>
#include "sdd/tools/size.hh"

#include "mc/bound_error.hh"
#include "mc/bounded_post.hh"
#include "mc/dead.hh"
#include "mc/live.hh"
#include "mc/post.hh"
#include "mc/pre.hh"
#include "mc/work.hh"

namespace pnmc { namespace mc {

namespace chrono = std::chrono;

typedef sdd::conf1 sdd_conf;
typedef sdd::SDD<sdd_conf> SDD;
typedef sdd::homomorphism<sdd_conf> homomorphism;

using sdd::Composition;
using sdd::Fixpoint;
using sdd::Intersection;
using sdd::Sum;
using sdd::ValuesFunction;

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

homomorphism
transition_relation( const conf::pnmc_configuration& conf, const sdd::order<sdd_conf>& o
                   , const pn::net& net, boost::dynamic_bitset<>& transitions_bitset)
{
  chrono::time_point<chrono::system_clock> start;
  chrono::time_point<chrono::system_clock> end;
  std::size_t elapsed;

  start = chrono::system_clock::now();
  std::set<homomorphism> operands;
  operands.insert(sdd::Id<sdd_conf>());

  for (const auto& transition : net.transitions())
  {
    homomorphism h_t = sdd::Id<sdd_conf>();

    // Add a "canary" to detect live transitions.
    if (conf.compute_dead_transitions)
    {
      if (not transition.post.empty())
      {
        const auto f = ValuesFunction<sdd_conf>( o, transition.post.begin()->first
                                               , live(transition.index, transitions_bitset));
        h_t = sdd::carrier(o, transition.post.begin()->first, f);
      }
    }

    // Post actions.
    for (const auto& arc : transition.post)
    {
      homomorphism f = conf.marking_bound == 0
                     ? ValuesFunction<sdd_conf>(o, arc.first, post(arc.second))
                     : ValuesFunction<sdd_conf>(o, arc.first, bounded_post( arc.second
                                                                          , conf.marking_bound
                                                                          , arc.first));
      h_t = Composition(h_t, sdd::carrier(o, arc.first, f));
    }

    // Pre actions.
    for (const auto& arc : transition.pre)
    {
      homomorphism f = ValuesFunction<sdd_conf>(o, arc.first, pre(arc.second));
      h_t = Composition(h_t, sdd::carrier(o, arc.first, f));
    }

    operands.insert(h_t);
  }
  end = chrono::system_clock::now();
  elapsed = chrono::duration_cast<chrono::seconds>(end-start).count();

  if (conf.show_time)
  {
    std::cout << "Transition relation time: " << elapsed << "s" << std::endl;
  }

  start = chrono::system_clock::now();
  const auto res = sdd::rewrite(o, Fixpoint(Sum<sdd_conf>(o, operands.cbegin(), operands.cend())));
  end = chrono::system_clock::now();
  elapsed = chrono::duration_cast<chrono::seconds>(end-start).count();

  if (conf.show_time)
  {
    std::cout << "Rewrite time: " << elapsed << "s" << std::endl;
  }

  return res;
}

/*------------------------------------------------------------------------------------------------*/

SDD
state_space( const conf::pnmc_configuration& conf, const sdd::order<sdd_conf>& o, SDD m
           , homomorphism h)
{
  chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
  const auto res = h(o, m);
  chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();
  const std::size_t elapsed = chrono::duration_cast<chrono::seconds>(end-start).count();
  if (conf.show_time)
  {
    std::cout << "State space computation time: " << elapsed << "s" << std::endl;
  }
  return res;
}

/*------------------------------------------------------------------------------------------------*/

SDD
dead_states( const conf::pnmc_configuration& conf, const sdd::order<sdd_conf>& o, const pn::net& net
           , const SDD& state_space)
{
  chrono::time_point<chrono::system_clock> start;
  chrono::time_point<chrono::system_clock> end;
  std::size_t elapsed;

  start = chrono::system_clock::now();

  std::set<homomorphism> and_operands;
  std::set<homomorphism> or_operands;

  for (const auto& transition : net.transitions())
  {
    // We are only interested in pre actions.
    for (const auto& arc : transition.pre)
    {
      const auto h = ValuesFunction<sdd_conf>(o, arc.first, dead(arc.second));
      or_operands.insert(sdd::carrier(o, arc.first, h));
    }

    and_operands.insert(Sum(o, or_operands.cbegin(), or_operands.cend()));
    or_operands.clear();
  }
  const auto tmp = Intersection(o, and_operands.cbegin(), and_operands.cend());
  end = chrono::system_clock::now();
  elapsed = chrono::duration_cast<chrono::seconds>(end-start).count();
  if (conf.show_time)
  {
    std::cout << "Dead states relation time: " << elapsed << "s" << std::endl;
  }

  /* --------------------  */
  start = chrono::system_clock::now();
  const auto h = sdd::rewrite(o, tmp);
  end = chrono::system_clock::now();
  elapsed = chrono::duration_cast<chrono::seconds>(end-start).count();
  if (conf.show_time)
  {
    std::cout << "Dead states relation rewrite time: " << elapsed << "s" << std::endl;
  }

  /* --------------------  */

  start = chrono::system_clock::now();
  const auto res = h(o, state_space);
  end = chrono::system_clock::now();
  elapsed = chrono::duration_cast<chrono::seconds>(end-start).count();
  if (conf.show_time)
  {
    std::cout << "Dead states computation time: " << elapsed << "s" << std::endl;
  }

  return res;
}

/*------------------------------------------------------------------------------------------------*/

void
work(const conf::pnmc_configuration& conf, const pn::net& net)
{
  auto manager = sdd::manager<sdd_conf>::init();

  boost::dynamic_bitset<> transitions_bitset(net.transitions().size());

  const sdd::order<sdd_conf>& o = mk_order(conf, net);
  assert(not o.empty() && "Empty order");

  if (conf.order_show)
  {
    std::cout << o << std::endl;
  }
  const SDD m0 = initial_state(o, net);

  const homomorphism h = transition_relation(conf, o, net, transitions_bitset);
  if (conf.show_relation)
  {
    std::cout << h << std::endl;
  }

  try
  {
    SDD m = sdd::zero<sdd_conf>();
    m = state_space(conf, o, m0, h);
    std::cout << m.size().template convert_to<long double>() << " states" << std::endl;

    if (conf.sdd_export_state_space)
    {
      std::ofstream dot_file(conf.sdd_export_state_space_file);
      if (dot_file.is_open())
      {
        dot_file << sdd::tools::dot(m) << std::endl;
      }
      else
      {
        std::cerr << "Can't export state space's SDD to " << conf.sdd_export_state_space_file
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
      const auto dead = dead_states(conf, o, net, m);
      if (dead.empty())
      {
        std::cout << "No dead states" << std::endl;
      }
      else
      {
        std::cout << dead.size().template convert_to<long double>() << " dead states" << std::endl;

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

    if (conf.export_to_lua)
    {
      std::ofstream lua_file(conf.export_to_lua_file);
      if (lua_file.is_open())
      {
        lua_file << sdd::tools::lua(m) << std::endl;
      }
    }

    if (conf.show_state_space_bytes)
    {
      std::cout << "Final SDD size: " << sdd::tools::size(m) << " bytes" << std::endl;
    }

    if (conf.show_hash_tables_stats)
    {
      std::cout << manager << std::endl;
    }
  }
  catch (const bound_error& be)
  {
    std::cout << "Marking limit reached for place " << be.place << std::endl;
  }
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc
