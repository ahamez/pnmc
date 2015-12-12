/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <deque>
#include <string>

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

/// @brief Describes a token (its type and its string value).
template <typename Ty>
struct token_holder
{
  Ty ty;
  std::string val;
  std::size_t line;
  std::size_t column;
};

/*------------------------------------------------------------------------------------------------*/

template <typename Ty>
struct parse_context
{
  typename std::deque<token_holder<Ty>>::const_iterator curr;
  typename std::deque<token_holder<Ty>>::const_iterator end;
  bool             eof()      const noexcept {return curr == end;}
  std::string      val()      const noexcept {return (curr - 1)->val;}
  token_holder<Ty> previous() const noexcept {return *(curr - 1);}
  token_holder<Ty> current()  const noexcept {return *curr;}
  void             advance()  noexcept       {++curr;}
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Gets the position of tk in the enum class token_t.
template <typename Ty>
std::size_t
pos(Ty tk)
{
  return static_cast<std::underlying_type_t<Ty>>(tk);
}

/*------------------------------------------------------------------------------------------------*/

template <typename Cxt, typename Ty>
bool
accept(Cxt& cxt, Ty ty)
noexcept
{
  return cxt.curr->ty == ty and (cxt.advance(), true);
}

/*------------------------------------------------------------------------------------------------*/

template <typename Cxt>
void
error(Cxt& cxt)
{
  const auto tk = cxt.current();
  throw parse_error( "Unexpected '" + tk.val + "' at " + std::to_string(tk.line) + ":"
                   + std::to_string(tk.column - tk.val.size()));
}

/*------------------------------------------------------------------------------------------------*/

template <typename Cxt, typename Ty>
bool
expect(Cxt& cxt, Ty ty)
{
  if   (accept(cxt, ty)) {return true;}
  else                   {error(cxt); __builtin_unreachable();}
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
