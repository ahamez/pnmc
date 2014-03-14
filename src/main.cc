#include <cstdio>
#include <fstream>
#include <iostream>

#include <boost/program_options/errors.hpp>

#include "conf/fill_configuration.hh"
#include "mc/mc.hh"
#include "parsers/parse.hh"
#include "parsers/parse_error.hh"

/*------------------------------------------------------------------------------------------------*/

namespace pnmc {

struct unreadable_file
  : std::exception
{};

std::shared_ptr<std::istream>
file_or_cin(const conf::configuration& conf)
{
  if (conf.read_stdin)
  {
    std::ios_base::sync_with_stdio(false); // Standard input buffering.
    return std::shared_ptr<std::istream>(&std::cin, [](std::istream*){}); // Avoid to erase cin.
  }
  else
  {
    std::shared_ptr<std::istream> ptr(new std::ifstream(conf.file_name));
    if (not dynamic_cast<std::ifstream&>(*ptr).is_open())
    {
      throw unreadable_file();
    }
    return ptr;
  } 
}

} // namespace pnmc

/*------------------------------------------------------------------------------------------------*/

int
main(int argc, char** argv)
{
  using namespace pnmc;

  try
  {
    boost::optional<conf::configuration> conf_opt;
    try
    {
      conf_opt = conf::fill_configuration(argc, argv);
    }
    catch (const boost::program_options::error& e)
    {
      std::cerr << e.what() << std::endl;
      return 1;
    }

    if (not conf_opt) // --help or --version
    {
      return 0;
    }

    const auto& conf = *conf_opt;
    try
    {
      auto in = file_or_cin(conf);
      const auto net_ptr = parsers::parse(conf, *in);
      if (conf.delete_file and not conf.read_stdin)
      {
        ::remove(conf.file_name.c_str());
      }
      mc::mc worker(conf);
      worker(*net_ptr);
    }
    catch (const unreadable_file&)
    {
      std::cerr << "Can't open '" << conf.file_name << "'." << std::endl;
      return 2;
    }
    catch (const parsers::parse_error& p)
    {
      std::cerr << "Error when parsing input." << std::endl;
      std::cerr << p.what() << std::endl;
      return 3;
    }
    return 0;
  }
  catch (std::exception& e)
  {
    std::cerr << "Fatal error. Please report the following to a.hamez@isae.fr." << std::endl;
    std::cerr << e.what() << std::endl;
    return -1;
  }
}

/*------------------------------------------------------------------------------------------------*/
