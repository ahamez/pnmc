/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <algorithm> // search
#include <array>
#include <cassert>
#include <deque>
#include <iostream>
#include <limits>
#include <regex>
#include <sstream>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include <boost/algorithm/string/join.hpp>
#include <boost/optional.hpp>

#include "support/parsers/parse_error.hh"
#include "support/parsers/net.hh"
#include "support/parsers/token.hh"

namespace pnmc { namespace parsers {

namespace {

/*------------------------------------------------------------------------------------------------*/

// @brief Used to start keywords in token_t at indexes higher than other regular tokens matched by 
// the regular expression.
static constexpr auto keywords_gap = 1000u;

// @brief Start at 1 because the relative position in the enum is used to compute the type of the 
// token and because submatches of regular expressions start at 1 (0 is the whole match).
enum class tk { skip = 1u, newline, number, qname, name, colon, comma
              , lbracket, rbracket, arrow, lparen, rparen, question, exclamation, minus, mult
              , lt, gt
              , net = keywords_gap, transition, place, prio, module};

/*------------------------------------------------------------------------------------------------*/

using token = token_holder<tk>;
using parse_cxt = parse_context<tk>;

/*------------------------------------------------------------------------------------------------*/

/// @brief Returns the tokens of a Tina model.
template <typename InputIterator>
auto
tokens(InputIterator&& begin, InputIterator&& end)
{
  static constexpr std::array<const char*, 5> keywords = {{"net", "tr", "pl", "pr", "md"}};
  // Order must match enum token_t
  static const std::regex regex
  {
    "([ \\t]+)|"                  // skip spaces and tabs
    "(\\r\\n|\\n|\\r)|"           // newline (to keep track of line numbers)
    "(\\d+[KM]?)|"                // numbers can have a suffix
    "(\\{)|"                      // only search for opening brace for qualified names
    "([a-zA-Z'_][a-zA-Z0-9'_]*)|" // don't begin by a number, we add it if necessary in the parser
    "(:)|"                        // colon
    "(,)|"                        // comma
    "(\\[)|"                      // left bracket
    "(\\])|"                      // right bracket
    "(->)|"                       // arrow
    "(\\()|"                      // left parenthesis
    "(\\))|"                      // right parenthesis
    "(\\?)|"                      // question mark
    "(!)|"                        // exclamation mark
    "(-)|"                        // minus
    "(\\*)|"                      // multiplication
    "(<)|"                        // less than
    "(>)"                         // upper than
  };

  // To report error location, if any.
  auto line = 1ul;
  auto column = 1ul;
  std::smatch match;
  std::deque<token> tokens;

  while (true)
  {
    if (begin == end) {break;}

    std::regex_search(begin, end, match, regex);

    if (match.position() != 0)
    {
      throw parse_error( "Unexpected '" + match.prefix().str() + "' at " + std::to_string(line)
                       + ':' + std::to_string(column - match.prefix().str().size()));
    }

    column += match.length();
    begin += match.length();

    if (match[pos(tk::skip)].matched)    {continue;}
    if (match[pos(tk::newline)].matched) {column = 1; ++line; continue;}

    // Get the type of token by looking for the first successful submatch.
    const auto ty = [&match]
    {
      auto t = pos(tk::number);
      for (; t < match.size(); ++t)
      {
        if (match[t].matched) {break;}
      }
      return t;
    }();
    assert(ty < match.size());

    // Check if name is a keyword.
    if (ty == pos(tk::name))
    {
      const auto search = std::find(keywords.cbegin(), keywords.cend(), match[ty].str());
      if (search != keywords.cend())
      {
        const auto kw_ty = std::distance(keywords.cbegin(), search) + keywords_gap;
        tokens.emplace_back(token {tk(kw_ty), "", line, column});
        continue;
      }
    }
    // Process qualified names manually (to handle despecialization).
    else if (ty == pos(tk::qname))
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
      continue;
    }
    // All other tokens.
    tokens.emplace_back(token {tk(ty), match[ty].str(), line, column});
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

boost::optional<std::string>
accept_id(parse_cxt& cxt)
{
  if      (const auto n = accept_name(cxt)) {return *n;}
  else if (accept(cxt, tk::qname))          {return cxt.val();}
  else                                      {return {};}
}

/*------------------------------------------------------------------------------------------------*/

unsigned int
number(parse_cxt& cxt)
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

template <typename Fun>
void
input_arcs(parse_cxt& cxt, Fun&& add_arc)
{
  while (true)
  {
    const auto maybe_target = accept_id(cxt);
    if (not maybe_target) {break;} // no more arcs

    auto arc_ty = pn::arc::type::normal;
    auto valuation = 1u;

    if (accept(cxt, tk::mult))
    {
      if (accept(cxt, tk::mult))
      {
        arc_ty = pn::arc::type::reset;
      }
      else
      {
        valuation = number(cxt);
      }
    }
    else if (accept(cxt, tk::question))
    {
      if   (accept(cxt, tk::minus)) {arc_ty = pn::arc::type::inhibitor;}
      else                          {arc_ty = pn::arc::type::read;}
      valuation = number(cxt);
    }
    else if (accept(cxt, tk::exclamation))
    {
      throw parse_error("Unsupported stopwatch inhibitor arc");
    }
    add_arc(*maybe_target, valuation, arc_ty);
  }
}

/*------------------------------------------------------------------------------------------------*/

template <typename Fun>
void
output_arcs(parse_cxt& cxt, Fun&& add_arc)
{
  while (true)
  {
    const auto maybe_target = accept_id(cxt);
    if (not maybe_target)
    {
      break; // no more arcs
    }
    auto valuation = 1u;
    if (accept(cxt, tk::mult))
    {
      valuation = number(cxt);
    }
    add_arc(*maybe_target, valuation, pn::arc::type::normal);
  }
}

/*------------------------------------------------------------------------------------------------*/

void
transition(parse_cxt& cxt, pn::net& n)
{
  const auto tid = id(cxt);
  n.add_transition(tid);

  if (accept(cxt, tk::colon)) // label
  {
    id(cxt);
  }

  if (accept(cxt, tk::lbracket) or accept(cxt, tk::rbracket)) // time interval
  {
    static constexpr auto inf = std::numeric_limits<unsigned int>::max();

    // To report error location.
    const auto line = cxt.previous().line;
    const auto column = cxt.previous().column - 1;

    const bool open_lower_endpoint = cxt.val() == "]";
    bool open_upper_endpoint = false;

    auto low = number(cxt);

    expect(cxt, tk::comma);

    auto high = 0u;
    if (accept(cxt, tk::name))
    {
      if (cxt.val() != "w") {error(cxt);}
      high = inf;
    }
    else
    {
      high = number(cxt);
    }

    if (not accept(cxt, tk::rbracket))
    {
      expect(cxt, tk::lbracket);
      open_upper_endpoint = cxt.val() == "[";
    }

    if (open_lower_endpoint)                 {low += 1;}
    if (open_upper_endpoint and high != inf) {high -= 1;}

    if (high < low)
    {
      throw parse_error( "Time interval at " + std::to_string(line) + ":" + std::to_string(column));
    }

    n.add_time_interval(tid, low, high);
  }

  input_arcs(cxt, [&](const auto& pre, unsigned int weight, pn::arc::type ty)
                     {
                       n.add_pre_place(tid, pre, weight, ty);
                     });
  if (accept(cxt, tk::arrow))
  {
    output_arcs(cxt, [&](const auto& post, unsigned int weight, pn::arc::type ty)
                        {
                          n.add_post_place(tid, post, weight, ty);
                        });
  }
}

/*------------------------------------------------------------------------------------------------*/

void
place(parse_cxt& cxt, pn::net& n)
{
  const auto name = id(cxt);
  auto marking = 0u;
  if (accept(cxt, tk::colon)) // label
  {
    id(cxt);
  }
  if (accept(cxt, tk::lparen)) // initial marking
  {
    marking = number(cxt);
    expect(cxt, tk::rparen);
  }
  n.add_place(name, marking);
}

/*------------------------------------------------------------------------------------------------*/

void
priority(parse_cxt& cxt, pn::net&)
{
  id(cxt);
  while (accept_id(cxt)) {}
  if (not accept(cxt, tk::lt))
  {
    expect(cxt, tk::gt);
  }
  id(cxt);
  while (accept_id(cxt)) {}
}

/*------------------------------------------------------------------------------------------------*/

void
module(parse_cxt& cxt, std::unordered_map<std::string, std::vector<std::string>>& modules_id)
{
  const auto insertion = modules_id.emplace(id(cxt), std::vector<std::string>{});
  while (const auto pl = accept_id(cxt))
  {
    insertion.first->second.emplace_back(*pl);
  }
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Remove commented lines and uncomment '#!' pragmas.
std::string
preprocess(std::istream& in)
{
  std::stringstream ss;
  auto comment = false;
  auto new_line = true;
  while (true)
  {
    const char c = in.get();
    if (in.eof())
    {
      break;
    }
    if (comment)
    {
      if (c == '!')
      {
        comment = false; // a comment is terminated by a line
        new_line = false;
        ss << ' '; // remove pragma ! indicator, the parser will directly read the associated token
      }
      else if (c == '\n' or c == '\r')
      {
        comment = false; // a comment is terminated by a line
        ss << c;
        if (c == '\r' and in.peek() == '\n')
        {
          ss << static_cast<char>(in.get());
          new_line = true;
        }
      }
      // else Comments are not copied
    }
    else if (not comment and c == '#')
    {
      comment = true;
      ss << ' ';
      new_line = false;
    }
    else if (new_line and not comment and c == 'n' and in.peek() == 't')
    // process notes as comments
    {
      comment = true;
    }
    else if (c == '\n')
    {
      ss << '\n';
      new_line = true;
    }
    else if (not comment)
    {
      ss << c;
      new_line = false;
    }
  }
  return ss.str();
}

/*------------------------------------------------------------------------------------------------*/

pn::module
make_module( const std::string& id, std::unordered_map<std::string, std::vector<std::string>>& graph
           , pn::net& net, const std::string& container, std::unordered_set<std::string>& places)
{
  const auto place_search = net.places().find(id);
  if (place_search != end(net.places()))
  {
    places.insert(place_search->name);
    return pn::make_module(*place_search);
  }
  else
  {
    pn::module_node m{id};
    const auto submodule_search = graph.find(id);
    if (submodule_search == end(graph))
    {
      throw parse_error("Undefined module or place " + id + " in module " + container);
    }
    for (const auto& sub : submodule_search->second)
    {
      m.add_submodule(make_module(sub, graph, net, id, places));
    }
    const auto insertion = net.modules.emplace(id, pn::make_module(m));
    if (not insertion.second)
    {
      throw parse_error("Module or place " + id + " is present in another module");
    }
    return insertion.first->second;
  }
}

/*------------------------------------------------------------------------------------------------*/

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
net(std::istream& in)
{
  auto net_ptr = std::make_shared<pn::net>();

  const auto text = preprocess(in);
  const auto tks = tokens(begin(text), end(text));
  auto cxt = parse_cxt{tks.cbegin(), tks.cend()};

  auto modules_id = std::unordered_map<std::string, std::vector<std::string>>{};

  while (not cxt.eof())
  {
    if      (accept(cxt, tk::net))        {net_ptr->name = id(cxt);}
    else if (accept(cxt, tk::transition)) {transition(cxt, *net_ptr);}
    else if (accept(cxt, tk::place))      {place(cxt, *net_ptr);}
    else if (accept(cxt, tk::prio))       {priority(cxt, *net_ptr);}
    else if (accept(cxt, tk::module))     {module(cxt, modules_id);}
    else                                  {error(cxt);}
  };

  if (modules_id.size() > 0)
  {
    const auto& net_name = net_ptr->name;
    const auto& root_search = modules_id.find(net_name);
    auto places_encountered = std::unordered_set<std::string>{};

    if (root_search == end(modules_id))
    {
      throw parse_error("No root module(s).");
    }
    for (const auto& submodule : root_search->second)
    {
      net_ptr->root_modules.emplace_back(make_module( submodule, modules_id, *net_ptr, net_name
                                                    , places_encountered));
    }
    auto invalid = std::vector<std::string>{};
    std::for_each( begin(net_ptr->places()), end(net_ptr->places())
                 , [&](const auto& p)
                      {
                        if (not places_encountered.count(p.name)) {invalid.push_back(p.name);}
                      });
    std::for_each( begin(modules_id), end(modules_id)
                 , [&](const auto& kv)
                      {
                        if (kv.first != net_ptr->name and not net_ptr->modules.count(kv.first))
                        {
                          invalid.push_back(kv.first);
                        }
                      });
    if (not invalid.empty())
    {
      const auto msg = boost::algorithm::join(invalid, ", ");
      throw parse_error("The following modules or places do not belong to any module: " + msg);
    }
  }

  net_ptr->format = conf::pn_format::net;
  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
