#pragma once

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

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const
  {
    sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
    builder.reserve(val.size());
    for (const auto v : val)
    {
      if ((v + post) >= pre)
      {
        builder.insert(builder.end(), v);
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

  friend
  bool
  operator==(const enabled& lhs, const enabled& rhs)
  noexcept
  {
    return lhs.pre == rhs.pre and lhs.post == rhs.post;
  }

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
