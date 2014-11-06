#include <stdexcept>
#include <string>

#include "shared/util/paths.hh"

namespace pnmc { namespace util {

/*------------------------------------------------------------------------------------------------*/

std::string
trim(const std::string& str, const std::string& whitespaces = " \t")
{
  const auto begin = str.find_first_not_of(whitespaces);
  if (begin == std::string::npos)
  {
    return "";
  }
  const auto count = str.find_last_not_of(whitespaces) - begin + 1;
  return str.substr(begin, count);
}

/*------------------------------------------------------------------------------------------------*/

boost::filesystem::path
canonize_path(const std::string& path)
{
  auto tmp = trim(path);
  if (tmp.empty())
  {
    throw std::runtime_error("Empty path");
  }
  if (tmp[0] == '~')
  {
    tmp.replace(tmp.begin(), tmp.begin() + 1, std::getenv("HOME"));
  }
  try
  {
    return boost::filesystem::canonical(tmp);
  }
  catch (const boost::filesystem::filesystem_error&)
  {
    throw std::runtime_error(path + ": no such file or directory");
  }
}

/*------------------------------------------------------------------------------------------------*/

boost::filesystem::path
file(const std::string& p)
{
  boost::filesystem::path path(p);
  const auto parent_dir_str = trim(path.parent_path().string());
  boost::filesystem::path parent_dir;
  if (not parent_dir_str.empty())
  {
    parent_dir = util::canonize_path(parent_dir_str);
  }
  return parent_dir / path.filename();
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
  return f;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::util
