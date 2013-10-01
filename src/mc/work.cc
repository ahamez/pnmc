#include <chrono>
#include <iostream>
#include <set>
#include <utility> // pair

#include <sdd/sdd.hh>

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
transition_relation(const sdd::order<sdd_conf>& o, const pn::net& net)
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

    // post actions.
    for (const auto& arc : transition.post)
    {
      homomorphism f = ValuesFunction<sdd_conf>(o, arc.first, post(arc.second.weight));
      h_t = Composition(h_t, sdd::carrier(o, arc.first, f));
    }

    // pre actions.
    for (const auto& arc : transition.pre)
    {
      homomorphism f = ValuesFunction<sdd_conf>(o, arc.first, pre(arc.second.weight));
      h_t = Composition(h_t, sdd::carrier(o, arc.first, f));
    }

    operands.insert(h_t);
  }
  end = chrono::system_clock::now();
  elapsed = chrono::duration_cast<chrono::seconds>(end-start).count();
  std::cout << "Transition relation time: " << elapsed << "s" << std::endl;

  start = chrono::system_clock::now();
  const auto res = sdd::rewrite(Fixpoint(Sum<sdd_conf>(o, operands.cbegin(), operands.cend())), o);
  end = chrono::system_clock::now();
  elapsed = chrono::duration_cast<chrono::seconds>(end-start).count();
  std::cout << "Rewrite time: " << elapsed << "s" << std::endl;

  return res;
}

/*------------------------------------------------------------------------------------------------*/

SDD
state_space(const sdd::order<sdd_conf>& o, SDD m, homomorphism h)
{
  return h(o, m);
}

/*------------------------------------------------------------------------------------------------*/

void
work(const conf::pnmc_configuration&, const pn::net& net)
{
  auto manager = sdd::manager<sdd_conf>::init();

  const sdd::order<sdd_conf>& o = mk_order(net);
//  std::cout << o << std::endl;
  const SDD m0 = initial_state(o, net);
//  std::cout << m0 << std::endl << std::endl;

  const homomorphism h = transition_relation(o, net);
  std::cout << h << std::endl;

  chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
  const SDD m = state_space(o, m0, h);
  chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();
  const std::size_t elapsed = chrono::duration_cast<chrono::seconds>(end-start).count();
  std::cout << "State space computation time: " << elapsed << "s" << std::endl;

  const auto n = sdd::count_combinations(m);
  long double n_prime = n.template convert_to<long double>();
  std::cout << n_prime << std::endl;

  std::cout << manager << std::endl;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc
