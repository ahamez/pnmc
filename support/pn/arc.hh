#pragma once

#include "support/pn/types.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

/// @brief An arc in a Petri Net.
struct arc
{
  /// @brief The possible types of an arc.
  enum class type {inhibitor, normal, read, reset, stopwatch, stopwatch_inhibitor};

  /// @brief The valuation of this arc.
  valuation_type weight;

  /// @brief The type of this arc.
  type kind;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
