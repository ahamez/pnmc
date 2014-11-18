#pragma once

#include <boost/program_options.hpp>

#include "support/parsers/configuration.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

boost::program_options::options_description
input_options();

/*------------------------------------------------------------------------------------------------*/

parsers::configuration
configure_parser(const boost::program_options::variables_map&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
