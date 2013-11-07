#ifndef _PNMC_PN_NET_HH_
#define _PNMC_PN_NET_HH_

#pragma GCC diagnostic push
#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#pragma GCC diagnostic pop

#include "pn/module.hh"
#include "pn/place.hh"
#include "pn/transition.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

using namespace boost::multi_index;

/*------------------------------------------------------------------------------------------------*/

struct net
{
private:

  /// @brief A tag to identify the view ordered by insertion order for boost::multi_index.
  struct insertion_index{};

  /// @brief A tag to identify the view ordered by idendifiers for boost::multi_index.
  struct id_index{};

  /// @brief A tag to identify the view ordered by markings for boost::multi_index.
  struct marking_index{};

  /// @brief A tag to identify the view ordered by indices for boost::multi_index.
  struct index_index{};

public:

  /// @brief The type of the set of all places.
  typedef multi_index_container<
                place
  						, indexed_by<

                  // keep insertion order
                    sequenced<tag<insertion_index>>

                  // sort by id
                  ,  ordered_unique< tag<id_index>
                                   , member<place, const std::string, &place::id>>

                  // sort by marking
                  , ordered_non_unique< tag<marking_index>
                                      , member<place, unsigned int, &place::marking>>
              >
          > places_type;

  /// @brief The type of the set of all transitions.
  typedef multi_index_container<
                transition
              , indexed_by<

                   // sort by id
                     ordered_unique< tag<id_index>
                                   , member<transition, const std::string, &transition::id>>

                   // sort by index
                   , ordered_unique< tag<index_index>
                                   , member<transition, const std::size_t, &transition::index>>
              >
         > transitions_type;

  /// @brief The net's name.
  std::string name;

  /// @brief The set of places.
  places_type places_set;

  /// @brief The set of transitions.
  transitions_type transitions_set;

  /// @brief The hierarchical description, if any, of this Petri net.
  module modules;

  /// @brief Default constructor.
  net();

  /// @brief Add a place.
  ///
  /// If the place already exist, its marking is updated.
  const place&
  add_place(const std::string& id, unsigned int marking);

  /// @brief Add a transition.
  ///
  /// If the transition already exists, no operation is done.
  const transition&
  add_transition(const std::string& tid);

  /// @brief Add a post place to a transition.
  ///
  /// If the place doesn't exist, it is created with a marking set to 0.
  void
  add_post_place(const std::string& tid, const std::string& post, unsigned int marking);

  /// @brief Add a pre place to a transition.
  ///
  /// If the place doesn't exist, it is created with a marking set to 0.
  void
  add_pre_place(const std::string& tid, const std::string& pre, unsigned int marking);

  /// @brief Return all places by insertion order.
  const places_type::index<insertion_index>::type&
  places() const noexcept;

  /// @brief Return all places by identifier.
  const places_type::index<id_index>::type&
  places_by_id() const noexcept;

  /// @brief Return all transitions.
  const transitions_type::index<id_index>::type&
  transitions() const noexcept;

  /// @brief Get a transition using its index.
  const transition&
  get_transition_by_index(std::size_t index) const;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn

#endif // _PNMC_PN_NET_HH_
