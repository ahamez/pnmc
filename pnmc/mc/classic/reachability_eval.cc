#include <vector>

#include <boost/range/numeric.hpp>

#include "mc/classic/reachability_eval.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

using boost::accumulate;
using boost::apply_visitor;

namespace /* anonymous */ {

/*------------------------------------------------------------------------------------------------*/

struct integer_eval
{
  using result_type = int;
  const std::vector<pn::valuation_type>& values;

  result_type
  operator()(const integer_constant& e)
  const noexcept
  {
    return e.value;
  }

  result_type
  operator()(const integer_sum& e)
  const noexcept
  {
    return accumulate( e.expressions, 0
                     , [this](auto v, const auto& sub_e){return apply_visitor(*this, sub_e) + v;});
  }

  result_type
  operator()(const integer_product& e)
  const noexcept
  {
    return accumulate( e.expressions, 1
                     , [this](auto v, const auto& sub_e){return apply_visitor(*this, sub_e) * v;});
  }

  result_type
  operator()(const integer_difference& e)
  const noexcept
  {
    return apply_visitor(*this, e.lhs_expression) - apply_visitor(*this, e.rhs_expression);
  }

  result_type
  operator()(const integer_division& e)
  const noexcept
  {
    return apply_visitor(*this, e.lhs_expression) / apply_visitor(*this, e.rhs_expression);
  }

  result_type
  operator()(const integer_tokens&)
  const noexcept
  {
//    return values[e.pos];
    throw std::runtime_error(__PRETTY_FUNCTION__);
  }
};

/*------------------------------------------------------------------------------------------------*/

struct boolean_eval
{
  using result_type = bool;
  const integer_eval& integer;

  result_type
  operator()(const invariant&)
  const
  {
    throw std::runtime_error(__PRETTY_FUNCTION__);
  }

  result_type
  operator()(const possibility&)
  const
  {
    throw std::runtime_error(__PRETTY_FUNCTION__);
  }

  result_type
  operator()(const impossibility&)
  const
  {
    throw std::runtime_error(__PRETTY_FUNCTION__);
  }

  result_type
  operator()(const true_&)
  const
  {
    throw std::runtime_error(__PRETTY_FUNCTION__);
  }

  result_type
  operator()(const false_&)
  const
  {
    throw std::runtime_error(__PRETTY_FUNCTION__);
  }

  result_type
  operator()(const negation& e)
  const
  {
    return not apply_visitor(*this, e.expression);
  }

  result_type
  operator()(const conjunction& e)
  const
  {
    for (const auto& sub_e : e.expressions)
    {
      if (not apply_visitor(*this, sub_e))
      {
        return false;
      }
    }
    return true;
  }

  result_type
  operator()(const disjunction& e)
  const
  {
    for (const auto& sub_e : e.expressions)
    {
      if (apply_visitor(*this, sub_e))
      {
        return true;
      }
    }
    return false;
  }

  result_type
  operator()(const exclusive_disjonction& e)
  const
  {
    bool has_true = false;
    for (const auto& sub_e : e.expressions)
    {
      const auto tmp = apply_visitor(*this, sub_e);
      if      (tmp and not has_true) {has_true = true;}
      else if (tmp and has_true)     {return false;}
    }
    return has_true;
  }

  result_type
  operator()(const implication& e)
  const
  {
    return apply_visitor(*this, e.lhs_expression)
         ? apply_visitor(*this, e.rhs_expression)
         : true;
  }

  result_type
  operator()(const equivalence& e)
  const
  {
    return apply_visitor(*this, e.lhs_expression) == apply_visitor(*this, e.rhs_expression);
  }

  result_type
  operator()(const integer_eq& e)
  const noexcept
  {
    return apply_visitor(integer, e.lhs_expression) == apply_visitor(integer, e.rhs_expression);
  }

  result_type
  operator()(const integer_ne& e)
  const noexcept
  {
    return apply_visitor(integer, e.lhs_expression) != apply_visitor(integer, e.rhs_expression);
  }

  result_type
  operator()(const integer_lt& e)
  const noexcept
  {
    return apply_visitor(integer, e.lhs_expression) < apply_visitor(integer, e.rhs_expression);
  }

  result_type
  operator()(const integer_le& e)
  const noexcept
  {
    return apply_visitor(integer, e.lhs_expression) <= apply_visitor(integer, e.rhs_expression);
  }

  result_type
  operator()(const integer_gt& e)
  const noexcept
  {
    return apply_visitor(integer, e.lhs_expression) > apply_visitor(integer, e.rhs_expression);
  }

  result_type
  operator()(const integer_ge& e)
  const noexcept
  {
    return apply_visitor(integer, e.lhs_expression) >= apply_visitor(integer, e.rhs_expression);
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

int
eval(const integer_ast& a, const SDD&)
{
  if (const auto* ptr = boost::get<integer_constant>(&a))
  {
    return ptr->value;
  }
  std::vector<pn::valuation_type> values;
  return apply_visitor(integer_eval{values}, a);
}

/*------------------------------------------------------------------------------------------------*/

bool
eval(const boolean_ast& a, const SDD&)
{
  if (boost::get<true_>(&a))
  {
    return true;
  }
  else if (boost::get<false_>(&a))
  {
    return false;
  }
  std::vector<pn::valuation_type> values;
  integer_eval integer{values};
  return apply_visitor(boolean_eval{integer}, a);
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
