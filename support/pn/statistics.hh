#pragma once

#include <utility> // pair
#include <vector>

#include "support/pn/net.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

struct statistics
{
  const std::size_t nb_places;
  const std::size_t nb_transitions;
  const std::vector<std::pair<unsigned int /*pre*/, unsigned int /*post*/>> place_transition;

  statistics(const net& n)
    : nb_places(n.places().size())
    , nb_transitions(n.transitions().size())
    , place_transition(mk_place_transition(n))
  {}

private:

  static
  std::vector<std::pair<unsigned int /*pre*/, unsigned int /*post*/>>
  mk_place_transition(const net& n)
  {
    std::vector<std::pair<unsigned int, unsigned int>> res;
    res.reserve(n.places().size());
    for (const auto& place : n.places())
    {
      res.emplace_back(place.pre.size(), place.post.size());
    }
    return res;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
