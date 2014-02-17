#include <exception>
#include <string>

namespace pnmc { namespace mc {

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

}} // namespace pnmc::mc
