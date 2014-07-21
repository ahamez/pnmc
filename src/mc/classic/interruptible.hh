#ifndef _PNMC_MC_INTERRUPTIBLE_HH_
#define _PNMC_MC_INTERRUPTIBLE_HH_

#include <functional> // hash
#include <ostream>

#include "conf/configuration.hh"
#include "mc/classic/exceptions.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

/// @brief A generic function which throws an interrupted exception when the stop flag has been set.
template <typename C, typename Fun>
struct interruptible
{
  /// @brief The configuration keeps the timing informations.
  const bool& stop_;

  /// @brief The real function to apply
  const Fun fun_;

  /// @brief Constructor.
  ///
  /// Delegates to the contained function.
  template <typename... Args>
  interruptible(const bool& stop, Args&&... args)
    : stop_(stop), fun_(std::forward<Args>(args)...)
  {}

  /// @brief Function application
  template <typename T>
  T
  operator()(const T& x)
  const
  {
    if (stop_)
    {
      throw interrupted<C>();
    }
    return fun_(x);
  }

  /// @brief Equality of two interruptible.
  friend
  bool
  operator==(const interruptible& lhs, const interruptible& rhs)
  noexcept
  {
    return lhs.fun_ == rhs.fun_;
  }

  /// @brief Textual output of a interruptible.
  friend
  std::ostream&
  operator<<(std::ostream& os, const interruptible& t)
  {
    return os << "interruptible(" << t.fun_ << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std {

/*------------------------------------------------------------------------------------------------*/

template <typename C, typename Fun>
struct hash<pnmc::mc::classic::interruptible<C, Fun>>
{
  std::size_t
  operator()(const pnmc::mc::classic::interruptible<C, Fun>& t)
  const noexcept
  {
    return std::hash<Fun>()(t.fun_);
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_INTERRUPTIBLE_HH_
