#ifndef _PNMC_MC_CLASSIC_RESULTS_SERIALIZE_HH_
#define _PNMC_MC_CLASSIC_RESULTS_SERIALIZE_HH_

#include <cereal/archives/json.hpp>

#include "conf/configuration.hh"
#include "mc/classic/results.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

template<class Archive>
void
save(Archive& archive, const results& r)
{
  archive( cereal::make_nvp("states", r.nb_states.template convert_to<long double>())
         , cereal::make_nvp("fired transitions", r.nb_fired_transitions)
         , cereal::make_nvp("maximal number of tokens per marking", r.max_token_markings)
         , cereal::make_nvp("maximal number of tokens in a place", r.max_token_places));
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

#endif // _PNMC_MC_CLASSIC_RESULTS_SERIALIZE_HH_
