#include <algorithm> // max_element
#include <cassert>

#include <boost/container/flat_set.hpp>

#include "mc/classic/count_tokens.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

void
count_tokens(results& res, const sdd::SDD<sdd::conf1>& state_space)
{
  boost::container::flat_set<unsigned long> token_sums1;
  boost::container::flat_set<unsigned long> token_sums2;
  token_sums1.reserve(1024);
  token_sums2.reserve(1024);

  for (const auto& path : state_space.paths())
  {
    for (const auto& values : path)
    {
      if (token_sums1.empty()) // First values in path
      {
        for (const auto value : values)
        {
          res.max_token_places = std::max(res.max_token_places, static_cast<unsigned long>(value));
          token_sums1.emplace(value);
        }
      }
      else
      {
        for (const auto value : values)
        {
          res.max_token_places = std::max(res.max_token_places, static_cast<unsigned long>(value));
          for (const auto sum : token_sums1)
          {
            token_sums2.emplace(sum + value);
          }
          token_sums1.swap(token_sums2);
        }
      }
    }
    const auto max_sum = *std::max_element(token_sums1.cbegin(), token_sums1.cend());
    res.max_token_markings = std::max(res.max_token_markings, max_sum);

    token_sums1.clear();
    token_sums2.clear();
  }
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
