/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <sdd/sdd.hh>

#include "mc/shared/results.hh"
#include "mc/shared/statistics.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

using sdd_conf = sdd::conf1;

using SDD = sdd::SDD<sdd_conf>;
static auto zero = []{return sdd::zero<sdd_conf>();};
static auto one  = []{return sdd::one<sdd_conf>();};

using flat_set         = sdd_conf::Values;
using flat_set_builder = sdd::values::values_traits<flat_set>::builder;

using homomorphism = sdd::homomorphism<sdd_conf>;
using sdd::composition;
using sdd::fixpoint;
using sdd::function;
using sdd::id;
using sdd::if_then_else;
using sdd::intersection;
using sdd::sum;

using order_identifier = sdd::order_identifier<sdd_conf>;
using order_builder    = sdd::order_builder<sdd_conf>;
using order            = sdd::order<sdd_conf>;

using results    = shared::results<sdd_conf>;
using statistics = shared::statistics<sdd_conf>;

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
