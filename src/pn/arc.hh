#pragma once

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

/// @brief An arc in a Petri Net.
struct arc
{
  /// @brief The possible types of an arc.
  enum class type {normal, read, inhibitor, stopwatch, stopwatch_inhibitor};

  /// @brief The valuation of this arc.
  const unsigned int weight;

  /// @brief The type of this arc.
  const type kind;

  /// @brief Constructor.
  arc(unsigned int w, type t)
    : weight(w), kind(t)
  {}
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
