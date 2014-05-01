#include <cstdio>
#include <fstream>
#include <iostream>

#include <boost/program_options/errors.hpp>
#include <sdd/order/order_error.hh>

#include "conf/fill_configuration.hh"
#include "mc/mc.hh"
#include "parsers/parse.hh"
#include "parsers/parse_error.hh"
#include "pn/tina.hh"

/*------------------------------------------------------------------------------------------------*/

namespace pnmc {

struct unreadable_file
  : std::exception
{
  const std::string name;
  unreadable_file(const std::string& n) : name(n) {}
};

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
      throw unreadable_file(conf.file_name);
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
    conf_opt = conf::fill_configuration(argc, argv);

    if (not conf_opt) // --help or --version
    {
      return 0;
    }

    const auto& conf = *conf_opt;
    auto in = file_or_cin(conf);
    const auto net_ptr = parsers::parse(conf, *in);
    if (conf.delete_file and not conf.read_stdin)
    {
      ::remove(conf.file_name.c_str());
    }
    if (conf.export_tina_file)
    {
      std::ofstream file(*conf.export_tina_file);
      if (file.is_open())
      {
        pn::tina(file, *net_ptr);
      }
      else
      {
        std::cerr << "Can't export Petri net to " << *conf.export_tina_file << std::endl;
      }
    }
    mc::mc worker(conf);
    worker(*net_ptr);
    return 0;
  }
  catch (const boost::program_options::error& e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << "Exiting." << std::endl;
    return 1;
  }
  catch (const unreadable_file& e)
  {
    std::cerr << "Can't open '" << e.name << "'." << std::endl;
    std::cerr << "Exiting." << std::endl;
    return 2;
  }
  catch (const parsers::parse_error& e)
  {
    std::cerr << "Error when parsing input." << std::endl;
    std::cerr << e.what() << std::endl;
    std::cerr << "Exiting." << std::endl;
    return 3;
  }
  catch (const sdd::order_error& e)
  {
    std::cerr << "Order error." << std::endl;
    std::cerr << e.what() << std::endl;
    std::cerr << "Exiting." << std::endl;
    return 4;
  }
  catch (const std::runtime_error& e)
  {
    std::cerr << "Other error." << std::endl;
    std::cerr << e.what() << std::endl;
    std::cerr << "Exiting." << std::endl;
    return 5;
  }
  catch (std::exception& e)
  {
    std::cerr << "Error unknown. Please report the following to a.hamez@isae.fr." << std::endl;
    std::cerr << e.what() << std::endl;
    std::cerr << "Exiting." << std::endl;
    return -1;
  }
}

/*------------------------------------------------------------------------------------------------*/
