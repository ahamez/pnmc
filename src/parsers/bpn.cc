#include <algorithm> // any_of
#include <iostream>
#include <string>
#include <unordered_map>

#include "parsers/bpn.hh"
#include "parsers/helpers.hh"
#include "parsers/parse_error.hh"

namespace pnmc { namespace parsers {

// BPN grammar
// see http://cadp.inria.fr/man/caesar.bdd.html
//
// <basic-petri-net> ::= 
//    places #<nb-of-places> <min-place-nb>...<max-place-nb>\n
//    initial place <init-place-nb>\n
//    units #<nb-of-units> <min-unit-nb>...<max-unit-nb>\n
//    root unit <root-unit-nb>\n
//    <unit-description>*\n
//    transitions #<nb-of-trans> <min-trans-nb>...<max-trans-nb>\n
//    <trans-description>*\n
// <unit-description> ::=
//    U<unit-nb>
//    #<nb-of-subplaces> <min-subplace-nb>...<max-subplace-nb>
//    #<nb-of-subunits> <subunit-list>\n
// <trans-description> ::=
//    T<transition-nb> 
//    #<nb-of-input-places> <input-place-list>
//    #<nb-of-output-places> <output-place-list>\n
// <input-place-list> ::= <place-nb>*
// <output-place-list> ::= <place-nb>*
// <subunit-list> ::= <unit-nb>*
// <nb-of-places>        ::= <unsigned-integer>
// <min-place-nb>        ::= <unsigned-integer>
// <max-place-nb>        ::= <unsigned-integer>
// <init-place-nb>       ::= <unsigned-integer>
// <place-nb>            ::= <unsigned-integer>
// <nb-of-units>         ::= <unsigned-integer>
// <min-unit-nb>         ::= <unsigned-integer>
// <max-unit-nb>         ::= <unsigned-integer>
// <root-unit-nb>        ::= <unsigned-integer>
// <unit-nb>             ::= <unsigned-integer>
// <nb-of-trans>         ::= <unsigned-integer>
// <min-trans-nb>        ::= <unsigned-integer>
// <max-trans-nb>        ::= <unsigned-integer>
// <nb-of-subplaces>     ::= <unsigned-integer>
// <min-subplace-nb>     ::= <unsigned-integer>
// <max-subplace-nb>     ::= <unsigned-integer>
// <nb-of-subunits>      ::= <unsigned-integer>
// <transition-nb>       ::= <unsigned-integer>
// <nb-of-input-places>  ::= <unsigned-integer>
// <nb-of-output-places> ::= <unsigned-integer>
//
// A valid BPN file should satisfy the following constraints: 
//
// In <basic-petri-net>:
//   1.  <nb-of-places> > 0     -- a net has at least one place
//   2.  <max-place-nb> - <min-place-nb> + 1 = <nb-of-places>
//   3.  <min-place-nb> <= <init-place-nb> <= <max-place-nb>
//   4.  <nb-of-units> > 0     -- a net has at least one unit
//   5.  <max-unit-nb> - <min-unit-nb> + 1 = <nb-of-units>
//   6.  <min-unit-nb> <= <root-unit-nb> <= <max-unit-nb>
//   7.  <nb-of-trans> >= 0    -- a net may have zero transition
//   8.  <nb-of-trans> = 0 => <min-trans-nb> = 1
//   9.  <nb-of-trans> = 0 => <max-trans-nb> = 0
//  10.  <max-trans-nb> - <min-trans-nb> + 1 = <nb-of-trans>
// In each <unit-description>:
//  11.  <min-unit-nb> <= <unit-nb> <= <max-unit-nb>
//  12.  <nb-of-subplaces> > 0 -- a unit has at least one local place
//  13.  <min-place-nb> <= <min-subplace-nb> <= <max-place-nb>
//  14.  <min-place-nb> <= <max-subplace-nb> <= <max-place-nb>
//  15.  <max-subplace-nb> - <min-sublace-nb> + 1 = <nb-of-subplaces>
//  16.  <nb-of-subunits> >= 0
//  17.  length (<subunit-list>) = <nb-of-subunits>
// Globally to all <unit-description>s:
//  18.  each <unit-nb> occurs once and only once after a "U"
//  19.  the sum of all <nb-of-subplaces> is equal to <nb-of-places>
//  20.  all intervals <min-subplace-nb>...<max-subplace-nb> form
//       a partition of <min-place-nb>...<max-place-nb>
//  21.  the sum of all <nb-of-subunits> is equal to <nb-of-units>
//  22.  <root-unit-number> and all non-empty <subunit-list>s form
//       a partition of <min-unit-nb>...<max-unit-nb>
// In each <subunit-list>:
//  23.  <min-unit-nb> <= <unit-nb> <= <max-unit-nb>
// In each <trans-description>:
//  24.  <min-trans-nb> <= <trans-nb> <= <max-trans-nb>
//  25.  length (<input-place-list>) = <nb-of-input-places>
//  26.  length (<output-place-list>) = <nb-of-output-places>
// Globally to all <trans-description>s:
//  27.  each <trans-nb> occurs once and only once after a "T"
// In each <input-place-list> and each <output-place-list>
//  28. <min-place-nb> <= <place-nb> <= <max-place-nb>

namespace {

/*------------------------------------------------------------------------------------------------*/

struct uint
{
  unsigned int& res_;

  uint(unsigned int& res)
    : res_(res)
  {}

  friend
  std::istream&
  operator>>(std::istream& in, const uint& manip)
  {
    std::string s;
    if (not (in >> s))
    {
      throw parse_error("Expected a value");
    }
    try
    {
      manip.res_ = std::stoi(s);
    }
    catch (const std::invalid_argument&)
    {
      throw parse_error("Expected a value, got " + s);
    }
    return in;
  }
};

/*------------------------------------------------------------------------------------------------*/

struct sharp
{
  unsigned int* res_;

  sharp(unsigned int& res)
    : res_(&res)
  {}

  sharp()
    : res_(nullptr)
  {}

  friend
  std::istream&
  operator>>(std::istream& in, const sharp& manip)
  {
    std::string s;
    if (not (in >> s))
    {
      throw parse_error("Expected a #value");
    }
    try
    {
      if (s[0] != '#')
      {
        throw parse_error("Expected a #value, got " + s);
      }
      const auto res = std::stoi(s.substr(1));
      if (manip.res_)
      {
        *manip.res_ = res;
      }
    }
    catch (const std::invalid_argument&)
    {
      throw parse_error("Expected a #value, got " + s);
    }
    return in;
  }
};

/*------------------------------------------------------------------------------------------------*/

struct interval
{
  unsigned int* first_;
  unsigned int* last_;

  interval(unsigned int& first, unsigned int& last)
    : first_(&first), last_(&last)
  {}

  interval()
    : first_(nullptr), last_(nullptr)
  {}

  friend
  std::istream&
  operator>>(std::istream& in, const interval& manip)
  {
    unsigned int first, last;
    std::size_t pos;

    std::string s;
    if (not (in >> s))
    {
      throw parse_error("Expected an interval");
    }

    try
    {
      first = std::stoi(s, &pos);
      auto it = s.cbegin() + pos;
      if (std::distance(it, s.cend()) < 3 or std::any_of(it, it + 3, [](char c){return c != '.';}))
      {
        throw parse_error("Expected '...' in interval, got " + s);
      }
      std::advance(it, 3);
      last = std::stoi(std::string(it, s.cend()));
    }
    catch (const std::invalid_argument&)
    {
      throw parse_error("Expected a value in interval, got " + s);
    }

    if (manip.first_)
    {
      *manip.first_ = first;
      *manip.last_  = last;
    }
    return in;
  }
};

/*------------------------------------------------------------------------------------------------*/

struct prefix
{
  const char p_;
  std::string* res_;

  prefix(char p, std::string& res)
    : p_(p), res_(&res)
  {}

  prefix(char p)
    : p_(p), res_(nullptr)
  {}

  friend
  std::istream&
  operator>>(std::istream& in, const prefix& manip)
  {
    std::string s;

    if (not (in >> s))
    {
      throw parse_error("Expected a prefixed value");
    }
    if (s[0] != manip.p_)
    {
      throw parse_error("Expected a prefixed value, got " + s);
    }
    if (manip.res_)
    {
      *manip.res_ = s;
    }

    return in;
  }
};

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
bpn(std::istream& in)
{
  auto net_ptr = std::make_shared<pn::net>();
  auto& net = *net_ptr;

  // temporary string placeholders
  std::string s0, s1;

  // map a unit id to its corresponding module and its nested modules
  using module_places = std::pair< pn::module                 // corresponding module
                                 , std::vector<std::string>>; // nested modules' names
  std::unordered_map<std::string, module_places> modules;

  in >> kw("places") >> sharp() >> interval();

  unsigned int initial_place;
  in >> kw("initial") >> kw("place") >> uint(initial_place);

  unsigned int nb_units;
  in >> kw("units") >> sharp(nb_units) >> interval();

  // units
  unsigned int root_module_nb;
  in >> kw("root") >> kw("unit") >> uint(root_module_nb);

  const auto root_module = std::to_string(root_module_nb);

  while (nb_units > 0)
  {
    unsigned int nb_nested_units, first, last;
    std::string module_name;
    in >> prefix('U', module_name) >> sharp() >> interval(first, last) >> sharp(nb_nested_units);

    pn::module_node m(module_name);
    for (unsigned int i = first; i <= last; ++i)
    {
      const auto& p = net.add_place(std::to_string(i), 0);
      m.add_module(pn::make_module(p));
    }
    const auto insertion = modules.emplace( module_name
                                          , std::make_pair( pn::make_module(m)
                                                          , std::vector<std::string>()));

    // nested modules
    for (unsigned int i = 0; i < nb_nested_units; ++i)
    {
      if (not(in >> s1))
      {
        throw parse_error();
      }
      // keep the name of the nested unit
      insertion.first->second.second.push_back("U" + s1);
    }

    --nb_units;
  }

  // transitions
  unsigned int nb_transitions;
  in >> kw("transitions") >> sharp(nb_transitions) >> interval();
  while (nb_transitions > 0)
  {
    // input places
    unsigned int nb_places;
    std::string transition_id;
    in >> prefix('T', transition_id) >> sharp(nb_places);

    net.add_transition(transition_id);

    while (nb_places > 0)
    {
      if (not (in >> s0))
      {
        throw parse_error();
      }
      net.add_pre_place(transition_id, s0, 1);
      --nb_places;
    }

    // output places
    in >> sharp(nb_places);
    while (nb_places > 0)
    {
      if (not (in >> s0))
      {
        throw parse_error();
      }
      net.add_post_place(transition_id, s0, 1);
      --nb_places;
    }

    --nb_transitions;
  }

  net.add_place(std::to_string(initial_place), 1);

  for (auto& kv : modules)
  {
    for (const auto& module_name : kv.second.second)
    {
      const auto nested_module = modules[module_name].first;
      boost::get<pn::module_node>(*kv.second.first).add_module(nested_module);
    }
  }
  net.modules = modules["U" + root_module].first;

  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
