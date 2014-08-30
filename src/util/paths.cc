#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "util/paths.hh"

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
output_file(const std::string& p)
{
  boost::filesystem::path path(p);
  auto parent_dir = util::canonize_path(path.parent_path().string());
  return parent_dir / path.filename();
}

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<std::istream>
select_input(const conf::configuration& conf)
{
  if (conf.read_stdin)
  {
    std::ios_base::sync_with_stdio(false);
    return std::shared_ptr<std::istream>(&std::cin, [](std::istream*){}); // Don't erase cin.
  }
  else
  {
    auto path = canonize_path(conf.file_name);
    if (not boost::filesystem::is_regular_file(path))
    {
      throw std::runtime_error(conf.file_name + ": not a regular file");
    }
    return std::shared_ptr<std::istream>(new boost::filesystem::ifstream((path)));
  }
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::util
