/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <istream>
#include <string>

#include <rapidxml.hpp>

#include "support/parsers/parse_error.hh"
#include "support/parsers/pnml.hh"

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
pnml(std::istream& in)
{
  using namespace rapidxml;

  std::string buffer(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>{});
  if (buffer.empty())
  {
    throw parse_error("PNML parser: empty file");
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

  const std::string type_str       = "http://www.pnml.org/version-2009/grammar/ptnet";
  const std::string place_str      = "place";
  const std::string transition_str = "transition";
  const std::string arc_str        = "arc";

  auto net_ptr = std::make_shared<pn::net>();

  const auto pnml_node = doc.first_node();
  const auto net_node = pnml_node->first_node("net");
  net_ptr->name = net_node->first_attribute("id")->value();
  const auto type = net_node->first_attribute("type")->value();
  if (type_str != type)
  {
    throw parse_error("Unsupported Petri net type " + std::string(type));
  }
  const auto page_node = net_node->first_node("page");

  //  First, parse all places and transitions.
  auto node = page_node->first_node();
  while (node)
  {
    if (place_str == node->name())
    {
      const auto id = node->first_attribute("id")->value();
      auto marking = 0u;
      const auto marking_node = node->first_node("initialMarking");
      if (marking_node)
      {
        const auto text = marking_node->first_node("text")->value();
        try
        {
          marking = std::stoi(text);
        }
        catch (const std::invalid_argument&)
        {
          throw parse_error("Expected a integer for marking, got " + std::string(text));
        }
      }
      net_ptr->add_place(id, marking);
    }
    else if (transition_str == node->name())
    {
      net_ptr->add_transition(node->first_attribute("id")->value());
    }
    node = node->next_sibling();
  }

  // Then, parse all arcs.
  node = page_node->first_node();
  while (node)
  {
    if (arc_str == node->name())
    {
      const auto src = node->first_attribute("source")->value();
      const auto dst = node->first_attribute("target")->value();
      auto valuation = 1u;
      const auto inscription_node = node->first_node("inscription");
      if (inscription_node)
      {
        const auto text = inscription_node->first_node("text")->value();
        try
        {
          valuation = std::stoi(text);
        }
        catch (const std::invalid_argument&)
        {
          throw parse_error("Expected an integer for valuation, got " + std::string(text));
        }
      }
      if (net_ptr->places().find(src) != end(net_ptr->places()))
      {
        // src is a place.
        net_ptr->add_pre_place(dst, src, valuation, pn::arc::type::normal);
      }
      else
      {
        // src is a transitions.
        net_ptr->add_post_place(src, dst, valuation, pn::arc::type::normal);
      }
    }
    node = node->next_sibling();
  }
  net_ptr->format = conf::pn_format::pnml;
  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
