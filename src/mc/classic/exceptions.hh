#ifndef _PNMC_MC_CLASSIC_EXCEPTIONS_HH_
#define _PNMC_MC_CLASSIC_EXCEPTIONS_HH_

#include <exception>
#include <string>

namespace pnmc { namespace mc { namespace classic {

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

}}} // namespace pnmc::mc::classic

#endif // _PNMC_MC_CLASSIC_EXCEPTIONS_HH_
