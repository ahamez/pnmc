#include "conf/options.hh"

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

static const auto nupn_str = "nupn";
static const auto pnml_str = "pnml";
static const auto tina_str = "tina";
static const auto xml_str  = "xml";
static const auto decompress_str = "decompress";

/*------------------------------------------------------------------------------------------------*/

boost::program_options::options_description
file_options()
{
  boost::program_options::options_description options("Input file format options");
  options.add_options()
    (nupn_str       , "Read input file as ")
    (pnml_str       , "Parse PNML format")
    (tina_str       , "Parse TINA format")
    (xml_str        , "Parse pnmc's XML format (default)")
    (decompress_str , "Decompress file to read")
    ;

  return options;
}

/*------------------------------------------------------------------------------------------------*/

pn_format
pn_format_from_options(const boost::program_options::variables_map& vm)
{
  const bool nupn = vm.count(nupn_str);
  const bool pnml = vm.count(pnml_str);
  const bool tina = vm.count(tina_str);

  if (not (nupn or pnml or tina))
  {
    return pn_format::xml;
  }
  else if (not (nupn xor pnml xor tina))
  {
    throw boost::program_options::error("Can specify only one input format.");
  }
  else
  {
    if (nupn)
    {
      return pn_format::nupn;
    }
    else if (pnml)
    {
      return pn_format::pnml;
    }
    else // tina
    {
      return pn_format::tina;
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

bool
decompress(const boost::program_options::variables_map& vm)
{
  return vm.count(decompress_str);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
