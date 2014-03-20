#ifndef _PNMC_PN_STATISTICS_SERIALIZE_HH_
#define _PNMC_PN_STATISTICS_SERIALIZE_HH_

#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

#include "pn/statistics.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

template<class Archive>
void
save(Archive& archive, const statistics& s)
{
  archive( cereal::make_nvp("places", s.nb_places)
         , cereal::make_nvp("transitions", s.nb_transitions)
         , cereal::make_nvp("place transition ratio", s.place_transition_ratio));
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn

#endif // _PNMC_PN_STATISTICS_SERIALIZE_HH_
