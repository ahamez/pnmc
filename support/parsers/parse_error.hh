/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <exception>
#include <string>

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

struct parse_error final
  : public std::exception
{
private:

  const std::string message_;

public:

  parse_error(std::string  message);
  parse_error();

  ~parse_error() noexcept;

  const char* what() const noexcept override;
};

/*------------------------------------------------------------------------------------------------*/

struct unsupported_error final
  : public std::exception
{
private:

  const std::string message_;

public:

  unsupported_error(std::string  message);
  unsupported_error();

  ~unsupported_error() noexcept;

  const char* what() const noexcept override;
};


/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
