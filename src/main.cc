#include <iostream>

#include <boost/program_options/errors.hpp>
#include <sdd/order/order_error.hh>

#include "conf/fill_configuration.hh"
#include "mc/mc.hh"
#include "parsers/parse.hh"
#include "parsers/parse_error.hh"
#include "pn/tina.hh"
#include "util/export_to_tina.hh"
#include "util/paths.hh"

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
    const auto net_ptr = [&]
    {
      auto in = util::select_input(conf);
      return parsers::parse(conf, *in);
    }();
    util::export_to_tina(conf, *net_ptr);
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
  catch (const parsers::parse_error& e)
  {
    std::cerr << "Error when parsing input:" << std::endl;
    std::cerr << e.what() << std::endl;
    std::cerr << "Exiting." << std::endl;
    return 2;
  }
  catch (const sdd::order_error& e)
  {
    std::cerr << "Order error:" << std::endl;
    std::cerr << e.what() << std::endl;
    std::cerr << "Exiting." << std::endl;
    return 3;
  }
  catch (const std::runtime_error& e)
  {
    std::cerr << "Error:" << std::endl;
    std::cerr << e.what() << std::endl;
    std::cerr << "Exiting." << std::endl;
    return 4;
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
