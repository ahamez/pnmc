#include <stdexcept>

#include "support/util/paths.hh"

namespace pnmc { namespace util {

/*------------------------------------------------------------------------------------------------*/

boost::filesystem::path
file(const std::string& p)
{
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
