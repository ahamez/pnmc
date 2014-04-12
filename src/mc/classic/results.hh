#ifndef _PNMC_MC_CLASSIC_RESULTS_HH_
#define _PNMC_MC_CLASSIC_RESULTS_HH_

#include <boost/multiprecision/cpp_int.hpp>

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct results
{
  boost::multiprecision::cpp_int nb_states;
  unsigned long nb_fired_transitions;
  unsigned long max_token_markings;
  unsigned long max_token_places;

  results()
    : nb_states(0), nb_fired_transitions(0), max_token_markings(0), max_token_places(0)
  {}
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

#endif // _PNMC_MC_CLASSIC_RESULTS_HH_
