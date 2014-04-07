#include <algorithm> // any_of
#include <iostream>
#include <string>
#include <unordered_map>

#include "parsers/bpn.hh"
#include "parsers/helpers.hh"
#include "parsers/parse_error.hh"

namespace pnmc { namespace parsers {

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

  // Pragmas.
  while ((in >> std::ws).peek() == std::char_traits<char>::to_int_type('!'))
  {
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

  in >> kw("places") >> sharp() >> interval();

  // Initial place(s).
  in >> kw("initial") >> s0;
  unsigned int nb_initial_places = 0;
  if (s0 == "place")
  {
    nb_initial_places = 1;
  }
  else if (s0 == "places")
  {
    in >> sharp(nb_initial_places);
  }
  else
  {
    throw parse_error("Expected 'place' or 'places' got " + s0);
  }

  std::vector<std::string> initial_places;
  initial_places.reserve(nb_initial_places);
  for (; nb_initial_places > 0; --nb_initial_places)
  {
    in >> s0;
    initial_places.push_back(s0);
  }

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

  // Set marking of initial places.
  std::for_each( initial_places.cbegin(), initial_places.cend()
               , [&](const std::string& p){net.add_place(p, 1);});

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
