#pragma once

#include <chrono>

namespace pnmc { namespace util {

/*------------------------------------------------------------------------------------------------*/

/// @brief Helps measure time.
class timer
{
private:

  std::chrono::time_point<std::chrono::steady_clock> start_;

public:

  timer()
    : start_(std::chrono::steady_clock::now())
  {}

  void
  reset()
  {
    start_ = std::chrono::steady_clock::now();
  }

  std::chrono::duration<double>
  duration()
  {
    return std::chrono::steady_clock::now() - start_;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::util
