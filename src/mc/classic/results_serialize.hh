#pragma once

#include <cereal/archives/json.hpp>

#include "conf/configuration.hh"
#include "mc/classic/results.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

template<class Archive>
void
save(Archive& archive, const results& r)
{
  archive(cereal::make_nvp("states", r.nb_states.template convert_to<long double>()));
  if (r.conf.count_tokens)
  {
    archive( cereal::make_nvp("maximal number of tokens per marking", r.max_token_markings)
           , cereal::make_nvp("maximal number of tokens in a place", r.max_token_places));
  }
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
