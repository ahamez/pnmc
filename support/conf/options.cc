#include <map>
#include <string>
#include <utility>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/map.hpp>

#include "support/conf/options.hh"
#include "support/conf/pn_format.hh"
#include "support/util/paths.hh"

namespace pnmc { namespace conf {

namespace po = boost::program_options;

/*------------------------------------------------------------------------------------------------*/

static const auto pn_input_str   = "input";
static const auto format_map = std::map<std::string, pn_format>
  { std::make_pair("ndr" , pn_format::ndr)
  , std::make_pair("net" , pn_format::net)
  , std::make_pair("nupn", pn_format::nupn)
  , std::make_pair("pnml", pn_format::pnml)
  };

static const auto possible_format_values = [&]
{
  using namespace boost::adaptors;
  using namespace std::string_literals;
  return "Petri net format ["s + boost::algorithm::join(format_map | map_keys, "|") + "]"s;
}();

/*------------------------------------------------------------------------------------------------*/

po::options_description
input_options()
{
  po::options_description options("Input file format options");
  options.add_options()
    (pn_input_str, po::value<std::string>()->default_value("net"), possible_format_values.c_str());
  return options;
}

/*------------------------------------------------------------------------------------------------*/

static
pn_format
pn_format_from_options(const po::variables_map& vm)
{
  const auto s = vm[pn_input_str].as<std::string>();
  const auto search = format_map.find(s);
  if (search != end(format_map))
  {
    return search->second;
  }
  else
  {
    throw po::error("Invalid input format " + s);
  }
}

/*------------------------------------------------------------------------------------------------*/

parsers::configuration
configure_parser(const boost::program_options::variables_map& vm)
{
  parsers::configuration conf;
  if (vm["input-file"].as<std::string>() != "-")
  {
    conf.file = util::in_file(vm["input-file"].as<std::string>());
  }
  conf.file_type = pn_format_from_options(vm);
  return conf;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
