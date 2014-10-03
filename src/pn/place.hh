#pragma once

#include <iosfwd>
#include <map>
#include <string>

#include "pn/arc.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

struct place
{
  using arcs_type = std::multimap<std::string, arc>;

  /// @brief This place's id.
  const std::string id;

  /// @brief This place's marking.
  unsigned int marking;

  /// @brief Pre transitions.
  arcs_type pre;

  /// @brief Post transitions.
  arcs_type post;

  /// @brief Constructor.
  place(const std::string& id, unsigned int m);

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
