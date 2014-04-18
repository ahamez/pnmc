#ifndef _PNMC_UTIL_TIMER_HH_
#define _PNMC_UTIL_TIMER_HH_

#include <chrono>

namespace pnmc { namespace util {

/*------------------------------------------------------------------------------------------------*/

/// @brief Helps measure time.
class timer
{
private:

  std::chrono::time_point<std::chrono::system_clock> start_;

public:

  timer()
    : start_(std::chrono::system_clock::now())
  {}

  void
  reset()
  {
    start_ = std::chrono::system_clock::now();
  }

  std::chrono::duration<double>
  duration()
  {
    return std::chrono::system_clock::now() - start_;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::util

#endif // _PNMC_UTIL_TIMER_HH_
