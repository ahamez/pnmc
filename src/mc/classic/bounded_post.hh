#pragma once

#include <functional> // hash
#include <iosfwd>

#include <sdd/values/flat_set.hh>
#include <sdd/values_manager.hh>

#include "mc/classic/exceptions.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct bounded_post
{
  const unsigned int valuation;
  const unsigned int bound;
  const std::string& place;

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const
  {
    sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
    builder.reserve(val.size());
    for (const auto& v : val)
    {
      if (v >= bound)
      {
        throw bound_error(place);
      }
      builder.insert(builder.end(), v + valuation);
    }
    return std::move(builder);
  }

  friend
  bool
  operator==(const bounded_post& lhs, const bounded_post& rhs)
  noexcept
  {
    return lhs.valuation == rhs.valuation and lhs.bound == rhs.bound;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const bounded_post& p)
  {
    return os << "bounded_post(" << p.valuation << "," << p.bound <<")";
  }

};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

namespace std
{

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct hash<pnmc::mc::classic::bounded_post<C>>
{
  std::size_t
  operator()(const pnmc::mc::classic::bounded_post<C>& p)
  const noexcept
  {
    using namespace sdd::hash;
    return seed(98678683) (val(p.valuation)) (val(p.bound));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
