#include <functional>
#include <iostream>
#include <istream>
#include <string>
#include <unordered_map>

#include <rapidxml.hpp>

#include "support/parsers/mcc.hh"
#include "support/parsers/parse_error.hh"
#include "support/properties/formulae.hh"

namespace pnmc { namespace parsers {

using std::bind;
using namespace std::string_literals;
using std::placeholders::_1;

/*------------------------------------------------------------------------------------------------*/

namespace /* anonymous */ {

/*------------------------------------------------------------------------------------------------*/

struct context
{
  bool& deadlock;
  std::set<std::string>& bounds;
  std::set<std::string>& transitions;
  std::set<std::string>& targets;
};

/*------------------------------------------------------------------------------------------------*/

std::string
id(const rapidxml::xml_node<>* node)
{
  if (const auto id_node = node->first_node("id"))
  {
    return id_node->value();
  }
  throw parse_error("id expected");
}

/*------------------------------------------------------------------------------------------------*/

bool
tag(const rapidxml::xml_node<>* node, const std::string& tag_str)
{
  if (const auto tag_node = node->first_node(tag_str.c_str()))
  {
    if (const auto value = tag_node->value())
    {
      if      (value == "true"s)  {return true;}
      else if (value == "false"s) {return false;}
      throw parse_error(tag_str + " has incorrect value " + value);
    }
    throw parse_error(tag_str + " has no value");
  }
  throw parse_error(tag_str + " expected");
}

/*------------------------------------------------------------------------------------------------*/

void
tags(const rapidxml::xml_node<>* node)
{
  if (const auto tags_node = node->first_node("tags"))
  {
    if (not tag(tags_node, "is-structural"s) and not tag(tags_node, "is-reachability"s))
    {
      throw parse_error("Not a structural or reachability property");
    }
    return;
  }
  throw parse_error("tags expected");
}

/*------------------------------------------------------------------------------------------------*/

// For recursive calls.
properties::formula
formula(context&, const rapidxml::xml_node<>*);

/*------------------------------------------------------------------------------------------------*/

template <typename Result>
properties::formula
unary(context& cxt, const rapidxml::xml_node<>* node)
{
  properties::formula rec = formula(cxt, node->first_node());
  if (properties::boolean_expression* ptr = boost::get<properties::boolean_expression>(&rec))
  {
    return properties::boolean_expression{Result{std::move(*ptr)}};
  }
  throw parse_error("boolean expression expected");
}

/*------------------------------------------------------------------------------------------------*/

template <typename Result, typename Expression>
properties::formula
nary(context& cxt, const rapidxml::xml_node<>* node)
{
  std::vector<Expression> subs;
  auto sub_node = node->first_node();
  while (sub_node)
  {
    properties::formula rec = formula(cxt, sub_node);
    if (Expression* ptr = boost::get<Expression>(&rec))
    {
      subs.emplace_back(std::move(*ptr));
    }
    else
    {
      throw parse_error("Incorrect sub expression type");
    }
    sub_node = sub_node->next_sibling();
  }
  return Expression{Result{std::move(subs)}};
}

/*------------------------------------------------------------------------------------------------*/

template <typename Result, typename Expression>
properties::formula
binary(context& cxt, const rapidxml::xml_node<>* node)
{
  Expression* lhs;
  auto sub_node = node->first_node();
  properties::formula rec = formula(cxt, sub_node);
  if (not (lhs = boost::get<Expression>(&rec)))
  {
    throw parse_error("Incorrect sub expression type");
  }

  sub_node = sub_node->next_sibling();
  Expression* rhs;
  rec = formula(cxt, sub_node);
  if (not (rhs = boost::get<Expression>(&rec)))
  {
    throw parse_error("Incorrect sub expression type");
  }

  return Expression{Result{std::move(*lhs), std::move(*rhs)}};
}

/*------------------------------------------------------------------------------------------------*/

template <typename Result>
properties::formula
comparison(context& cxt, const rapidxml::xml_node<>* node)
{
  properties::integer_expression* lhs;
  auto sub_node = node->first_node();
  properties::formula rec = formula(cxt, sub_node);
  if (not (lhs = boost::get<properties::integer_expression>(&rec)))
  {
    throw parse_error("integer expression expected");
  }

  sub_node = sub_node->next_sibling();
  properties::integer_expression* rhs;
  rec = formula(cxt, sub_node);
  if (not (rhs = boost::get<properties::integer_expression>(&rec)))
  {
    throw parse_error("integer expression expected");
  }

  return properties::boolean_expression{Result{std::move(*lhs), std::move(*rhs)}};
}

/*------------------------------------------------------------------------------------------------*/

properties::formula
integer_constant(context&, const rapidxml::xml_node<>* node)
{
  if (const auto* value = node->value())
  {
    try
    {
      return properties::integer_expression{properties::integer_constant{std::stoi(value)}};
    }
    catch (const std::invalid_argument&)
    {}
  }
  throw parse_error("no value for integer constant");
}

/*------------------------------------------------------------------------------------------------*/

properties::formula
place_bound(context& cxt, const rapidxml::xml_node<>* node)
{
  std::set<std::string> places;
  auto* place_node = node->first_node("place");
  while (place_node)
  {
    auto p = place_node->value();
    places.emplace(p);
    cxt.bounds.emplace(std::move(p));
    place_node = place_node->next_sibling();
  }
  auto vec = std::vector<std::string>{begin(places), end(places)};
  return properties::integer_expression{properties::place_bound{std::move(vec)}};
}

/*------------------------------------------------------------------------------------------------*/

//properties::formula
//tokens_count(context& cxt, const rapidxml::xml_node<>* node)
//{
//  std::set<std::string> places;
//  auto* place_node = node->first_node("place");
//  while (place_node)
//  {
//    auto p = place_node->value();
//    places.emplace(p);
//    cxt.targets.emplace(std::move(p));
//    place_node = place_node->next_sibling();
//  }
//  auto vec = std::vector<std::string>{begin(places), end(places)};
//  return properties::integer_expression{properties::place_bound{std::move(vec)}};
//}

/*------------------------------------------------------------------------------------------------*/

properties::formula
unsupported(context&, const rapidxml::xml_node<>* node)
{
  throw unsupported_error(node->name());
}

/*------------------------------------------------------------------------------------------------*/

template <typename Result>
properties::formula
element(context&, const rapidxml::xml_node<>*)
{
  return properties::boolean_expression{Result{}};
}

/*------------------------------------------------------------------------------------------------*/

properties::formula
is_fireable(context& cxt, const rapidxml::xml_node<>* node)
{
  std::set<std::string> transitions;
  auto* transition_node = node->first_node("transition");
  while (transition_node)
  {
    auto t = transition_node->value();
    transitions.emplace(t);
    cxt.transitions.emplace(std::move(t));
    transition_node = transition_node->next_sibling();
  }
  auto vec = std::vector<std::string>{begin(transitions), end(transitions)};
  return properties::boolean_expression{properties::is_fireable{std::move(vec)}};
}

/*------------------------------------------------------------------------------------------------*/

using fn_type = std::function<properties::formula (context&, const rapidxml::xml_node<>*)>;
const std::unordered_map<std::string, fn_type> map =
  { {"invariant"             , unsupported}
  , {"impossibility"         , unary<properties::impossibility>}
  , {"possibility"           , unary<properties::possibility>}
  , {"possibility"           , unary<properties::possibility>}
  , {"all−paths"             , unsupported}
  , {"exists−paths"          , unsupported}
  , {"globally"              , unsupported}
  , {"finally"               , unsupported}
  , {"next"                  , unsupported}
  , {"until"                 , unsupported}
  , {"before"                , unsupported}
  , {"reach"                 , unsupported}
  , {"strength"              , unsupported}
  , {"deadlock"              , [&](auto& cxt, const auto& n)
                                  {
                                    cxt.deadlock = true;
                                    return element<properties::deadlock>(cxt, n);
                                  }}
  , {"is-fireable"           , is_fireable}
  , {"true"                  , element<properties::true_>}
  , {"false"                 , element<properties::false_>}
  , {"negation"              , unary<properties::negation>}
  , {"conjunction"           , nary< properties::conjunction
                                   , properties::boolean_expression>}
  , {"disjunction"           , nary< properties::disjunction
                                   , properties::boolean_expression>}
  , {"exclusive-disjunction" , nary< properties::exclusive_disjonction
                                   , properties::boolean_expression>}
  , {"implication"           , binary< properties::implication
                                     , properties::boolean_expression>}
  , {"equivalence"           , binary< properties::equivalence
                                     , properties::boolean_expression>}
  , {"integer-eq"            , comparison<properties::integer_eq>}
  , {"integer-ne"            , comparison<properties::integer_ne>}
  , {"integer-lt"            , comparison<properties::integer_lt>}
  , {"integer-le"            , comparison<properties::integer_le>}
  , {"integer-gt"            , comparison<properties::integer_gt>}
  , {"integer-ge"            , comparison<properties::integer_ge>}
  , {"integer−constant"      , integer_constant}
  , {"integer−sum"           , nary< properties::integer_sum
                                   , properties::integer_expression>}
  , {"integer−product"       , nary< properties::integer_product
                                   , properties::integer_expression>}
  , {"integer−difference"    , binary< properties::integer_difference
                                     , properties::integer_expression>}
  , {"integer−division"      , binary< properties::integer_division
                                     , properties::integer_expression>}
  , {"place-bound"           , place_bound}
  , {"tokens-count"          , unsupported}
  };

/*------------------------------------------------------------------------------------------------*/

properties::formula
formula(context& cxt, const rapidxml::xml_node<>* node)
{
  if (node)
  {
    try
    {
      return map.at(node->name())(cxt, node);
    }
    catch (const std::out_of_range &)
    {
      throw parse_error("invalid node in formula: "s + node->name());
    }
  }
  throw parse_error("formula expected");
}

/*------------------------------------------------------------------------------------------------*/

properties::formula
top_formula(context& cxt, const rapidxml::xml_node<>* node)
{
  if (const auto formula_node = node->first_node("formula"))
  {
    return formula(cxt, formula_node->first_node());
  }
  throw parse_error("top formula expected");
}

/*------------------------------------------------------------------------------------------------*/

std::pair<std::string, properties::formula>
property(context& cxt, const rapidxml::xml_node<>* node)
{
  tags(node);
  return {id(node), top_formula(cxt, node)};
}

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

properties::formulae
mcc(std::istream& in)
{
  using namespace rapidxml;

  properties::formulae formulae;

  std::string buffer{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>{}};
  if (buffer.empty())
  {
    throw parsers::parse_error("MCC formula parser: empty file");
  }

  rapidxml::xml_document<> doc;
  try
  {
    doc.parse<0>(&buffer[0]);
  }
  catch (const rapidxml::parse_error& p)
  {
    throw parsers::parse_error(p.what());
  }

  if (const auto properties_node = doc.first_node("property-set"))
  {
    std::set<std::string> targets;
    auto cxt = context{ formulae.compute_deadlock, formulae.places_bounds
                      , formulae.fireable_transitions, targets};

    auto node = properties_node->first_node();
    while (node)
    {
      try
      {
        auto p = property(cxt, node);
        if (auto* bptr = boost::get<properties::boolean_expression>(&p.second))
        {
          auto boolean = properties::formulae::boolean{p.first, targets, std::move(*bptr)};
          formulae.booleans.emplace_back(std::move(boolean));
        }
        else if (auto* iptr = boost::get<properties::integer_expression>(&p.second))
        {
          auto integer = properties::formulae::integer{p.first, targets, std::move(*iptr)};
          formulae.integers.emplace_back(std::move(integer));
        }
      }
      catch (const unsupported_error& e)
      {
        std::cerr << "Unsupported: '" << e.what() << "'\n";
      }
      targets.clear();
      node = node->next_sibling("property");
    }
  }
  else
  {
    throw parse_error("Unvalid properties file");
  }

  return formulae;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
