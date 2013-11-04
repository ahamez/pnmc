#include <fstream>
#include <iostream>

#include "conf/fill_configuration.hh"
#include "mc/work.hh"
#include "parsers/parse.hh"
#include "parsers/parse_error.hh"

/*------------------------------------------------------------------------------------------------*/

int
main(int argc, char** argv)
{
  using namespace pnmc;
  const auto conf_opt = conf::fill_configuration(argc, argv);

  if (conf_opt) // configuration succeeded
  {
    const std::string& file_name = conf_opt->file_name;

    // Try to open file.
    std::ifstream in(file_name);
    if (not in.is_open())
    {
      std::cerr << "File '" << file_name << "' could not be opened." << std::endl;
      return 1;
    }

    // Try to parse the model.
    try
    {
      const auto net_ptr = parsers::parse(*conf_opt, in);
      mc::work(*conf_opt, *net_ptr);
    }
    catch (const parsers::parse_error& p)
    {
      std::cerr << "Error when parsing " << file_name << "." << std::endl;
      std::cerr << p.what() << std::endl;
      return 1;
    }
  }
  return 0;
}

/*------------------------------------------------------------------------------------------------*/
