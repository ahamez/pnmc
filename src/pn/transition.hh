#ifndef _PNMC_PN_TRANSITION_HH_
#define _PNMC_PN_TRANSITION_HH_

#include <iosfwd>
#include <limits>
#include <map>

#include "pn/arc.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

/// @brief A transition in a Petri Net.
struct transition
{
  /// @brief This transition's id.
  const std::string id;

  /// @brief This transition's label.
  std::string label;

  /// @brief This transition's index.
  ///
  /// Used to compute dead transitions: this index is used to get the position of this transition
  /// into a bitset describing all transitions' status.
  const std::size_t index;

  /// @brief Pre-places
  std::multimap<std::string, arc> pre;

  /// @brief Post-places
  std::multimap<std::string, arc> post;

  /// @brief Constructor.
  transition(const std::string& id, const std::string& label, std::size_t index);
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

#endif // _PNMC_PN_TRANSITION_HH_
