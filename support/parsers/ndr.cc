#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <regex>
#include <type_traits>

#include <boost/optional.hpp>

#include "support/parsers/parse_error.hh"
#include "support/parsers/ndr.hh"
#include "support/parsers/token.hh"
#include "support/pn/constants.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace parsers {

namespace {

/*------------------------------------------------------------------------------------------------*/

// @brief Start at 1 because the relative position in the enum is used to compute the type of the
// token and because submatches of regular expressions start at 1 (0 is the whole match).
enum class tk { skip = 1u, newline, float_, number, qname, name, question, exclamation, minus, mult
              , comment};

/*------------------------------------------------------------------------------------------------*/

using namespace std::string_literals;
using token = token_holder<tk>;
using parse_cxt = parse_context<tk>;

/*------------------------------------------------------------------------------------------------*/

/// @brief Returns the tokens of a Tina model.
template <typename InputIterator>
std::deque<token>
tokens(InputIterator&& begin, InputIterator&& end)
{
  // Order must match enum token_t
  static const std::regex regex
  {
    "([ \\t]+)|"                  // skip spaces and tabs
    "(\\n)|"                      // newline (to keep track of line numbers)
    "(\\d+\\.\\d+)|"              // float
    "(\\d+[KM]?)|"                // numbers can have a suffix
    "(\\{)|"                      // only search for opening brace for qualified names
    "([a-zA-Z'_][a-zA-Z0-9'_]*)|" // don't begin by a number, we add it if necessary in the parser
    "(\\?)|"                      // question mark
    "(!)|"                        // exclamation mark
    "(-)|"                        // minus
    "(\\*)|"                      // multiplication
    "(^#[^\\n]*)"                 // comments start at the beginning of a line
  };

  // To report error location, if any.
  auto line = 1ul;
  auto column = 1ul;
  std::smatch match;
  bool last_match_is_newline = true;
  std::deque<token> tokens;

  while (true)
  {
    if (begin == end) {break;}

    std::regex_search(begin, end, match, regex);

    if (match.position() != 0)
    {
      throw parse_error( "Unexpected '" + match.prefix().str() + "' at " + std::to_string(line)
                       + ':' + std::to_string(column));
    }

    column += match.length();
    begin += match.length();

    if (match[pos(tk::skip)].matched)
    {
      continue;
    }

    if (match[pos(tk::newline)].matched)
    {
      last_match_is_newline = true;
      column = 1;
      line += 1;
      tokens.emplace_back(token{tk::newline, "\\n", line, column});
      continue;
    }

    if (match[pos(tk::comment)].matched)
    {
      if (not last_match_is_newline)
      {
        throw parse_error("Invalid comment at line " + std::to_string(line));
      }
      continue;
    }
    last_match_is_newline = false;

    // Get the type of token by looking for the first successful submatch.
    const auto ty = [&match]
    {
      auto t = pos(tk::float_);
      for (; t < match.size(); ++t)
      {
        if (match[t].matched) {break;}
      }
      return t;
    }();
    assert(ty < match.size());

    // Process qualified names manually (to handle despecialization).
    if (ty == pos(tk::qname))
    {
      std::string qname;
      qname.reserve(32);
      bool despecialization = false;
      while (true)
      {
        if (despecialization)
        {
          qname.push_back(*begin);
          despecialization = false;
        }
        else
        {
          if      (*begin == '}')  {++begin; ++column; break;}
          else if (*begin == '\\') {despecialization = true;}
          else                     {qname.push_back(*begin);}
        }
        ++begin;
        ++column;
      }
      tokens.emplace_back(token {tk::qname, qname, line, column});
    }
    else // All other tokens.
    {
      tokens.emplace_back(token {tk(ty), match[ty].str(), line, column});
    }
  }

  return tokens;
}

/*------------------------------------------------------------------------------------------------*/

boost::optional<std::string>
accept_name(parse_cxt& cxt)
{
  if (accept(cxt, tk::number)) // identifiers may start with a number
  {
    const auto name = cxt.val();
    accept(cxt, tk::name);
    return name + cxt.val();
  }
  else if (accept(cxt, tk::name))
  {
    return cxt.val();
  }
  return {};
}

/*------------------------------------------------------------------------------------------------*/

boost::optional<std::string>
accept_id(parse_cxt& cxt)
{
  if      (const auto maybe_name = accept_name(cxt)) {return *maybe_name;}
  else if (accept(cxt, tk::qname))                   {return cxt.val();}
  else                                               {return {};}
}

/*------------------------------------------------------------------------------------------------*/

std::string
id(parse_cxt& cxt)
{
  if (const auto maybe_name = accept_name(cxt))
  {
    return *maybe_name;
  }
  else
  {
    expect(cxt, tk::qname);
    return cxt.val();
  }
}

/*------------------------------------------------------------------------------------------------*/

bool
accept_anchor(parse_cxt& cxt)
{
  static const std::string anchors[] = {"n", "nw", "w", "sw", "s", "se", "e", "ne", "c"};
  return accept(cxt, tk::name)
       ? std::any_of(begin(anchors), end(anchors), [&](const auto& a){return a == cxt.val();})
       : false;
}

/*------------------------------------------------------------------------------------------------*/

void
anchor(parse_cxt& cxt)
{
  if (not accept_anchor(cxt)) {error(cxt);}
}

/*------------------------------------------------------------------------------------------------*/

bool
accept_label(parse_cxt& cxt)
{
  return accept_name(cxt) or accept(cxt, tk::qname);
}

/*------------------------------------------------------------------------------------------------*/

void
label(parse_cxt& cxt)
{
  if (not accept_label(cxt)) {error(cxt);}
}

/*------------------------------------------------------------------------------------------------*/

pn::valuation_type
weight(parse_cxt& cxt)
{
  expect(cxt, tk::number);
  const auto res = std::stoi(cxt.val());
  switch (cxt.val().back())
  {
    case 'K': return res * 1'000;
    case 'M': return res * 1'000'000;
    default : return res;
  }
}

/*------------------------------------------------------------------------------------------------*/

pn::clock_type
eft(parse_cxt& cxt)
{
  const bool neg = accept(cxt, tk::minus);
  expect(cxt, tk::number);
  return std::stoi(cxt.val()) - (neg ? 1 : 0);
}

/*------------------------------------------------------------------------------------------------*/

pn::clock_type
lft(parse_cxt& cxt)
{
  const bool neg = accept(cxt, tk::minus);
  if (neg)
  {
    expect(cxt, tk::number);
    return std::stoi(cxt.val()) - 1;
  }
  else if (accept(cxt, tk::number))
  {
    return std::stoi(cxt.val());
  }
  else
  {
    expect(cxt, tk::name);
    if   (cxt.val() == "w") {return pn::infinity;}
    else                    {error(cxt); __builtin_unreachable();}
  }
}

/*------------------------------------------------------------------------------------------------*/

boost::optional<pn::arc::type>
accept_arc_kind(parse_cxt& cxt)
{
  if (accept(cxt, tk::question))
  {
    if   (accept(cxt, tk::minus)) {return pn::arc::type::inhibitor;}
    else                          {return pn::arc::type::read;}
  }
  else if (accept(cxt, tk::exclamation))
  {
    throw parse_error("Unsupported stopwatch arc");
  }
  else
  {
    return {};
  }
}

/*------------------------------------------------------------------------------------------------*/

void
ang(parse_cxt& cxt)
{
  expect(cxt, tk::float_);
  if (std::stof(cxt.val()) > 1) {error(cxt);}
}

/*------------------------------------------------------------------------------------------------*/

void
edge(parse_cxt& cxt, pn::net& n)
{
  const auto src = id(cxt);
  const auto dst = [&]
  {
    auto res = std::string{};
    if (accept_id(cxt))
    {
      res = cxt.val();
    }
    else
    {
      ang(cxt);
      expect(cxt, tk::float_);
      res = id(cxt);
      ang(cxt);
      expect(cxt, tk::float_);
    }
    return res;
  }();
  const auto kind = [&]
  {
    if   (const auto k = accept_arc_kind(cxt)) {return *k;}
    else                                       {return pn::arc::type::normal;}
  }();
  const auto valuation = weight(cxt);
  anchor(cxt);

  if (n.places().count(src))
  {
    if (not n.transitions().count(dst))
    {
      throw parse_error("Transition "s + dst + " not found"s);
    }
    n.add_pre_place(dst, src, valuation, kind);
  }
  else
  {
    if (not n.transitions().count(src))
    {
      throw parse_error("Transition "s + src + " not found"s);
    }
    if (not n.places().count(dst))
    {
      if (n.transitions().count(dst))
      {
        throw parse_error("Priorities unsupported"s);
      }
      else
      {
        throw parse_error("Place "s + dst + " not found"s);
      }
    }
    n.add_post_place(src, dst, valuation, kind);
  }
}

/*------------------------------------------------------------------------------------------------*/

void
transition(parse_cxt& cxt, pn::net& n)
{
  expect(cxt, tk::float_);
  expect(cxt, tk::float_);
  const auto name = id(cxt);
  pn::clock_type lower;
  pn::clock_type higher;
  if (accept_anchor(cxt))
  {
    lower = eft(cxt);
    higher = lft(cxt);
    anchor(cxt);
    label(cxt);
    anchor(cxt);
  }
  else
  {
    lower = eft(cxt);
    higher = lft(cxt);
    anchor(cxt);
  }
  n.add_transition(name);
  n.add_time_interval(name, lower, higher);
}

/*------------------------------------------------------------------------------------------------*/

void
place(parse_cxt& cxt, pn::net& n)
{
  expect(cxt, tk::float_);
  expect(cxt, tk::float_);
  const auto name = id(cxt);
  const auto marking = weight(cxt);
  anchor(cxt);
  if (accept_label(cxt))
  {
    anchor(cxt);
  }
  n.add_place(name, marking);
}

/*------------------------------------------------------------------------------------------------*/

void
net(parse_cxt& cxt, pn::net& n)
{
  n.name = id(cxt);
  if (accept(cxt, tk::name))
  {
    accept(cxt, tk::name);
  }
}

/*------------------------------------------------------------------------------------------------*/

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
ndr(std::istream& in)
{
  const std::string text{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
  auto net_ptr = std::make_shared<pn::net>();

  const auto tks = tokens(begin(text), end(text));
  parse_cxt cxt{tks.cbegin(), tks.cend()};
  bool edge_seen = false;

  while (not cxt.eof())
  {
    if (accept(cxt, tk::newline)) break;

    expect(cxt, tk::name);
    if (cxt.val().size() > 1) {error(cxt);}

    switch (cxt.val()[0])
    {
      case 'p':
        if (edge_seen) error(cxt);
        place(cxt, *net_ptr);
        break;

      case 't':
        if (edge_seen) error(cxt);
        transition(cxt, *net_ptr);
        break;

      case 'e':
        edge_seen = true;
        edge(cxt, *net_ptr);
        break;

      case 'h':
        net(cxt, *net_ptr);
        break;

      default:
        error(cxt);
    }
    expect(cxt, tk::newline);
  };
  net_ptr->format = conf::pn_format::ndr;
  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
