#ifndef _PNMC_MC_BOUNDED_ERROR_HH_
#define _PNMC_MC_BOUNDED_ERROR_HH_

#include <exception>
#include <string>

#include <sdd/hom/interrupt.hh>

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct bound_error final
  : public sdd::interrupt<C>
{
  const std::string& place;

  bound_error(const std::string& p)
    : sdd::interrupt<C>(), place(p)
  {}
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

#endif // _PNMC_MC_BOUNDED_ERROR_HH_
