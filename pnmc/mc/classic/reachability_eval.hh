#pragma once

#include "mc/classic/reachability_ast.hh"
#include "mc/classic/sdd.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

int
eval(const integer_ast&, const SDD&);

/*------------------------------------------------------------------------------------------------*/

bool
eval(const boolean_ast&, const SDD&);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
