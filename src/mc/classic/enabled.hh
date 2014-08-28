#ifndef _PNMC_MC_ENABLED_HH_
#define _PNMC_MC_ENABLED_HH_

#include <functional> // hash
#include <ostream>

#include <sdd/util/hash.hh>
#include "sdd/values/flat_set.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct enabled
{
  const unsigned int pre;
  const unsigned int post;

  enabled(unsigned int pr, unsigned int po)
    : pre(pr), post(po)
  {}

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const
  {
    sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
    for (const auto v : val)
    {
      if ((v + post) >= pre)
      {
        builder.insert(v);
      }
    }
    return std::move(builder);
  }

  bool
  selector()
  const noexcept
  {
    return true;
  }

  /// @brief Equality.
  friend
  bool
  operator==(const enabled& lhs, const enabled& rhs)
  noexcept
  {
    return lhs.pre == rhs.pre and lhs.post == rhs.post;
  }

  /// @brief Textual output.
  friend
  std::ostream&
  operator<<(std::ostream& os, const enabled& e)
  {
    return os << "enabled(" << e.pre << "," << e.post << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<pnmc::mc::classic::enabled>
{
  std::size_t
  operator()(const pnmc::mc::classic::enabled& e)
  const noexcept
  {
    using namespace sdd::hash;
    return seed(8067541972) (val(e.pre)) (val(e.post));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _PNMC_MC_ENABLED_HH_
