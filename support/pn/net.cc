/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <algorithm> // any_of

#include "support/pn/net.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

net::net()
  : name{}
  , modules{}
  , root_modules{}
  , format{}
  , m_places{}
  , m_transitions{}
  , m_current_place_uid{0}
  , m_current_transition_uid{0}
{}

/*------------------------------------------------------------------------------------------------*/

const place&
net::add_place(const std::string& pname, valuation_type marking)
{
  const auto cit = places_by<name_index>().find(pname);
  if (cit == places_by<name_index>().end())
  {
    return *places_by<insertion_index>().emplace_back(m_current_place_uid++, pname, marking).first;
  }
  else
  {
    // This place was created before by add_post_place() or add_pre_place().
    // At this time, the marking was not known. We can now update it.
    places_by<name_index>().modify(cit, [&](place& p){p.marking = marking;});
    return *cit;
  }
}

/*------------------------------------------------------------------------------------------------*/

const transition&
net::add_transition(const std::string& tname)
{
  const auto cit = transitions_by<name_index>().find(tname);
  if (cit == transitions_by<name_index>().cend())
  {
    return *transitions_by<name_index>().insert({m_current_transition_uid++, tname}).first;
  }
  else
  {
    return *cit;
  }
}

/*------------------------------------------------------------------------------------------------*/

void
net::add_post_place( const std::string& tname, const std::string& post
                   , valuation_type weight, arc::type ty)
{
  if (ty != arc::type::normal)
  {
    throw std::runtime_error("An output arc of a transition must be normal.");
  }

  const auto it = transitions_by<name_index>().find(tname);
  if (it == transitions_by<name_index>().cend())
  {
    throw std::runtime_error("Adding a post place to a non-existing transition.");
  }

  if (it->post.find(post) != end(it->post))
  {
    // Add weights of arcs with the same direction between the same place and transition.
    m_transitions.modify(it, [&](transition& t)
                             {
                               auto search = t.post.find(post);
                               search->second.weight += weight;
                             });
  }
  else
  {
    m_transitions.modify( it
                        , [&](transition& t)
                          {
                            t.post.emplace_hint(end(t.post), post , pn::arc{weight, ty});
                          });

  }

  if (places_by<name_index>().find(post) == places_by<name_index>().end())
  {
    add_place(post, 0);
  }
  places_by<name_index>().modify( places_by<name_index>().find(post)
                                , [&](place& p)
                                  {
                                    p.pre.emplace_hint(end(p.pre), tname, pn::arc{weight, ty});
                                  });
}

/*------------------------------------------------------------------------------------------------*/

void
net::add_pre_place( const std::string& tname, const std::string& pre
                  , valuation_type weight, arc::type ty)
{
  const auto it = transitions_by<name_index>().find(tname);
  if (it == transitions_by<name_index>().end())
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
    m_transitions.modify(it, [&](transition& t)
                             {
                               auto search = t.pre.find(pre);
                               search->second.weight += weight;
                             });
  }
  else
  {
    m_transitions.modify( it
                        , [&](transition& t)
                          {
                            t.pre.emplace_hint(end(t.pre), pre , pn::arc{weight, ty});
                          });

  }

  if (places_by<name_index>().find(pre) == places_by<name_index>().end())
  {
    add_place(pre, 0);
  }
  places_by<name_index>().modify( places_by<name_index>().find(pre)
                                , [&](place& p)
                                  {
                                    p.post.emplace_hint(end(p.post), tname, pn::arc{weight, ty});
                                  });
}

/*------------------------------------------------------------------------------------------------*/

const net::places_type::index<net::insertion_index>::type&
net::places_by_insertion()
const noexcept
{
  return m_places.get<insertion_index>();
}

/*------------------------------------------------------------------------------------------------*/

const net::places_type::index<net::name_index>::type&
net::places()
const noexcept
{
  return m_places.get<name_index>();
}

/*------------------------------------------------------------------------------------------------*/

const net::transitions_type::index<net::name_index>::type&
net::transitions()
const noexcept
{
  return transitions_by<name_index>();
}

/*------------------------------------------------------------------------------------------------*/

const transition&
net::get_transition_by_uid(std::size_t uid)
const
{
  return *transitions_by<uid_index>().find(uid);
}

/*------------------------------------------------------------------------------------------------*/

void
net::add_time_interval(const std::string& tname, clock_type low, clock_type high)
{
  const auto it = transitions_by<name_index>().find(tname);
  m_transitions.modify(it, [&](transition& t){t.low = low; t.high = high;});
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
net::enabled(const std::string& tname)
const
{
  const auto search = transitions().find(tname);
  if (search == transitions().cend())
  {
    throw std::invalid_argument("Transition " + tname + " doesn't exist");
  }
  const auto& t = *search;

  for (const auto& arc : t.pre)
  {
    const auto place_cit = places().find(arc.first);
    if (place_cit == end(places()))
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
