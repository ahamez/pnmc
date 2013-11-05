#include <fstream>
#include <iostream>

#include <boost/program_options/errors.hpp>

#include "conf/fill_configuration.hh"
#include "mc/work.hh"
#include "parsers/parse.hh"
#include "parsers/parse_error.hh"

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
      std::istream* in;
      std::ifstream fstream;
      if (conf_opt->read_stdin)
      {
        in = &std::cin;
      }
      else
      {
        fstream = std::ifstream(conf_opt->file_name);
        if (not fstream.is_open())
        {
          std::cerr << "Can't open " << conf_opt->file_name << std::endl;
          return 1;
        }
        in = &fstream;
      }

      // Try to parse the model.
      try
      {
        const auto net_ptr = parsers::parse(*conf_opt, *in);
        mc::work(*conf_opt, *net_ptr);
      }
      catch (const parsers::parse_error& p)
      {
        std::cerr << "Error when parsing input." << std::endl;
        std::cerr << p.what() << std::endl;
        return 1;
      }
    }
  }
  catch (const boost::program_options::error& e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}

/*------------------------------------------------------------------------------------------------*/
