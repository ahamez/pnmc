#include <cassert>
#include <sstream>
#include <stdexcept>

#include "pn/net.hh"
#include "pn/arc.hh"
#include "pn/place.hh"
#include "pn/transition.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

net::net()
  : name()
  , places_set()
  , transitions_set()
  , modules(nullptr)
{}

/*------------------------------------------------------------------------------------------------*/

const place&
net::add_place(const std::string& pid, const std::string& label, unsigned int marking)
{
  const auto insertion = places_set.insert({pid, label, marking});
  if (not insertion.second)
  {
    std::stringstream ss;
    ss << "Place " << pid << " already exists" << std::endl;
    throw std::runtime_error(ss.str());
  }
  return *insertion.first;
}

/*------------------------------------------------------------------------------------------------*/

const transition&
net::add_transition(const std::string& tid, const std::string& label)
{
  const auto insertion = transitions_set.insert({tid, label});
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
  auto it = transitions_set.get<id_index>().find(tid);
  if(it != transitions_set.get<id_index>().end())
  {
    transitions_set.modify(it, transition::add_post_place(a,post));
  }
  else
  {
    assert(false);
  }
}

/*------------------------------------------------------------------------------------------------*/

void
net::add_pre_place(const std::string& tid, const std::string& post, const arc& a)
{
  auto it = transitions_set.get<id_index>().find(tid);
  if(it != transitions_set.get<id_index>().end())
  {
    transitions_set.modify(it, transition::add_pre_place(a,post));
  }
  else
  {
    assert(false);
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

}} // namespace pnmc::pn
