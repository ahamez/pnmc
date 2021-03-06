/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <boost/program_options.hpp>

#include "support/parsers/pn_configuration.hh"
#include "support/parsers/properties_configuration.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

boost::program_options::options_description
pn_input_options();

/*------------------------------------------------------------------------------------------------*/

parsers::pn_configuration
configure_pn_parser(const boost::program_options::variables_map&);

/*------------------------------------------------------------------------------------------------*/

boost::program_options::options_description
properties_input_options();

/*------------------------------------------------------------------------------------------------*/

parsers::properties_configuration
configure_properties_parser(const boost::program_options::variables_map&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
