#ifndef _PNMC_MC_CLASSIC_RESULTS_HH_
#define _PNMC_MC_CLASSIC_RESULTS_HH_

#include <sdd/util/boost_multiprecision_no_warnings.hh>

#include "conf/configuration.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct results
{
  const conf::configuration& conf;

  boost::multiprecision::cpp_int nb_states;
  unsigned long max_token_markings;
  unsigned long max_token_places;

  results(const conf::configuration& c)
    : conf(c), nb_states(0), max_token_markings(0), max_token_places(0)
  {}
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

#endif // _PNMC_MC_CLASSIC_RESULTS_HH_
