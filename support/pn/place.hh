/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <iosfwd>
#include <map>
#include <string>

#include "support/pn/arc.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

struct place
{
  using arcs_type = std::map<std::string, arc>;

  /// @brief This place's unique identifier.
  std::size_t uid;

  /// @brief This place's name.
  const std::string name;

  /// @brief This place's marking.
  unsigned int marking;

  /// @brief Pre transitions.
  arcs_type pre;

  /// @brief Post transitions.
  arcs_type post;

  /// @brief Constructor.
  place(std::size_t, std::string , valuation_type);

  /// @brief Tell if this place is connected to one or more transitions.
  bool connected() const noexcept;
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Compare two places using their ids.
bool
operator<(const place& left, const place& right) noexcept;

/*------------------------------------------------------------------------------------------------*/

/// @brief Export a place to an output stream.
std::ostream&
operator<<(std::ostream& os, const place& p);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
