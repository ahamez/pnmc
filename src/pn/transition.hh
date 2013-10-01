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

  /// @brief Pre-places
  std::multimap<std::string, arc> pre;

  /// @brief Post-places
  std::multimap<std::string, arc> post;

  /// @brief Constructor.
  transition(const std::string& id, const std::string& label)
    : id(id)
    , label(label)
    , pre()
    , post()
  {
  }

  /// @brief Used by Boost.MultiIndex.
  struct add_post_place
  {
    const arc& new_arc;
    std::string new_place;

    add_post_place(const arc& a, const std::string& id)
  	  : new_arc(a)
      , new_place(id)
    {
    }

    void
    operator()(transition& t)
    const
    {
      t.post.insert(std::make_pair(new_place , new_arc));
    }
  };

  /// @brief Used by Boost.MultiIndex.
  struct add_pre_place
  {
    const arc& new_arc;
    std::string new_place;

    add_pre_place(const arc& a, const std::string& id)
	  	: new_arc(a)
  	  , new_place(id)
    {
    }

    void
    operator()(transition& t)
    const
    {
      t.pre.insert(std::make_pair(new_place , new_arc));
    }
  };
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
