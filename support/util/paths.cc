/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#include <cstdlib>
#include <stdexcept>

#include <boost/algorithm/string.hpp>

#include "support/util/paths.hh"

namespace pnmc { namespace util {

/*------------------------------------------------------------------------------------------------*/

boost::filesystem::path
file(std::string p)
{
  boost::algorithm::trim(p);
  if (p.size() > 0 and p[0] == '~')
  {
    p.replace(0, 1, std::getenv("HOME"));
  }
  return boost::filesystem::absolute(boost::filesystem::path(p));
}

/*------------------------------------------------------------------------------------------------*/

boost::filesystem::path
in_file(const std::string& p)
{
  const auto f = file(p);
  if (not boost::filesystem::is_regular_file(f))
  {
    throw std::runtime_error(f.string() + ": not a regular file");
  }
  return boost::filesystem::canonical(f);
}

/*------------------------------------------------------------------------------------------------*/

boost::filesystem::path
directory(const std::string& p)
{
  auto path = file(p);
  boost::filesystem::create_directories(path);
  return boost::filesystem::canonical(path);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::util
