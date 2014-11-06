#pragma once

#include <boost/multiprecision/cpp_int.hpp>

#include "conf/configuration.hh"

namespace pnmc { namespace mc { namespace shared {

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

}}} // namespace pnmc::mc::shared
