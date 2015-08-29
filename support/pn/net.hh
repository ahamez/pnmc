/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <unordered_map>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/sequenced_index.hpp>

#include "support/conf/pn_format.hh"
#include "support/pn/arc.hh"
#include "support/pn/module.hh"
#include "support/pn/place.hh"
#include "support/pn/transition.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

using namespace boost::multi_index;

/*------------------------------------------------------------------------------------------------*/

class net
{
private:

  /// @brief A tag to identify the view ordered by insertion order for boost::multi_index.
  struct insertion_index{};

  /// @brief A tag to identify the view ordered by unique identifiers for boost::multi_index.
  struct uid_index{};

  /// @brief A tag to identify the view ordered by names for boost::multi_index.
  struct name_index{};

  /// @brief A tag to identify the view ordered by markings for boost::multi_index.
  struct marking_index{};

public:

  /// @brief The type of the set of all places.
  using places_type =
    multi_index_container<
      place
    , indexed_by< // keep insertion order
                  sequenced<tag<insertion_index>>

                   // sort by unique idenfier
                ,  ordered_unique< tag<uid_index>
                                 , member<place, decltype(place::uid), &place::uid>>

                  // sort by name
                ,  ordered_unique< tag<name_index>
                                 , member<place, decltype(place::name), &place::name>>

                  // sort by marking
                , ordered_non_unique< tag<marking_index>
                                    , member<place, unsigned int, &place::marking>>
    >>;

  /// @brief The type of the set of all transitions.
  using transitions_type =
    multi_index_container<
      transition
    , indexed_by< // sort by name
                  ordered_unique< tag<name_index>
                                , member<transition, decltype(transition::name), &transition::name>>

                  // sort by unique identifier
                , ordered_unique< tag<uid_index>
                                , member<transition, decltype(transition::uid), &transition::uid>>
    >>;

  /// @brief The Petri net's name.
  std::string name;

  /// @brief The hierarchical description.
  std::unordered_map<std::string, module> modules;
  std::vector<module> root_modules;

  /// @brief The original format of this net.
  conf::pn_format format;

  /// @brief Default constructor.
  net();

  /// @brief Add a place.
  ///
  /// If the place already exist, its marking is updated.
  const place&
  add_place(const std::string&, valuation_type);

  /// @brief Add a transition.
  ///
  /// If the transition already exists, no operation is done.
  const transition&
  add_transition(const std::string&);

  /// @brief Add a post place to a transition.
  ///
  /// If the place doesn't exist, it is created with a marking set to 0.
  void
  add_post_place( const std::string& tid, const std::string& post, valuation_type weight
                , arc::type ty = arc::type::normal);

  /// @brief Add a pre place to a transition.
  ///
  /// If the place doesn't exist, it is created with a marking set to 0.
  void
  add_pre_place( const std::string& tname, const std::string& pre, valuation_type weight
               , arc::type ty = arc::type::normal);

  /// @brief Return all places by insertion order.
  const places_type::index<insertion_index>::type&
  places_by_insertion() const noexcept;

  /// @brief Return all places by name.
  const places_type::index<name_index>::type&
  places() const noexcept;

  /// @brief Return all transitions by name.
  const transitions_type::index<name_index>::type&
  transitions() const noexcept;

  /// @brief Get a transition using its unique identifier.
  const transition&
  get_transition_by_uid(std::size_t) const;

  /// @brief Add a time interval to a transition.
  void
  add_time_interval(const std::string& tname, clock_type low, clock_type high);

  /// @brief Tell if some transitions are timed.
  bool
  timed() const noexcept;

  /// @brief Tell if a transition is initially enabled.
  bool enabled(const std::string& tid) const;

private:

  template <typename Index>
  const typename places_type::index<Index>::type&
  places_by()
  const noexcept
  {
    return m_places.template get<Index>();
  }

  template <typename Index>
  typename places_type::index<Index>::type&
  places_by()
  noexcept
  {
    return m_places.template get<Index>();
  }

  template <typename Index>
  const typename transitions_type::index<Index>::type&
  transitions_by()
  const noexcept
  {
    return m_transitions.template get<Index>();
  }

  template <typename Index>
  typename transitions_type::index<Index>::type&
  transitions_by()
  noexcept
  {
    return m_transitions.template get<Index>();
  }

private:

  /// @brief The set of places.
  places_type m_places;

  /// @brief The set of transitions.
  transitions_type m_transitions;

  /// @brief Used to identify places with an unique identifier.
  std::size_t m_current_place_uid;

  /// @brief Used to identify transitions with an unique identifier.
  std::size_t m_current_transition_uid;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
