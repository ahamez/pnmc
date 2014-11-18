#pragma once

#include <unordered_map>

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

struct net
{
private:

  /// @brief A tag to identify the view ordered by insertion order for boost::multi_index.
  struct insertion_index{};

  /// @brief A tag to identify the view ordered by identifiers for boost::multi_index.
  struct id_index{};

  /// @brief A tag to identify the view ordered by markings for boost::multi_index.
  struct marking_index{};

  /// @brief A tag to identify the view ordered by indices for boost::multi_index.
  struct index_index{};

public:

  /// @brief The type of the set of all places.
  using places_type =
    multi_index_container<
      place
    , indexed_by< // keep insertion order
                  sequenced<tag<insertion_index>>
                   // sort by id
                ,  ordered_unique< tag<id_index>
                                 , member<place, const std::string, &place::id>>
                  // sort by marking
                , ordered_non_unique< tag<marking_index>
                                    , member<place, unsigned int, &place::marking>>>>;

  /// @brief The type of the set of all transitions.
  using transitions_type =
    multi_index_container<
      transition
    , indexed_by< // sort by id
                  ordered_unique< tag<id_index>
                                , member<transition, const std::string, &transition::id>>
                  // sort by index
                , ordered_unique< tag<index_index>
                                , member<transition, const std::size_t, &transition::index>>>>;

  /// @brief The Petri net's name.
  std::string name;

  /// @brief The set of places.
  places_type places_set;

  /// @brief The set of transitions.
  transitions_type transitions_set;

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
  add_place(const std::string& id, valuation_type marking);

  /// @brief Add a transition.
  ///
  /// If the transition already exists, no operation is done.
  const transition&
  add_transition(const std::string& tid);

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
  add_pre_place( const std::string& tid, const std::string& pre, valuation_type weight
               , arc::type ty = arc::type::normal);

  /// @brief Return all places by insertion order.
  const places_type::index<insertion_index>::type&
  places_by_insertion() const noexcept;

  /// @brief Return all places by identifier.
  const places_type::index<id_index>::type&
  places() const noexcept;

  /// @brief Return all transitions.
  const transitions_type::index<id_index>::type&
  transitions() const noexcept;

  /// @brief Get a transition using its index.
  const transition&
  get_transition_by_index(std::size_t index) const;

  /// @brief Add a time interval to a transition.
  void
  add_time_interval(const std::string& tid, clock_type low, clock_type high);

  /// @brief Tell if some transitions are timed.
  bool
  timed() const noexcept;

  /// @brief Tell if a transition is initially enabled.
  bool enabled(const std::string& tid) const;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn