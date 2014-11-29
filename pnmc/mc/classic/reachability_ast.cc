#include <algorithm>
#include <functional>

#include <boost/range/numeric.hpp>

#include "mc/classic/reachability_ast.hh"

namespace pnmc { namespace mc { namespace classic {

using boost::apply_visitor;
using boost::accumulate;

namespace /* anonymous */ {

/*------------------------------------------------------------------------------------------------*/

template <typename T>
struct is
  : boost::static_visitor<bool>
{
                            bool operator()(const T&)     const noexcept {return true;}
  template <typename Other> bool operator()(const Other&) const noexcept {return false;}
};

/*------------------------------------------------------------------------------------------------*/

bool
is_constant(const integer_ast& ast)
noexcept
{
  return apply_visitor(is<integer_constant>{}, ast);
}

/*------------------------------------------------------------------------------------------------*/

struct integer_ast_builder
{
  using result_type = integer_ast;

  const std::unordered_map<std::string /* place id */, pn::valuation_type>& bounds;

  integer_ast_builder(const std::unordered_map<std::string, pn::valuation_type>& b)
    : bounds(b)
  {}

  result_type
  operator()(const properties::integer_constant& e)
  const noexcept
  {
    return integer_constant{e.value};
  }

  template <typename Result>
  struct nary
  {
    const integer_ast_builder& builder;

    template <typename Expression, typename Fn>
    result_type
    operator()(const Expression& e, Fn&& fn)
    const
    {
      /// @todo Merge consecutive sums or products

      auto sub = std::vector<integer_ast>{};
      sub.reserve(e.expressions.size());
      for (const auto& sub_e : e.expressions)
      {
        sub.emplace_back(apply_visitor(builder, sub_e));
      }
      const auto point = std::partition( begin(sub), end(sub)
                                       , [](const auto& s){return not is_constant(s);});
      const auto value = [&]
      {
        int res = 0;
        for (auto cit = point; cit != end(sub); ++cit)
        {
          res = fn(res, boost::get<integer_constant>(*cit).value);
        }
        return res;
      }();
      if (point == end(sub)) // no constants
      {
        return Result{sub};
      }
      else if (point == begin(sub)) // only constants
      {
        return integer_constant{value};
      }
      else
      {
        sub.erase(point, end(sub)); // remove all constants
        sub.emplace_back(integer_constant{value});
        return Result{sub};
      }
    }
  };

  result_type
  operator()(const properties::integer_sum& e)
  const
  {
    return nary<integer_sum>{*this}(e, std::plus<int>{});
  }

  result_type
  operator()(const properties::integer_product& e)
  const
  {
    return nary<integer_product>{*this}(e, std::multiplies<int>{});
  }

  template <typename Result>
  struct binary
  {
    const integer_ast_builder& builder;

    template <typename Expression, typename Fn>
    result_type
    operator()(const Expression& e, Fn&& fn)
    const
    {
      const auto lhs = apply_visitor(builder, e.lhs_expression);
      const auto rhs = apply_visitor(builder, e.rhs_expression);
      if (is_constant(lhs) and is_constant(rhs))
      {
        const auto value = fn( boost::get<integer_constant>(lhs).value
                             , boost::get<integer_constant>(rhs).value);
        return integer_constant{value};
      }
      else
      {
        return Result{lhs, rhs};
      }
    }
  };

  result_type
  operator()(const properties::integer_difference& e)
  const noexcept
  {
    return binary<integer_difference>{*this}(e, std::minus<int>{});
  }

  result_type
  operator()(const properties::integer_division& e)
  const noexcept
  {
    return binary<integer_division>{*this}(e, std::divides<int>{});
  }

  result_type
  operator()(const properties::place_bound& e)
  const
  {
    const auto sum = accumulate(e.places, 0, [&](auto x, const auto& p){return x + bounds.at(p);});
    return integer_constant{sum};
  }

  result_type
  operator()(const properties::tokens_count&)
  const noexcept
  {
    throw std::runtime_error(__PRETTY_FUNCTION__);
  }
};

/*------------------------------------------------------------------------------------------------*/

bool
is_true(const boolean_ast& ast)
noexcept
{
  return apply_visitor(is<true_>{}, ast);
}

/*------------------------------------------------------------------------------------------------*/

bool
is_false(const boolean_ast& ast)
noexcept
{
  return apply_visitor(is<false_>{}, ast);
}

/*------------------------------------------------------------------------------------------------*/

struct boolean_ast_builder
{
  using result_type = boolean_ast;
  const bool has_deadlock;
  const std::set<std::string>& dead_transitions;
  const integer_ast_builder& integer_builder;

  boolean_ast_builder( bool deadlock, const std::set<std::string>& dead
                     , const integer_ast_builder& builder)
    : has_deadlock(deadlock), dead_transitions(dead), integer_builder(builder)
  {}

  result_type
  operator()(const properties::invariant&)
  const
  {
    throw std::runtime_error(__PRETTY_FUNCTION__);
  }

  result_type
  operator()(const properties::impossibility& e)
  const
  {
    const auto rec = apply_visitor(*this, e.expression);
    if (is_true(rec))       {return false_{};}
    else if (is_false(rec)) {return true_{};}
    else                    {return impossibility{rec};}
  }

  result_type
  operator()(const properties::possibility& e)
  const
  {
    const auto rec = apply_visitor(*this, e.expression);
    if      (is_true(rec))  {return true_{};}
    else if (is_false(rec)) {return false_{};}
    else                    {return invariant{rec};}
  }

  result_type
  operator()(const properties::deadlock&)
  const noexcept
  {
    return has_deadlock ? result_type{true_{}} : result_type{false_{}};
  }

  result_type
  operator()(const properties::is_fireable& e)
  const noexcept
  {
    for (const auto& t : e.transitions)
    {
      if (dead_transitions.count(t)) {return false_{};}
    }
    return true_{};
  }

  result_type
  operator()(const properties::true_&)
  const noexcept
  {
    return true_{};
  }

  result_type
  operator()(const properties::false_&)
  const noexcept
  {
    return false_{};
  }

  result_type
  operator()(const properties::negation& e)
  const
  {
    const auto rec = apply_visitor(*this, e.expression);
    if      (is_true(rec))  {return false_{};}
    else if (is_false(rec)) {return true_{};}
    else                    {return negation{rec};}
  }

  result_type
  operator()(const properties::conjunction& e)
  const
  {
    auto rec = std::vector<boolean_ast>{};
    rec.reserve(e.expressions.size());
    for (const auto& sub_e : e.expressions)
    {
      auto tmp = apply_visitor(*this, sub_e);
      if      (is_false(tmp)) {return false_{};}
      else if (is_true(tmp))  {continue;}
      rec.emplace_back(std::move(tmp));
    }
    if  (rec.empty()) {return true_{};}
    else              {return conjunction{rec};}
  }

  result_type
  operator()(const properties::disjunction& e)
  const
  {
    auto rec = std::vector<boolean_ast>{};
    rec.reserve(e.expressions.size());
    for (const auto& sub_e : e.expressions)
    {
      auto tmp = apply_visitor(*this, sub_e);
      if      (is_true(tmp))  {return true_{};}
      else if (is_false(tmp)) {continue;}
      rec.emplace_back(std::move(tmp));
    }
    if   (rec.empty()) {return false_{};}
    else               {return disjunction{rec};}
  }

  result_type
  operator()(const properties::exclusive_disjonction& e)
  const
  {
    auto rec = std::vector<boolean_ast>{};
    rec.reserve(e.expressions.size());
    auto has_true = false;
    for (const auto& sub_e : e.expressions)
    {
      auto tmp = apply_visitor(*this, sub_e);
      if      (is_true(tmp) and not has_true) {has_true = true;}
      else if (is_true(tmp) and has_true)     {return false_{};}
      else if (is_false(tmp))                 {continue;}
      rec.emplace_back(std::move(tmp));
    }
    if   (rec.empty()) {return false_{};}
    else               {return exclusive_disjonction{rec};}
  }

  result_type
  operator()(const properties::implication& e)
  const
  {
    const auto lhs = apply_visitor(*this, e.lhs_expression);
    if (is_false(lhs))
    {
      return true_{};
    }
    const auto rhs = apply_visitor(*this, e.lhs_expression);
    if (is_true(lhs))
    {
      return rhs;
    }
    return implication{lhs, rhs};
  }

  result_type
  operator()(const properties::equivalence& e)
  const
  {
    const auto lhs = apply_visitor(*this, e.lhs_expression);
    const auto rhs = apply_visitor(*this, e.rhs_expression);
    if (is_true(lhs))
    {
      if      (is_true(rhs))  {return true_{};}
      else if (is_false(rhs)) {return false_{};}
      else                    {return rhs;}
    }
    else if (is_false(lhs))
    {
      if      (is_true(rhs))  {return false_{};}
      else if (is_false(rhs)) {return true_{};}
      else                    {return negation{rhs};}
    }
    else if (is_true(rhs))
    {
      if      (is_true(lhs))  {return true_{};}
      else if (is_false(lhs)) {return false_{};}
      else                    {return lhs;}
    }
    else if (is_false(rhs))
    {
      if      (is_true(lhs))  {return false_{};}
      else if (is_false(lhs)) {return true_{};}
      else                    {return negation{lhs};}
    }
    else
    {
      return equivalence{lhs, rhs};
    }
  }

  template <typename Result>
  struct integer_cmp
  {
    const integer_ast_builder& builder;

    template <typename Expression, typename Fn>
    result_type
    operator()(const Expression& e, Fn&& fn)
    const
    {
      const auto lhs = apply_visitor(builder, e.lhs_expression);
      const auto rhs = apply_visitor(builder, e.rhs_expression);
      if (is_constant(lhs) and is_constant(rhs))
      {
        return fn(boost::get<integer_constant>(lhs).value, boost::get<integer_constant>(rhs).value)
             ? result_type{true_{}}
             : result_type{false_{}};
      }
      else
      {
        return Result{lhs, rhs};
      }
    }
  };

  result_type
  operator()(const properties::integer_eq& e)
  const
  {
    return integer_cmp<integer_eq>{integer_builder}(e, std::equal_to<int>{});
  }

  result_type
  operator()(const properties::integer_ne& e)
  const
  {
    return integer_cmp<integer_ne>{integer_builder}(e, std::not_equal_to<int>{});
  }

  result_type
  operator()(const properties::integer_lt& e)
  const
  {
    return integer_cmp<integer_lt>{integer_builder}(e, std::less<int>{});
  }

  result_type
  operator()(const properties::integer_le& e)
  const
  {
    return integer_cmp<integer_le>{integer_builder}(e, std::less_equal<int>{});
  }

  result_type
  operator()(const properties::integer_gt& e)
  const
  {
    return integer_cmp<integer_gt>{integer_builder}(e, std::greater<int>{});
  }

  result_type
  operator()(const properties::integer_ge& e)
  const
  {
    return integer_cmp<integer_ge>{integer_builder}(e, std::greater_equal<int>{});
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

integer_ast
make_ast( const properties::integer_expression& e
        , const std::unordered_map<std::string, pn::valuation_type>& bounds)
{
  return apply_visitor(integer_ast_builder{bounds}, e);
}

/*------------------------------------------------------------------------------------------------*/

boolean_ast
make_ast( const properties::boolean_expression& e,  bool has_deadlock
        , const std::set<std::string>& dead_transitions
        , const std::unordered_map<std::string, pn::valuation_type>& bounds)
{
  const auto integer = integer_ast_builder{bounds};
  return apply_visitor(boolean_ast_builder{has_deadlock, dead_transitions, integer}, e);
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
