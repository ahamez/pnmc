#include <fstream>
#include <iostream>
#include <stdexcept>

#include "conf/fill_configuration.hh"
#include "mc/work.hh"
#include "parsers/parse.hh"

/*------------------------------------------------------------------------------------------------*/

int
main(int argc, char** argv)
{
  using namespace pnmc;
  try
  {
    const auto conf_opt = conf::fill_configuration(argc, argv);

    if (conf_opt) // configuration succeeded
    {
      const std::string& file_name = conf_opt->file_name;

      // Try to open file.
      std::ifstream in(file_name);
      if (not in.is_open())
      {
        std::cerr << "File \'" << file_name << "\' could not be opened." << std::endl;
        return 1;
      }

      // Try to parse the model.
      const auto net_ptr = parsers::parse(in);
      if (not net_ptr)
      {
        std::cerr << "\'" << file_name << "\' is not a valid TINA or PROD file." << std::endl;
        return 1;
      }

      // Launch the model checking process.
      mc::work(*conf_opt, *net_ptr);
    }
    return 0;
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}

/*------------------------------------------------------------------------------------------------*/
