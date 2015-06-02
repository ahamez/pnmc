/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <iosfwd>

#include "support/util/timer.hh"

namespace pnmc { namespace mc { namespace shared {

/*------------------------------------------------------------------------------------------------*/

struct step
{
  util::timer timer;
  const std::string name;
  std::chrono::duration<double>* stat;
  std::ostream& os;

  step(const std::string& n, std::chrono::duration<double>* s, std::ostream& o)
    : timer{}, name{n}, stat{s}, os{o}
  {
    os << name << std::setw(static_cast<int>(20 - name.size())) << ": " << std::flush;
  }

  step(const std::string& n, std::chrono::duration<double>* s)
    : step{n, s, std::cout}
  {}

  step(const std::string& n, std::ostream& o)
    : step{n, nullptr, o}
  {}

  step(const std::string& n)
    : step{n, nullptr, std::cout}
  {}

  ~step()
  {
    os  << timer.duration().count() << "s" << std::endl;
    if (stat)
    {
      *stat = timer.duration();
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::shared
