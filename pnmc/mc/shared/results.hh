#pragma once

#include <deque>
#include <iosfwd>
#include <string>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/optional.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <sdd/tools/size.hh>

#include "conf/default.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace mc { namespace shared {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct results
{
  boost::optional<sdd::order<C>> order;
  boost::optional<sdd::SDD<C>> m0;
  boost::optional<sdd::SDD<C>> states;
  boost::optional<pn::valuation_type> max_token_markings;
  boost::optional<pn::valuation_type> max_token_places;
  boost::optional<std::deque<std::string>> dead_transitions;
  boost::optional<sdd::SDD<C>> dead_states;

  friend
  std::ostream&
  operator<<(std::ostream& os, const results& r)
  {
    if (r.states)
    {
      os << r.states->size().template convert_to<long double>() << " state(s)\n";
    }
    if (r.max_token_markings)
    {
      os << "maximal number of tokens per marking : " << *r.max_token_markings << '\n';
    }
    if (r.max_token_places)
    {
      os << "maximal number of tokens in a place : " << *r.max_token_places << '\n';
    }
    if (r.dead_transitions)
    {
      os << r.dead_transitions->size() << " dead transition(s)\n";
      if (not r.dead_transitions->empty())
      {
        os << "  ";
        boost::range::copy(*r.dead_transitions, std::ostream_iterator<std::string>(os, " "));
        os << '\n';
      }
    }
    if (r.dead_states)
    {
      assert(r.order);
      const auto sz = r.dead_states->size().template convert_to<long double>();
      os << sz << " dead state(s)\n";
      if (not r.dead_states->empty())
      {
        // Limit the number of displayed dead states.
        auto max = std::min(static_cast<long double>(conf::max_shown_states), sz);
        if (max < sz)
        {
          os << "  " << max << " first dead state(s):\n";
        }

        // Get the identifier of each level (SDD::paths() doesn't give this information).
        std::deque<std::reference_wrapper<const std::string>> identifiers;
        r.order->flat(std::back_inserter(identifiers));

        // We can't use the range-based for loop as it produces an ambiguity with clang when
        // using Boost 1.56.
        auto path_generator = r.dead_states->paths();
        while (path_generator and max-- > 0)
        {
          const auto& path = path_generator.get();
          path_generator(); // advance generator
          auto id_cit = begin(identifiers);
          auto path_cit = begin(path);
          for (; path_cit != std::prev(path.cend()); ++path_cit, ++id_cit)
          {
            os << "  " << id_cit->get() << " : " << *path_cit << ", ";
          }
          os << "  " << id_cit->get() << " : " << *path_cit << '\n';
        }
      }
    }
    return os;
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::shared
