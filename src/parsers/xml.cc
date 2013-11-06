#include <algorithm> // all_of
#include <cstdlib>   // atoi
#include <cstring>   // strlen, strncmp
#include <istream>
#include <string>

#include "parsers/parse_error.hh"
#include "parsers/xml.hh"
#include "parsers/rapidxml/rapidxml.hpp"

namespace pnmc { namespace parsers {

namespace {

/*------------------------------------------------------------------------------------------------*/

pn::module
xml_parse_place(std::shared_ptr<pn::net> net, rapidxml::xml_node<>* node)
{
  using namespace rapidxml;
  if (std::strncmp(node->name(), "place", std::strlen("place")) == 0)
  {
    const std::string id(node->first_attribute("id")->value());
    const unsigned int marking(std::atoi(node->first_attribute("marking")->value()));
    return pn::make_module(net->add_place(id, marking));
  }
  else if (std::strncmp(node->name(), "module", std::strlen("module")) == 0)
  {
    const std::string name(node->first_attribute("name")->value());
    pn::module_node mn(name);
    node = node->first_node();
    while (node)
    {
      mn.add_module(xml_parse_place(net, node));
      node = node->next_sibling();
    }
    return pn::make_module(mn);
  }
  else
  {
    throw parse_error();
  }
}

/*------------------------------------------------------------------------------------------------*/

void
xml_parse_transition(std::shared_ptr<pn::net> net, rapidxml::xml_node<>* node)
{
  using namespace rapidxml;
  if (std::strncmp(node->name(), "transition", std::strlen("transition")) == 0)
  {
    const std::string id(node->first_attribute("id")->value());
    net->add_transition(id);
    auto ref = node->first_node();
    while (ref)
    {
      const unsigned int valuation = std::atoi(ref->first_attribute("weight")->value());
      const std::string rid(ref->first_attribute("ref")->value());
      if (std::strncmp(ref->name(), "pre", std::strlen("pre")) == 0)
      {
        net->add_pre_place(id, rid, valuation);
      }
      else if (std::strncmp(ref->name(), "post", std::strlen("post")) == 0)
      {
        net->add_post_place(id, rid, valuation);
      }
      else
      {
        throw parse_error();
      }
      ref = ref->next_sibling();
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

struct xml_only_places
  : public boost::static_visitor<bool>
{
  bool
  operator()(const pn::place*)
  const noexcept
  {
    return true;
  }

  bool
  operator()(const pn::module_node&)
  const noexcept
  {
    return false;
  }
};

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
xml(std::istream& in)
{
  using namespace rapidxml;

  std::string buffer;
  buffer.reserve(4096);
  {
    std::string line;
    while(std::getline(in,line))
    {
      buffer += line;
    }
  }

  rapidxml::xml_document<> doc;
  try
  {
    doc.parse<0>(&buffer[0]); // is it really OK to modify the content of the string?
  }
  catch (const rapidxml::parse_error& p)
  {
    throw parse_error(p.what());
  }

  auto net_ptr = std::make_shared<pn::net>();

  xml_node<>* node = doc.first_node();
  net_ptr->name = node->first_attribute("name")->value();

  pn::module_node mn(net_ptr->name);

  // Places.
  auto place = node->first_node("places")->first_node();
  while (place)
  {
    mn.add_module(xml_parse_place(net_ptr, place));
    place = place->next_sibling();
  }

  // Transitions.
  auto transition = node->first_node("transitions")->first_node();
  while (transition)
  {
    xml_parse_transition(net_ptr, transition);
    transition = transition->next_sibling();
  }

  const bool only_places = std::all_of( mn.nested.begin(), mn.nested.end()
                                      , [](const pn::module& m)
                                          {
                                            return boost::apply_visitor(xml_only_places(), *m);
                                          }
                                      );
  if (not only_places)
  {
    net_ptr->modules = make_module(mn);
  }

  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
