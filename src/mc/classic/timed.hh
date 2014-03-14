#ifndef _PNMC_MC_TIMED_HH_
#define _PNMC_MC_TIMED_HH_

#include <functional> // hash
#include <iosfwd>

#include <sdd/hom/interrupt.hh>

#include "conf/configuration.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

/// @brief A generic function which throws an sdd::interrupt when a given ammount of time has been
/// reached.
template <typename C, typename Fun>
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
      throw sdd::interrupt<C>();
    }
    return fun(x);
  }
};

/// @brief Equality of two timed.
template <typename C, typename Fun>
bool
operator==(const timed<C, Fun>& lhs, const timed<C, Fun>& rhs)
noexcept
{
  return lhs.fun == rhs.fun;
}

/// @brief Textual output of a timed.
template <typename C, typename Fun>
std::ostream&
operator<<(std::ostream& os, const timed<C, Fun>& t)
{
  return os << "timed(" << t.fun << ")";
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std {

/*------------------------------------------------------------------------------------------------*/

template <typename C, typename Fun>
struct hash<pnmc::mc::classic::timed<C, Fun>>
{
  std::size_t
  operator()(const pnmc::mc::classic::timed<C, Fun>& t)
  const noexcept
  {
    return std::hash<Fun>()(t.fun);
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_TIMED_HH_
