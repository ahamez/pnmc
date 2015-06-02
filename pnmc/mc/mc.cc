/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include "mc/mc.hh"
#include "mc/mc_impl.hh"
#include "mc/classic/worker.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

mc::mc(const conf::configuration& conf)
  : impl_(std::shared_ptr<mc_impl>(new classic::worker{conf}))
{}

/*------------------------------------------------------------------------------------------------*/

void
mc::operator()(const pn::net& net, const properties::formulae& formulae)
{
  impl_->operator()(net, formulae);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc
