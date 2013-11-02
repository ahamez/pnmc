#include <exception>
#include <istream>
#include <string>
#include <tuple>
#include <unordered_map>

#include "parsers/bpn.hh"

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

/*------------------------------------------------------------------------------------------------*/

struct parse_error final
  : public std::exception
{
public:

  ~parse_error()
  noexcept
  {}

  const char*
  what()
  const noexcept
  {
    return "";
  }
};

/*------------------------------------------------------------------------------------------------*/

void
keyword(const std::string& lhs, const std::string& rhs)
{
  if (lhs != rhs)
  {
    throw parse_error();
  }
}

std::string
prefix(const std::string& lhs, char c)
{
  if (lhs[0] != c)
  {
    throw parse_error();
  }
  return lhs.substr(1);
}

unsigned int
uint(const std::string& s)
{
  try
  {
    return std::stoi(s);
  }
  catch (const std::invalid_argument&)
  {
    throw parse_error();
  }
}

unsigned int
sharp(const std::string& s)
{
  if (s[0] != '#')
  {
    throw parse_error();
  }
  return uint(s.substr(1));
}

std::pair<unsigned int, unsigned int>
interval(const std::string& s)
{
  unsigned int first;
  unsigned int last;
  std::size_t pos;

  try
  {
    first = std::stoi(s, &pos);
  }
  catch (const std::invalid_argument&)
  {
    throw parse_error();
  }

  auto cit = s.cbegin() + pos;
  for (auto i = 0; i < 3; ++i) // three more dots
  {
    if (cit != s.cend() and *cit == '.')
    {
      ++cit;
      continue;
    }
    else
    {
      throw parse_error();
    }
  }

  try
  {
    last = std::stoi(std::string(cit, s.cend()));
  }
  catch (const std::invalid_argument&)
  {
    throw parse_error();
  }

  return std::make_pair(first, last);
}

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
bpn(std::istream& in)
{
  auto net_ptr = std::make_shared<pn::net>();
  auto& net = *net_ptr;

  try
  {
    // temporary string placeholders
    std::string s0, s1, s2, s3;

    in >> s0 >> s1 >> s2;
    keyword(s0, "places"); sharp(s1); interval(s2);

    in >> s0 >> s1 >> s2;
    keyword(s0, "initial"); keyword(s1, "place");
    const auto initial_place = uint(s2);

    in >> s0 >> s1 >> s2;
    keyword(s0, "units"); interval(s2);
    auto nb_units = sharp(s1);

    // units
    in >> s0 >> s1 >> s2;
    keyword(s0, "root"); keyword(s1, "unit"); uint(s2);
    const auto root_module = "U" + s2;
    std::unordered_map<std::string, pn::module> modules;;
    while (nb_units > 0)
    {
      in >> s0 >> s1 >> s2 >> s3;
      prefix(s0, 'U');
      sharp(s1);
      unsigned int first, last;
      std::tie(first, last) = interval(s2);

      pn::module_node m(s0);
      for (auto i = first; i <= last; ++i)
      {
        const auto& p = net.add_place(std::to_string(i), "", 0);
        m.add_module(pn::make_module(p));
      }

      const auto nb_nested_units = sharp(s3);
      for (auto i = 0; i < nb_nested_units; ++i)
      {
        in >> s1;
        m.add_module(modules["U" + s1]);
      }
      modules[s0] = pn::make_module(m);

      --nb_units;
    }
    net.modules = modules[root_module];

    // transitions
    in >> s0 >> s1 >> s2;
    keyword(s0, "transitions");
    auto nb_transitions = sharp(s1);
    interval(s2);
    while (nb_transitions > 0)
    {
      // input places
      in >> s0 >> s1;
      const auto transition_id = prefix(s0, 'T');

      net.add_transition(transition_id, "");

      auto nb_places = sharp(s1);
      while (nb_places > 0)
      {
        in >> s0;
        net.add_pre_place(transition_id, s0, pn::arc());
        --nb_places;
      }

      // output places
      in >> s0;
      nb_places = sharp(s0);
      while (nb_places > 0)
      {
        in >> s0;
        net.add_post_place(transition_id, s0, pn::arc());
        --nb_places;
      }

      --nb_transitions;
    }

    net.add_place(std::to_string(initial_place), "", 1);
  }
  catch (parse_error&)
  {
    return nullptr;
  }

  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
