#include <fstream>
#include <iostream>
#include <stdexcept>

#include "util/select_input.hh"

namespace pnmc { namespace util {

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<std::istream>
select_input(const conf::configuration& conf)
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
      throw std::runtime_error("Can't open " + conf.file_name);
    }
    return ptr;
  } 
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::util
