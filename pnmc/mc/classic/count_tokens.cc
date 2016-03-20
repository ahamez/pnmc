/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <algorithm> // max, max_element
#include <cassert>
#include <unordered_map>
#include <utility>   // pair
#include <tuple>

#include <boost/range/algorithm/for_each.hpp>

#include "mc/classic/count_tokens.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

namespace /* anonymous */{

/// @internal
struct count_tokens_visitor
{
  using result_type = std::pair<unsigned long, unsigned long>;

  /// @brief A cache is necessary to know if a node has already been encountered.
  ///
  /// We use the addresses of nodes as key. It's legit because nodes are unified and immutable.
  mutable std::unordered_map<const char*, result_type> cache_;

  /// @brief |0|.
  result_type
  operator()(const sdd::zero_terminal<sdd::conf1>&)
  const
  {
    assert(false && "|0|");
    __builtin_unreachable();
  }

  /// @brief |1|.
  result_type
  operator()(const sdd::one_terminal<sdd::conf1>&)
  const
  {
    return {0ul, 0ul};
  }

  /// @brief Flat SDD.
  result_type
  operator()(const sdd::flat_node<sdd::conf1>& n)
  const
  {
    auto insertion = cache_.emplace(reinterpret_cast<const char*>(&n), std::make_pair(0, 0));
    if (insertion.second)
    {
      auto markings = 0ul;
      auto places = 0ul;

      for (const auto& arc : n)
      {
        const auto max_value = *std::max_element(arc.valuation().cbegin(), arc.valuation().cend());
        const auto res = visit(*this, arc.successor());

        markings = std::max(markings, max_value + res.first);
        places = std::max({places, static_cast<unsigned long>(max_value), res.second});
      }

      insertion.first->second = std::make_pair(markings, places);
    }
    return insertion.first->second;
  }

  /// @brief Hierarchical SDD.
  result_type
  operator()(const sdd::hierarchical_node<sdd::conf1>& n)
  const
  {
    auto insertion = cache_.emplace(reinterpret_cast<const char*>(&n), std::make_pair(0, 0));
    if (insertion.second)
    {
      auto markings = 0ul;
      auto places = 0ul;

      for (const auto& arc : n)
      {
        const auto val_res = visit(*this, arc.valuation());
        const auto succ_res = visit(*this, arc.successor());

        markings = std::max(markings, val_res.first + succ_res.first);
        places = std::max({places, val_res.second, succ_res.second});
      }

      insertion.first->second = std::make_pair(markings, places);
    }
    return insertion.first->second;
  }
}; // struct count_tokens_visitor

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

void
count_tokens(results& res, const SDD& states, const pn::net& net)
{
  res.max_token_markings = 0;
  res.max_token_places = 0;

  std::tie(*res.max_token_markings, *res.max_token_places) = visit(count_tokens_visitor(), states);

  // Add markings of (useless) places that are connected.
  boost::range::for_each(net.places(), [&](const auto& place)
                                          {
                                            if (not place.connected())
                                            {
                                              *res.max_token_markings += place.marking;
                                            }
                                          });
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
