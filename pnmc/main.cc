#include <iostream>
#include <new>
#include <sysexits.h>

#include <boost/program_options/errors.hpp>
#include <sdd/order/order_error.hh>

#include "conf/fill_configuration.hh"
#include "mc/mc.hh"
#include "shared/parsers/parse.hh"
#include "shared/parsers/parse_error.hh"
#include "shared/util/paths.hh"

/*------------------------------------------------------------------------------------------------*/

int
main(int argc, const char** argv)
{
  using namespace pnmc;

  try
  {
    const auto conf_opt = conf::fill_configuration(argc, argv);
    if (conf_opt)
    {
      const auto& conf = *conf_opt;
      const auto net_ptr = parsers::parse(conf.input);
      mc::mc worker(conf);
      worker(*net_ptr);
    }
    return EX_OK;
  }
  catch (const boost::program_options::error& e)
  {
    std::cerr << e.what() << std::endl;
    return EX_USAGE;
  }
  catch (const parsers::parse_error& e)
  {
    std::cerr << "Error when parsing input:" << std::endl;
    std::cerr << e.what() << std::endl;
    return EX_DATAERR;
  }
  catch (const sdd::order_error& e)
  {
    std::cerr << "Order error:" << std::endl;
    std::cerr << e.what() << std::endl;
    return EX_DATAERR;
  }
  catch (const std::runtime_error& e)
  {
    std::cerr << "Error:" << std::endl;
    std::cerr << e.what() << std::endl;
    return EX_DATAERR;
  }
  catch (const std::bad_alloc&)
  {
    std::cerr << "Can't allocate more memory" << std::endl;
    return EX_OSERR;
  }
  catch (std::exception& e)
  {
    std::cerr << "Error unknown. Please report the following to a.hamez@isae.fr." << std::endl;
    std::cerr << e.what() << std::endl;
    std::cerr << "Exiting." << std::endl;
    return EX_SOFTWARE;
  }
}

/*------------------------------------------------------------------------------------------------*/
