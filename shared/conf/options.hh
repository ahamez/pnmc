#pragma once

#pragma GCC diagnostic push
#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <boost/program_options.hpp>
#pragma GCC diagnostic pop

#include "shared/parsers/configuration.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

boost::program_options::options_description
input_options();

/*------------------------------------------------------------------------------------------------*/

parsers::configuration
configure_parser(const boost::program_options::variables_map&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
