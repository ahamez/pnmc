#pragma once

#include <deque>
#include <iosfwd>
#include <string>
#include <utility>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/optional.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <sdd/sdd.hh>
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
  boost::optional<std::deque<std::pair<std::string, sdd::SDD<C>>>> trace;

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

    // Get the identifier of each level (SDD::paths() doesn't give this information).
    std::deque<std::reference_wrapper<const std::string>> identifiers;
    r.order->flat(std::back_inserter(identifiers));

    const auto display_states = [&](const auto& x, auto limit, const auto& prefix)
    {
      auto path_generator = x.paths();
      while (path_generator and limit-- > 0)
      {
        const auto& path = path_generator.get();
        path_generator(); // advance generator
        auto id_cit = begin(identifiers);
        auto path_cit = begin(path);
        os << prefix;
        for (; path_cit != path.cend(); ++path_cit, ++id_cit)
        {
          auto copy = *path_cit;
          copy.erase(0);
          if (not copy.empty())
          {
            os << id_cit->get() << '*' << copy << ' ';
          }
        }
        os << '\n';
      }
    };

    if (r.dead_states)
    {
      assert(r.order);
      const auto sz = r.dead_states->size().template convert_to<long double>();
      if (not r.dead_states->empty())
      {
        // Limit the number of displayed dead states.
        auto max = std::min(static_cast<long double>(conf::max_shown_states), sz);
        if (max < sz)
        {
          os << max << " first dead state(s):\n";
        }
        else
        {
          os << sz << " dead state(s):\n";
        }
        display_states(*r.dead_states, max, "  ");
      }
    }
    if (r.trace)
    {
//      os << (r.trace->size() - 1) << " step(s) to error:\n";
//      for (auto cit = std::next(cbegin(*r.trace)); cit != end(*r.trace); ++cit)
//      {
//        os << "  " << cit->first << '\n';
//      }
      os << (r.trace->size()) << " step(s) to error:\n";
      for (auto cit = cbegin(*r.trace); cit != end(*r.trace); ++cit)
      {
        os << "  " << cit->first << '\n';
      }

    }
    return os;
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::shared
