#include "pn/net.hh"
#include "pn/place.hh"
#include "pn/transition.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

/// @brief Used by Boost.MultiIndex.
struct add_post_place_to_transition
{
  const unsigned int new_valuation;
  const std::string new_place_id;

  add_post_place_to_transition(unsigned int valuation, const std::string& id)
	  : new_valuation(valuation), new_place_id(id)
  {}

  void
  operator()(transition& t)
  const
  {
    t.post.insert(std::make_pair(new_place_id , new_valuation));
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Used by Boost.MultiIndex.
struct add_pre_place_to_transition
{
  const unsigned int new_valuation;
  const std::string new_place_id;

  add_pre_place_to_transition(unsigned int valuation, const std::string& id)
  	: new_valuation(valuation), new_place_id(id)
  {}

  void
  operator()(transition& t)
  const
  {
    t.pre.insert(std::make_pair(new_place_id, new_valuation));
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Used by Boost.MultiIndex.
struct update_place
{
  const unsigned int marking;

  update_place(unsigned int m)
  	: marking(m)
  {}

  void
  operator()(place& p)
  const
  {
    p.marking = marking;
  }
};

/*------------------------------------------------------------------------------------------------*/

net::net()
  : name(), places_set(), transitions_set(), modules(nullptr)
{}

/*------------------------------------------------------------------------------------------------*/

const place&
net::add_place(const std::string& pid, unsigned int marking)
{
  const auto cit = places_set.get<id_index>().find(pid);
  if (cit == places_set.get<id_index>().cend())
  {
    return *places_set.insert({pid, marking}).first;
  }
  else
  {
    // This place was created before by add_post_place() or add_pre_place().
    // At this time, the marking was not known. We can now update it.
    places_set.modify(cit, update_place(marking));
    return *cit;
  }
}

/*------------------------------------------------------------------------------------------------*/

const transition&
net::add_transition(const std::string& tid)
{
  static std::size_t transition_index = 0;
  const auto insertion = transitions_set.insert({tid, transition_index++});
  if (not insertion.second)
  {
    throw "Transition " + tid + " already exists";
  }
  return *insertion.first;
}

/*------------------------------------------------------------------------------------------------*/

void
net::add_post_place(const std::string& tid, const std::string& post, unsigned int valuation)
{
  const auto it = transitions_set.get<id_index>().find(tid);
  transitions_set.modify(it, add_post_place_to_transition(valuation, post));
  if (places().find(post) == places().end())
  {
    add_place(post, 0);
  }
}

/*------------------------------------------------------------------------------------------------*/

void
net::add_pre_place(const std::string& tid, const std::string& pre, unsigned int valuation)
{
  const auto it = transitions_set.get<id_index>().find(tid);
  transitions_set.modify(it, add_pre_place_to_transition(valuation, pre));
  if (places().find(pre) == places().end())
  {
    add_place(pre, 0);
  }
}

/*------------------------------------------------------------------------------------------------*/

const net::places_type::index<net::id_index>::type&
net::places()
const noexcept
{
  return places_set.get<id_index>();
}

/*------------------------------------------------------------------------------------------------*/

const net::transitions_type::index<net::id_index>::type&
net::transitions()
const noexcept
{
  return transitions_set.get<id_index>();
}

/*------------------------------------------------------------------------------------------------*/

const transition&
net::get_transition_by_index(std::size_t index)
const
{
  return *transitions_set.get<index_index>().find(index);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
