#include <exception>
#include <string>

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

struct bound_error final
  : public std::exception
{};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc
