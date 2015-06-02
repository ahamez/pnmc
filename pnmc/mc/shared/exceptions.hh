/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <exception>
#include <string>

namespace pnmc { namespace mc { namespace shared {

/*------------------------------------------------------------------------------------------------*/

struct bound_error final
  : public std::exception
{
  const std::string& place;

  bound_error(const std::string& p)
    : place(p)
  {}
};

/*------------------------------------------------------------------------------------------------*/

struct interrupted final
  : public std::exception
{};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::shared
