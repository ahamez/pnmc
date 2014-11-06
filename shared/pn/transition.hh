#pragma once

#include <iosfwd>
#include <limits>
#include <map>
#include <string>

#include "shared/pn/arc.hh"

namespace pnmc { namespace pn {

/// @brief Time infinity.
constexpr unsigned int inf = std::numeric_limits<unsigned int>::max();

/// @brief Time sharp.
constexpr unsigned int sharp = inf - 1;

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

  /// @brief Lower time interval.
  unsigned int low;

  /// @brief Upper time interval.
  unsigned int high;

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
