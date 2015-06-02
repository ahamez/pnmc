/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

#include "support/pn/statistics.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

template<class Archive>
void
save(Archive& archive, const statistics& s)
{
  archive( cereal::make_nvp("places", s.nb_places)
         , cereal::make_nvp("transitions", s.nb_transitions)
         , cereal::make_nvp("transition by place", s.place_transition));
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
