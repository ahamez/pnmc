/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <unordered_set>

#include <boost/iterator/transform_iterator.hpp>

#include "mc/classic/place_bound.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

using variables_bounds_type = std::unordered_map<sdd::order_position_type, pn::valuation_type>;

struct bound_visitor
{
  std::unordered_set<const void*> visited;
  variables_bounds_type& bounds;

  bound_visitor(variables_bounds_type& b)
    : visited(), bounds(b)
  {}

  [[noreturn]]
  void
  operator()(const sdd::zero_terminal<sdd_conf>&, const order&)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }

  void
  operator()(const sdd::one_terminal<sdd_conf>&, const order&)
  const noexcept
  {}

  void
  operator()(const sdd::flat_node<sdd_conf>& n, const order& o)
  {
    if (not visited.count(&n))
    {
      if (bounds.count(o.position()))
      {
        pn::valuation_type current_max = 0;
        for (const auto& arc : n)
        {
          current_max = std::max(current_max, *arc.valuation().rbegin());
          visit(*this, arc.successor(), o.next());
        }
        if (bounds.at(o.position()) < current_max)
        {
          bounds[o.position()] = current_max;
        }
      }
      else
      {
        for (const auto& arc : n)
        {
          visit(*this, arc.successor(), o.next());
        }
      }
      visited.emplace_hint(end(visited), &n);
    }
  }

  void
  operator()(const sdd::hierarchical_node<sdd_conf>& n, const order& o)
  {
    if (not visited.count(&n))
    {
      for (const auto& arc : n)
      {
        visit(*this, arc.valuation(), o.nested());
        visit(*this, arc.successor(), o.next());
      }
      visited.emplace_hint(end(visited), &n);
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

std::unordered_map<std::string /* place id */, pn::valuation_type>
place_bound(const order& o, const SDD& states, const std::set<std::string>& places)
{
  if (places.empty())
  {
    return std::unordered_map<std::string, pn::valuation_type>{};
  }

  const auto init = [&](const auto& place){return std::make_pair(o.node(place).position(), 0);};
  auto variables_bounds =
    variables_bounds_type{ boost::make_transform_iterator(begin(places), init)
                         , boost::make_transform_iterator(end(places), init)};

  sdd::visit(bound_visitor{variables_bounds}, states, o);

  const auto to_id = [&](auto kv)
  {
    return std::make_pair(o.node_from_position(kv.first).identifier().user(), kv.second);
  };
  return { boost::make_transform_iterator(begin(variables_bounds), to_id)
         , boost::make_transform_iterator(end(variables_bounds), to_id)};
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
