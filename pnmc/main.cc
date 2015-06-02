/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <iostream>
#include <new>
#include <sysexits.h>

#include <boost/program_options/errors.hpp>
#include <sdd/order/order_error.hh>

#include "conf/fill_configuration.hh"
#include "mc/mc.hh"
#include "support/parsers/parse_pn.hh"
#include "support/parsers/parse_properties.hh"
#include "support/parsers/parse_error.hh"
#include "support/util/paths.hh"

/*------------------------------------------------------------------------------------------------*/

int
main(int argc, const char** argv)
{
  using namespace pnmc;

  try
  {
    if (const auto conf = conf::fill_configuration(argc, argv))
    {
      const auto net_ptr = parsers::parse(conf->pn_input);
      const auto properties = parsers::parse(conf->properties_input);
      mc::mc{*conf}(*net_ptr, properties);
    }
    return EX_OK;
  }
  catch (const boost::program_options::error& e)
  {
    std::cerr << e.what() << '\n';
    return EX_USAGE;
  }
  catch (const parsers::parse_error& e)
  {
    std::cerr << "Error when parsing input:\n";
    std::cerr << e.what() << '\n';
    return EX_DATAERR;
  }
  catch (const sdd::order_error& e)
  {
    std::cerr << "Order error:\n";
    std::cerr << e.what() << '\n';
    return EX_DATAERR;
  }
  catch (const std::runtime_error& e)
  {
    std::cerr << "Error:\n";
    std::cerr << e.what() << '\n';
    return EX_DATAERR;
  }
  catch (const std::bad_alloc&)
  {
    std::cerr << "Can't allocate memory\n";
    return EX_OSERR;
  }
  catch (std::exception& e)
  {
    std::cerr << "Error unknown. Please report the following to alexandre.hamez+pnmc@gmail.com.\n";
    std::cerr << e.what() << '\n';
    return EX_SOFTWARE;
  }
}

/*------------------------------------------------------------------------------------------------*/
