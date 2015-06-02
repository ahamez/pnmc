/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include "support/pn/module.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

void
module_node::add_submodule(const module& m)
{
  all_.push_back(m);
  struct helper
  {
    using result_type = void;
    places_list& places;
    nodes_list& nodes;
    void operator()(const place& p)       const noexcept {places.push_back(p);}
    void operator()(const module_node& n) const noexcept {nodes.push_back(n);}
  };
  boost::apply_visitor(helper{places_, nodes_}, m.variant());
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
