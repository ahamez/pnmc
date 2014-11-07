#include <algorithm> // any_of

#include "shared/pn/net.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

net::net()
  : name(), places_set(), transitions_set(), modules(nullptr)
{}

/*------------------------------------------------------------------------------------------------*/

const place&
net::add_place(const std::string& pid, valuation_type marking)
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
net::add_post_place( const std::string& tid, const std::string& post
                   , valuation_type weight, arc::type ty)
{
  if (ty != arc::type::normal)
  {
    throw std::runtime_error("An output arc of a transition must be normal.");
  }

  const auto it = transitions_set.get<id_index>().find(tid);
  if (it == end(transitions_set.get<id_index>()))
  {
    throw std::runtime_error("Adding a post place to a non-existing transition.");
  }

  if (it->post.find(post) != end(it->post))
  {
    // Add weights of arcs with the same direction between the same place and transition.
    transitions_set.modify(it, [&](transition& t)
                                  {
                                    auto search = t.post.find(post);
                                    search->second.weight += weight;
                                  });
  }
  else
  {
    transitions_set.modify( it
                          , [&](transition& t)
                               {
                                 t.post.emplace_hint(end(t.post), post , pn::arc{weight, ty});
                               });

  }

  if (places_by_id().find(post) == end(places_by_id()))
  {
    add_place(post, 0);
  }
  places_set.get<id_index>().modify( places_by_id().find(post)
                                   , [&](place& p)
                                        {
                                          p.pre.emplace_hint(end(p.pre), tid, pn::arc{weight, ty});
                                        });
}

/*------------------------------------------------------------------------------------------------*/

void
net::add_pre_place( const std::string& tid, const std::string& pre
                  , valuation_type weight, arc::type ty)
{
  const auto it = transitions_set.get<id_index>().find(tid);
  if (it == end(transitions_set.get<id_index>()))
  {
    throw std::runtime_error("Adding a pre place to a non-existing transition.");
  }

  const auto arc_search = it->pre.find(pre);
  if (arc_search != end(it->pre))
  {
    if (arc_search->second.kind != ty)
    {
      throw std::runtime_error
        ("Arcs of different types between a place and a transition are not supported yet.");
    }

    // Add weights of arcs with the same direction between the same place and transition.
    transitions_set.modify(it, [&](transition& t)
                                  {
                                    auto search = t.pre.find(pre);
                                    search->second.weight += weight;
                                  });
  }
  else
  {
    transitions_set.modify( it
                          , [&](transition& t)
                               {
                                 t.pre.emplace_hint(end(t.pre), pre , pn::arc{weight, ty});
                               });

  }

  if (places_by_id().find(pre) == end(places_by_id()))
  {
    add_place(pre, 0);
  }
  places_set.get<id_index>().modify( places_by_id().find(pre)
                                   , [&](place& p)
                                        {
                                          p.post.emplace_hint(end(p.post), tid, pn::arc{weight, ty});
                                        });
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
net::add_time_interval(const std::string& tid, clock_type low, clock_type high)
{
  const auto it = transitions_set.get<id_index>().find(tid);
  transitions_set.modify(it, [&](transition& t){t.low = low; t.high = high;});
}

/*------------------------------------------------------------------------------------------------*/

bool
net::timed()
const noexcept
{
  return std::any_of( transitions().cbegin(), transitions().cend()
                    , [](const transition& t){return t.timed();});
}

/*------------------------------------------------------------------------------------------------*/

bool
net::enabled(const std::string& tid)
const
{
  const auto search = transitions().find(tid);
  if (search == transitions().cend())
  {
    throw std::invalid_argument("Transition " + tid + " doesn't exist");
  }
  const auto& t = *search;

  for (const auto& arc : t.pre)
  {
    const auto place_cit = places_by_id().find(arc.first);
    if (place_cit == places_by_id().cend())
    {
      throw std::runtime_error("Place " + arc.first + " doesn't exist");
    }
    const auto& place = *place_cit;
    switch (arc.second.kind)
    {
      case pn::arc::type::normal:
      case pn::arc::type::read:
      {
        if (place.marking < arc.second.weight)
        {
          return false;
        }
        break;
      }

      case pn::arc::type::inhibitor:
      {
        if (place.marking >= arc.second.weight)
        {
          return false;
        }
        break;
      }

      default:
        throw std::runtime_error("Unsupported arc type.");
    }

  }
  return true;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
