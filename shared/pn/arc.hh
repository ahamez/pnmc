#pragma once

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

/// @brief An arc in a Petri Net.
struct arc
{
  /// @brief The possible types of an arc.
  enum class type {normal, read, inhibitor, stopwatch, stopwatch_inhibitor, reset};

  /// @brief The valuation of this arc.
  unsigned int weight;

  /// @brief The type of this arc.
  type kind;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn