#include <algorithm> // any_of
#include <iostream>
#include <string>
#include <unordered_map>

#include "support/parsers/nupn.hh"
#include "support/parsers/parse_error.hh"

namespace pnmc { namespace parsers {

namespace {

/*------------------------------------------------------------------------------------------------*/

/// @brief Input stream manipulator to detect a keyword.
struct kw
{
  const std::string k;

  friend
  std::istream&
  operator>>(std::istream& in, const kw& manip)
  {
    std::string s;
    if      (not (in >> s)) {throw parse_error("Expected " + manip.k + ", got nothing.");}
    else if (s != manip.k) {throw parse_error("Expected " + manip.k + ", got " + s);}
    return in;
  }
};

/*------------------------------------------------------------------------------------------------*/

struct uint
{
  unsigned int& res_;

  friend
  std::istream&
  operator>>(std::istream& in, const uint& manip)
  {
    std::string s;
    if (not (in >> s)) {throw parse_error("Expected a value");}
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
    if (not (in >> s)) {throw parse_error("Expected a #value");}
    try
    {
      if (s[0] != '#') {throw parse_error("Expected a #value, got " + s);}
      const auto res = std::stoi(s.substr(1));
      if (manip.res_) {*manip.res_ = res;}
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
    if (not (in >> s)) {throw parse_error("Expected an interval");}

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
  const char p;
  std::string& res;

  friend
  std::istream&
  operator>>(std::istream& in, const prefix& manip)
  {
    if (not (in >> manip.res))   {throw parse_error("Expected a prefixed value");}
    if (manip.res[0] != manip.p) {throw parse_error("Expected a prefixed value, got " + manip.res);}
    return in;
  }
};

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
nupn(std::istream& in)
{
  auto net_ptr = std::make_shared<pn::net>();
  auto& net = *net_ptr;

  // temporary placeholders
  std::string s0, s1;
  unsigned int i0;

  // Pragmas.
  while ((in >> std::ws).peek() == std::char_traits<char>::to_int_type('!'))
  {
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

  // The number of places and their ranges. Ignored.
  in >> kw{"places"} >> sharp{} >> interval{};

  // Initial place(s).
  const auto nb_initial_places = [&]
  {
    in >> kw{"initial"} >> s0;
    if      (s0 == "place")  {return 1u;}
    else if (s0 == "places") {in >> sharp{i0}; return i0;}
    else                     {throw parse_error("Expected 'place' or 'places' got " + s0);}
  }();

  const auto initial_places = [&]
  {
    std::vector<std::string> res;
    res.reserve(nb_initial_places);
    for (auto i = 0u; i < nb_initial_places; ++i)
    {
      in >> s0;
      res.push_back(s0);
    }
    return res;
  }();

  const auto nb_units = [&]{in >> kw{"units"} >> sharp{i0} >> interval{}; return i0;}();

  // units
  const auto root_module = [&]
  {
    auto root_module_nb = 0u;
    in >> kw{"root"} >> kw{"unit"} >> uint{root_module_nb};
    return std::to_string(root_module_nb);
  }();

  for (auto i = 0u; i < nb_units; ++i)
  {
    unsigned int nb_nested_units, first, last;
    in >> prefix{'U', s1} >> sharp{} >> interval{first, last} >> sharp{nb_nested_units};

    for (unsigned int j = first; j <= last; ++j)
    {
      net.add_place(std::to_string(j), 0);
    }

    // nested modules
    for (unsigned int j = 0; j < nb_nested_units; ++j)
    {
      if (not(in >> s1)) {throw parse_error();}
    }
  }

  // transitions
  const auto nb_transitions = [&]{in >> kw{"transitions"} >> sharp{i0} >> interval{}; return i0;}();

  for (auto i = 0u; i < nb_transitions; ++i)
  {
    const auto transition_id = [&]{in >> prefix{'T', s1}; return s1;}();
    net.add_transition(transition_id);

    // input places
    const auto nb_input_places = [&]{in >> sharp{i0}; return i0;}();
    for (auto j = 0u; j < nb_input_places; ++j)
    {
      if (not (in >> s0)) {throw parse_error();}
      net.add_pre_place(transition_id, s0, 1, pn::arc::type::normal);
    }

    // output places
    const auto nb_output_places = [&]{in >> sharp{i0}; return i0;}();
    for (auto j = 0u; j < nb_output_places; ++j)
    {
      if (not (in >> s0)) {throw parse_error();}
      net.add_post_place(transition_id, s0, 1, pn::arc::type::normal);
    }
  }

  // Set marking of initial places.
  std::for_each( begin(initial_places), end(initial_places)
               , [&](const auto& p){net.add_place(p, 1);});

  net.format = conf::pn_format::nupn;
  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
