#include <cassert>
#include <sstream>
#include <stdexcept>

#include "pn/net.hh"
#include "pn/arc.hh"
#include "pn/place.hh"
#include "pn/transition.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

/// @brief Used by Boost.MultiIndex.
struct add_post_place_to_transition
{
  const arc& new_arc;
  std::string new_place;

  add_post_place_to_transition(const arc& a, const std::string& id)
	  : new_arc(a), new_place(id)
  {}

  void
  operator()(transition& t)
  const
  {
    t.post.insert(std::make_pair(new_place , new_arc));
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Used by Boost.MultiIndex.
struct add_pre_place_to_transition
{
  const arc& new_arc;
  std::string new_place;

  add_pre_place_to_transition(const arc& a, const std::string& id)
  	: new_arc(a), new_place(id)
  {}

  void
  operator()(transition& t)
  const
  {
    t.pre.insert(std::make_pair(new_place , new_arc));
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Used by Boost.MultiIndex.
struct update_place
{
  const std::string label;
  const unsigned int marking;

  update_place(const std::string& l, unsigned int m)
  	: label(l), marking(m)
  {}

  void
  operator()(place& p)
  const
  {
    p.label = label;
    p.marking = marking;
  }
};

/*------------------------------------------------------------------------------------------------*/

net::net()
  : name(), places_set(), transitions_set(), modules(nullptr)
{}

/*------------------------------------------------------------------------------------------------*/

const place&
net::add_place(const std::string& pid, const std::string& label, unsigned int marking)
{
  const auto cit = places_set.get<id_index>().find(pid);
  if (cit == places_set.get<id_index>().cend())
  {
    return *places_set.insert({pid, label, marking}).first;
  }
  else
  {
    // This place was created before by add_post_place() or add_pre_place().
    // At this time, the marking was not known. Thus, we now update it.
    // The assert() is here because modify() would return false if the place could not have been
    // modified (when bmi sees a conflict on unique keys). Which is impossible here, because
    // the marking is not a unique key.
    assert(places_set.modify(cit, update_place(label, marking)));
    return *cit;
  }
}

/*------------------------------------------------------------------------------------------------*/

const transition&
net::add_transition(const std::string& tid, const std::string& label)
{
  static std::size_t transition_index = 0;
  const auto insertion = transitions_set.insert({tid, label, transition_index++});
  if (not insertion.second)
  {
    std::stringstream ss;
    ss << "Transition " << tid << " already exists" << std::endl;
    throw std::runtime_error(ss.str());
  }
  return *insertion.first;
}

/*------------------------------------------------------------------------------------------------*/

void
net::add_post_place(const std::string& tid, const std::string& post, const arc& a)
{
  const auto it = transitions_set.get<id_index>().find(tid);
  transitions_set.modify(it, add_post_place_to_transition(a,post));
  if (places().find(post) == places().end())
  {
    add_place(post, post, 0);
  }
}

/*------------------------------------------------------------------------------------------------*/

void
net::add_pre_place(const std::string& tid, const std::string& pre, const arc& a)
{
  const auto it = transitions_set.get<id_index>().find(tid);
  transitions_set.modify(it, add_pre_place_to_transition(a,pre));
  if (places().find(pre) == places().end())
  {
    add_place(pre, pre, 0);
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
