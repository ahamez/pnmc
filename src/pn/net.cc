#include "pn/net.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

net::net()
  : name(), places_set(), transitions_set(), modules(nullptr)
{}

/*------------------------------------------------------------------------------------------------*/

const place&
net::add_place(const std::string& pid, unsigned int marking)
{
  const auto cit = places_by_id().find(pid);
  if (cit == places_by_id().cend())
  {
    return *places_set.get<insertion_index>().emplace_back(pid, marking).first;
  }
  else
  {
    // This place was created before by add_post_place() or add_pre_place().
    // At this time, the marking was not known. We can now update it.
    places_set.get<id_index>().modify(cit, [&](place& p){p.marking = marking;});
    return *cit;
  }
}

/*------------------------------------------------------------------------------------------------*/

const transition&
net::add_transition(const std::string& tid)
{
  static std::size_t transition_index = 0;
  const auto cit = transitions_set.get<id_index>().find(tid);
  if (cit == transitions_set.get<id_index>().cend())
  {
    return *transitions_set.get<id_index>().insert({tid, transition_index++}).first;
  }
  else
  {
    return *cit;
  }
}

/*------------------------------------------------------------------------------------------------*/

void
net::add_post_place(const std::string& tid, const std::string& post, const arc& a)
{
  const auto it = transitions_set.get<id_index>().find(tid);
  transitions_set.modify( it
                        , [&](transition& t){t.post.insert({post , a});});
  if (places_by_id().find(post) == places_by_id().end())
  {
    add_place(post, 0);
  }
  const auto place_cit = places_by_id().find(post);
  places_set.get<id_index>().modify( place_cit
                                   , [&](place& p){p.pre.insert({tid, a});});
}

/*------------------------------------------------------------------------------------------------*/

void
net::add_pre_place(const std::string& tid, const std::string& pre, const arc& a)
{
  const auto it = transitions_set.get<id_index>().find(tid);
  transitions_set.modify( it
                        , [&](transition& t){t.pre.insert({pre, a});});
  if (places_by_id().find(pre) == places_by_id().end())
  {
    add_place(pre, 0);
  }
  /// @todo Only one lookup.
  const auto place_cit = places_by_id().find(pre);
  places_set.get<id_index>().modify( place_cit
                                   , [&](place& p){p.post.insert({tid , a});});
}

/*------------------------------------------------------------------------------------------------*/

const net::places_type::index<net::insertion_index>::type&
net::places()
const noexcept
{
  return places_set.get<insertion_index>();
}

/*------------------------------------------------------------------------------------------------*/

const net::places_type::index<net::id_index>::type&
net::places_by_id()
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

void
net::add_time_interval(const std::string& tid, unsigned int low, unsigned int high)
{
  const auto it = transitions_set.get<id_index>().find(tid);
  transitions_set.modify(it, [&](transition& t){t.low = low; t.high = high;});
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
