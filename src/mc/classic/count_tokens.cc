#include <algorithm> // max, max_element
#include <cassert>
#include <unordered_map>
#include <utility>   // pair
#include <tuple>

#include "mc/classic/count_tokens.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

//namespace /* anonymous */{

/// @internal
struct count_tokens_visitor
{
  /// @brief Required by mem::variant visitor mechanism.
  using result_type = std::pair<unsigned long, unsigned long>;

  /// @brief A cache is necessary to to know if a node has already been encountered.
  ///
  /// We use the addresses of nodes as key. It's legit because nodes are unified and immutable.
  mutable std::unordered_map<const char*, result_type> cache_;

  /// @The results to update.
//  results& res;

//  /// @brief Constructor.
//  count_tokens_visitor(results& r)
//    : res(r)
//  {}

  /// @brief |0|.
  result_type
  operator()( const sdd::zero_terminal<sdd::conf1>&
            , std::shared_ptr<sdd::dd::sdd_stack<sdd::conf1>>)
  const
  {
    assert(false);
    __builtin_unreachable();
  }

  /// @brief |1|.
  result_type
  operator()( const sdd::one_terminal<sdd::conf1>&
            , std::shared_ptr<sdd::dd::sdd_stack<sdd::conf1>> stack)
  const
  {
    if (stack)
    {
      return visit(*this, stack->sdd, stack->next);
    }
    else
    {
//      res.max_token_markings = std::max(res.max_token_markings, nb_marking_tokens);
      return {0ul, 0ul};
    }
  }

  /// @brief Flat SDD.
  result_type
  operator()( const sdd::flat_node<sdd::conf1>& n
            , std::shared_ptr<sdd::dd::sdd_stack<sdd::conf1>> stack)
  const
  {
    auto insertion = cache_.emplace(reinterpret_cast<const char*>(&n), std::make_pair(0, 0));
    if (insertion.second)
    {
      unsigned long markings = 0;
      unsigned long places = 0;

      for (const auto& arc : n)
      {
        const unsigned long max_value
          = *std::max_element(arc.valuation().cbegin(), arc.valuation().cend());
        const auto res = visit(*this, arc.successor(), stack);

        markings = std::max(markings, max_value + res.first);
        places = std::max(max_value, res.second);
      }

      insertion.first->second = std::make_pair(markings, places);
    }
    return insertion.first->second;
  }

  /// @brief Hierarchical SDD.
  result_type
  operator()( const sdd::hierarchical_node<sdd::conf1>& n
            , std::shared_ptr<sdd::dd::sdd_stack<sdd::conf1>> stack)
  const
  {
    auto insertion = cache_.emplace(reinterpret_cast<const char*>(&n), std::make_pair(0, 0));
    if (insertion.second)
    {
      unsigned long markings = 0;
      unsigned long places = 0;

      for (const auto& arc : n)
      {
        const auto local_stack
          = std::make_shared<sdd::dd::sdd_stack<sdd::conf1>>(arc.successor(), stack);
        const auto res = visit(*this, arc.valuation(), local_stack);

        markings = std::max(markings, res.first);
        places = std::max(places, res.second);
      }

      insertion.first->second = std::make_pair(markings, places);
    }
    return insertion.first->second;
  }
}; // struct count_tokens_visitor

//} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

void
count_tokens(results& res, const sdd::SDD<sdd::conf1>& state_space)
{
  const auto pair = visit(count_tokens_visitor(), state_space, nullptr);
  std::tie(res.max_token_markings, res.max_token_places) = pair;
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
