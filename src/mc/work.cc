#include <chrono>
#include <iostream>
#include <memory>  // unique_ptr
#include <set>
#include <utility> // pair

#include <sdd/sdd.hh>

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
using sdd::Sum;
using sdd::ValuesFunction;

/*------------------------------------------------------------------------------------------------*/

struct mk_order_visitor
  : public boost::static_visitor<std::pair<std::string, sdd::order_builder<sdd_conf>>>
{
  // Place: base case of the recursion, there's no more possible nested hierarchies.
  std::pair<std::string, sdd::order_builder<sdd_conf>>
  operator()(const pn::place* p)
  const noexcept
  {
    return make_pair(p->id, sdd::order_builder<sdd_conf>());
  }

  // Hierarchy.
  std::pair<std::string, sdd::order_builder<sdd_conf>>
  operator()(const pn::module_node& m)
  const noexcept
  {
    sdd::order_builder<sdd_conf> ob;
    for (const auto& h : m.nested)
    {
      const auto res = boost::apply_visitor(mk_order_visitor(), *h);
      ob.push(res.first, res.second);
    }
    return make_pair(m.id , ob);
  }
};

/*------------------------------------------------------------------------------------------------*/

sdd::order<sdd_conf>
mk_order(const pn::net& net)
{
  if (net.modules)
  {
    return sdd::order<sdd_conf>(boost::apply_visitor(mk_order_visitor(), *net.modules).second);
  }
  else
  {
    sdd::order_builder<sdd_conf> ob;
    for (const auto& place : net.places())
    {
      ob.push(place.id);
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
                         return {net.places().find(id)->marking};
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
    if (conf.compute_dead_transitions)
    {
      const auto f = ValuesFunction<sdd_conf>( o, transition.post.begin()->first
                                             , live(transition.index, transitions_bitset));
      h_t = sdd::carrier(o, transition.post.begin()->first, f);
    }

    // post actions.
    for (const auto& arc : transition.post)
    {
      homomorphism f = ValuesFunction<sdd_conf>(o, arc.first, post(arc.second));
      h_t = Composition(h_t, sdd::carrier(o, arc.first, f));
    }

    // pre actions.
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
  const auto res = sdd::rewrite(Fixpoint(Sum<sdd_conf>(o, operands.cbegin(), operands.cend())), o);
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

void
work(const conf::pnmc_configuration& conf, const pn::net& net)
{
  auto manager = sdd::manager<sdd_conf>::init();

  boost::dynamic_bitset<> transitions_bitset(net.transitions().size());

  const sdd::order<sdd_conf>& o = mk_order(net);
  if (conf.show_order)
  {
    std::cout << o << std::endl;
  }
  const SDD m0 = initial_state(o, net);

  const homomorphism h = transition_relation(conf, o, net, transitions_bitset);
  if (conf.show_relation)
  {
    std::cout << h << std::endl;
  }

  const SDD m = state_space(conf, o, m0, h);

  const auto n = sdd::count_combinations(m);
  long double n_prime = n.template convert_to<long double>();
  std::cout << n_prime << " states" << std::endl;

  if (conf.compute_dead_transitions)
  {
    std::deque<std::string> dead_transitions;
    for (auto i = 0; i < net.transitions().size(); ++i)
    {
      if (not transitions_bitset[i])
      {
        dead_transitions.push_back(net.get_transition_by_index(i).id);
      }
    }

    std::cout << dead_transitions.size() << " dead transitions." << std::endl;
    if (not dead_transitions.empty())
    {
      std::copy( dead_transitions.cbegin(), std::prev(dead_transitions.cend())
               , std::ostream_iterator<std::string>(std::cout, ","));
      std::cout << *std::prev(dead_transitions.cend());
    }
  }

  if (conf.show_hash_tables_stats)
  {
    std::cout << manager << std::endl;
  }
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc
