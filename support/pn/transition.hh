#pragma once

#include <iosfwd>
#include <map>
#include <string>

#include "support/pn/arc.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

/// @brief A transition in a Petri Net.
struct transition
{
  using arcs_type = std::map<std::string, arc>;

  /// @brief This transition's id.
  const std::string id;

  /// @brief This transition's index.
  ///
  /// Used to compute dead transitions: this index is used to get the position of this transition
  /// into a bitset describing all transitions' status.
  const std::size_t index;

  /// @brief Pre-places
  arcs_type pre;

  /// @brief Post-places
  arcs_type post;

  /// @brief Lower (closed) time interval.
  clock_type low;

  /// @brief Upper (closed) time interval.
  clock_type high;

  /// @brief Constructor.
  transition(const std::string& id, std::size_t index);

  /// @brief Tell if the transition has a time interval.
  bool timed() const;
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Compare two transitions using their ids.
bool
operator<(const transition&, const transition&) noexcept;

/*------------------------------------------------------------------------------------------------*/

/// @brief Export a transition to an output stream.
std::ostream&
operator<<(std::ostream&, const transition&);

/*------------------------------------------------------------------------------------------------*/
  
}} // namespace pnmc::pn
