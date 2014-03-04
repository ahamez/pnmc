#ifndef _PNMC_MC_TIMED_HH_
#define _PNMC_MC_TIMED_HH_

#include <functional> // hash
#include <iosfwd>

#include <sdd/sdd.hh>

#include "conf/configuration.hh"

namespace pnmc { namespace mc {

/*------------------------------------------------------------------------------------------------*/

/// @brief A generic function which throws an sdd::interrupt when a given ammount of time has been
/// reached.
template <typename Fun>
struct timed
{
  /// @brief The configuration keeps the timing informations.
  const bool& stop;

  /// @brief The real function to apply
  const Fun fun;

  /// @brief Constructor.
  ///
  /// Delegates to the contained function.
  template <typename... Args>
  timed(const bool& stop, Args&&... args)
    : stop(stop), fun(std::forward<Args>(args)...)
  {}

  /// @brief Function application
  template <typename T>
  T
  operator()(const T& x)
  const
  {
    if (stop)
    {
      throw sdd::interrupt<sdd::SDD<sdd::conf1>>();
    }
    return fun(x);
  }
};

/// @brief Equality of two timed.
template <typename Fun>
bool
operator==(const timed<Fun>& lhs, const timed<Fun>& rhs)
noexcept
{
  return lhs.fun == rhs.fun;
}

/// @brief Textual output of a timed.
template <typename Fun>
std::ostream&
operator<<(std::ostream& os, const timed<Fun>& t)
{
  return os << "timed(" << t.fun << ")";
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::mc

namespace std {

/*------------------------------------------------------------------------------------------------*/

template <typename Fun>
struct hash<pnmc::mc::timed<Fun>>
{
  std::size_t
  operator()(const pnmc::mc::timed<Fun>& t)
  const noexcept
  {
    return std::hash<Fun>()(t.fun);
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_TIMED_HH_
