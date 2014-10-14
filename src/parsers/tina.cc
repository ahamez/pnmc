#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <iostream>
#include <limits>
#include <regex>
#include <string>
#include <type_traits>

#include <boost/optional.hpp>

#include "parsers/parse_error.hh"
#include "parsers/tina.hh"

namespace pnmc { namespace parsers {

namespace {

/*------------------------------------------------------------------------------------------------*/

// @brief Used to start keywords in token_t at indexes higher than other regular tokens matched by 
// the regular expression.
static constexpr auto keywords_gap = 1000u;

// @brief Start at 1 because the relative position in the enum is used to compute the type of the 
// token and because submatches of regular expressions start at 1 (0 is the whole match).
enum class token_t { skip = 1u, newline, number, qname, name, colon, comma
                   , lbracket, rbracket, arrow, lparen, rparen, question, exclamation, minus, mult
                   , comment
                   , net = keywords_gap, transition, place};

/*------------------------------------------------------------------------------------------------*/

/// @brief Gets the position of tk in the enum class token_t.
std::size_t
pos(token_t tk)
{
  return static_cast<std::underlying_type_t<token_t>>(tk);
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Describes a token (its type and its string value).
struct token
{
  token_t ty;
  std::string val;
  std::size_t line;
  std::size_t column;

  std::string
  str()
  const
  {
    return "'" + val + "' at " + std::to_string(line) + ":" + std::to_string(column);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Returns the tokens of a Tina model.
template <typename InputIterator>
auto
tokens(InputIterator&& begin, InputIterator&& end)
{
  static constexpr std::array<const char*, 3> keywords = {{"net", "tr", "pl"}};
  // Order must match enum token_t
  static const std::regex regex
  ( 
    "([ \\t]+)|"                  // skip spaces and tabs
    "(\\n)|"                      // newline (to keep track of line numbers)
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
    "(^#[^\\n]*)"                 // comments start at the beginning of a line
  );

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

    if (match[pos(token_t::skip)].matched)
    {
      continue;
    }

    if (match[pos(token_t::newline)].matched)
    {
      last_match_is_newline = true;
      column = 1;
      line += 1;
      continue;
    }

    if (match[pos(token_t::comment)].matched)
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
      auto t = pos(token_t::number);
      for (; t < match.size(); ++t)
      {
        if (match[t].matched) {break;}
      }
      return t;
    }();
    assert(ty < match.size());

    // Check if name is a keyword.
    if (ty == pos(token_t::name))
    {
      const auto search = std::find(keywords.cbegin(), keywords.cend(), match[ty].str());
      if (search != keywords.cend())
      {
        const auto kw_ty = std::distance(keywords.cbegin(), search) + keywords_gap;
        tokens.emplace_back(token {token_t(kw_ty), "", line, column});
        continue;
      }
    }
    // Process qualified names manually (to handle despecialization).
    else if (ty == pos(token_t::qname))
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
      tokens.emplace_back(token {token_t::qname, qname, line, column});
      continue;
    }
    // All other tokens.
    tokens.emplace_back(token {token_t(ty), match[ty].str(), line, column});
  }

  return tokens;
}

/*------------------------------------------------------------------------------------------------*/

struct parse_cxt
{
  std::deque<token>::const_iterator curr;
  std::deque<token>::const_iterator end;
  auto eof()      const noexcept {return curr == end;}
  auto val()      const noexcept {return (curr - 1)->val;}
  auto previous() const noexcept {return *(curr - 1);}
  auto current()  const noexcept {return *curr;}
  void advance()  noexcept       {++curr;}
};

/*------------------------------------------------------------------------------------------------*/

bool
accept(parse_cxt& cxt, token_t ty)
noexcept
{
  return cxt.curr->ty == ty ? cxt.advance(), true : false;
}

/*------------------------------------------------------------------------------------------------*/

void
error(parse_cxt& cxt)
{
  throw parse_error("Unexpected " + cxt.current().str());
}

/*------------------------------------------------------------------------------------------------*/

bool
expect(parse_cxt& cxt, token_t ty)
{
  if   (accept(cxt, ty)) {return true;}
  else                   {error(cxt); __builtin_unreachable();}
}

/*------------------------------------------------------------------------------------------------*/

boost::optional<std::string>
accept_name(parse_cxt& cxt)
{
  if (accept(cxt, token_t::number)) // identifiers may start with a number
  {
    const auto name = cxt.val();
    accept(cxt, token_t::name);
    return name + cxt.val();
  }
  else if (accept(cxt, token_t::name))
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
    expect(cxt, token_t::qname);
    return cxt.val();
  }
}

/*------------------------------------------------------------------------------------------------*/

unsigned int
number(parse_cxt& cxt)
{
  expect(cxt, token_t::number);
  const auto res = std::stoi(cxt.val());
  switch (cxt.val().back())
  {
    case 'K': return res * 1'000;
    case 'M': return res * 1'000'000;
    default : return res;
  }
}

/*------------------------------------------------------------------------------------------------*/

boost::optional<std::string>
target(parse_cxt& cxt)
{
  if      (const auto maybe_name = accept_name(cxt)) {return *maybe_name;}
  else if (accept(cxt, token_t::qname))              {return cxt.val();}
  else                                               {return {};}
}

/*------------------------------------------------------------------------------------------------*/

template <typename Fun>
void
input_arcs(parse_cxt& cxt, Fun&& add_arc)
{
  while (true)
  {
    const auto maybe_target = target(cxt);
    if (not maybe_target) {break;} // no more arcs

    auto arc_ty = pn::arc::type::normal;
    auto valuation = 1u;

    if (accept(cxt, token_t::mult))
    {
      valuation = number(cxt);
    }
    else if (accept(cxt, token_t::question))
    {
      if (accept(cxt, token_t::minus)) {arc_ty = pn::arc::type::inhibitor;}
      else                             {arc_ty = pn::arc::type::read;}
      valuation = number(cxt);
    }
    else if (accept(cxt, token_t::exclamation))
    {
      if (accept(cxt, token_t::minus)) {arc_ty = pn::arc::type::stopwatch_inhibitor;}
      else                             {arc_ty = pn::arc::type::stopwatch;}
      valuation = number(cxt);
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
    const auto maybe_target = target(cxt);
    if (not maybe_target)
    {
      break; // no more arcs
    }
    auto valuation = 1u;
    if (accept(cxt, token_t::mult))
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

  if (accept(cxt, token_t::colon)) // label
  {
    id(cxt);
  }

  if (accept(cxt, token_t::lbracket) or accept(cxt, token_t::rbracket)) // time interval
  {
    static constexpr auto inf = std::numeric_limits<unsigned int>::max();

    // To report error location.
    const auto line = cxt.previous().line;
    const auto column = cxt.previous().column - 1;

    const bool open_lower_endpoint = cxt.val() == "]";
    bool open_upper_endpoint = false;

    auto low = number(cxt);

    expect(cxt, token_t::comma);

    auto high = 0u;
    if (accept(cxt, token_t::name))
    {
      if (cxt.val() != "w") {error(cxt);}
      high = inf;
    }
    else
    {
      high = number(cxt);
    }

    if (not accept(cxt, token_t::rbracket))
    {
      expect(cxt, token_t::lbracket);
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
  if (accept(cxt, token_t::arrow))
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
  if (accept(cxt, token_t::colon)) // label
  {
    id(cxt);
  }
  if (accept(cxt, token_t::lparen)) // initial marking
  {
    marking = number(cxt);
    expect(cxt, token_t::rparen);
  }
  n.add_place(name, marking);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
tina(std::istream& in)
{
  auto net_ptr = std::make_shared<pn::net>();

  const auto tks = [&]
  {
    std::string text{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    return tokens(text.cbegin(), text.cend());
  }();
  parse_cxt cxt {tks.cbegin(), tks.cend()};

  while (not cxt.eof())
  {
    if      (accept(cxt, token_t::net))        {net_ptr->name = id(cxt);}
    else if (accept(cxt, token_t::transition)) {transition(cxt, *net_ptr);}
    else if (accept(cxt, token_t::place))      {place(cxt, *net_ptr);}
    else                                       {error(cxt);}
  };

  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
