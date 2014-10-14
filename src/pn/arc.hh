#pragma once

#include <string>

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

/// @brief An arc in a Petri Net.
struct arc
{
  /// @brief The possible types of an arc.
  enum class type {normal, read, inhibitor, stopwatch, stopwatch_inhibitor};

  std::string target;

  /// @brief The valuation of this arc.
  unsigned int weight;

  /// @brief The type of this arc.
  type kind;

  arc(const std::string& s, unsigned int w, type t)
    : target(s), weight(w), kind(t)
  {}

  arc(const std::string& s)
    : arc(s, 0, type::normal)
  {}

  friend
  bool
  operator<(const arc& lhs, const arc& rhs)
  noexcept
  {
    return lhs.target < rhs.target;
  }

  friend
  bool
  operator==(const arc& lhs, const arc& rhs)
  {
    return lhs.target == rhs.target;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
