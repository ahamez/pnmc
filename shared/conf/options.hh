#pragma once

#pragma GCC diagnostic push
#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <boost/program_options.hpp>
#pragma GCC diagnostic pop

#include "conf/pn_format.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

boost::program_options::options_description
file_options();

/*------------------------------------------------------------------------------------------------*/

pn_format
pn_format_from_options(const boost::program_options::variables_map&);

/*------------------------------------------------------------------------------------------------*/

bool
decompress(const boost::program_options::variables_map&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
