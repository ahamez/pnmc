/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

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
