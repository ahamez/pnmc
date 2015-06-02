/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <string>
#include <set>
#include <unordered_map>
#include <vector>

#include <boost/variant.hpp>

#include "support/pn/types.hh"
#include "support/properties/formulae.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

struct integer_constant
{
  int value;
};
struct integer_sum;
struct integer_product;
struct integer_difference;
struct integer_division;
struct integer_tokens
{
  std::size_t pos;
};

using integer_ast = boost::variant< integer_constant
                                  , boost::recursive_wrapper<integer_sum>
                                  , boost::recursive_wrapper<integer_product>
                                  , boost::recursive_wrapper<integer_difference>
                                  , boost::recursive_wrapper<integer_division>
                                  , integer_tokens>;

struct integer_sum
{
  std::vector<integer_ast> expressions;
};

struct integer_product
{
  std::vector<integer_ast> expressions;
};

struct integer_difference
{
  integer_ast lhs_expression;
  integer_ast rhs_expression;
};

struct integer_division
{
  integer_ast lhs_expression;
  integer_ast rhs_expression;
};

/*------------------------------------------------------------------------------------------------*/

struct invariant;
struct impossibility;
struct possibility;

struct true_ {};
struct false_ {};
struct negation;
struct conjunction;
struct disjunction;
struct exclusive_disjonction;
struct implication;
struct equivalence;

struct integer_eq;
struct integer_ne;
struct integer_lt;
struct integer_le;
struct integer_gt;
struct integer_ge;

using boolean_ast = boost::variant< boost::recursive_wrapper<invariant>
                                  , boost::recursive_wrapper<impossibility>
                                  , boost::recursive_wrapper<possibility>
                                  , true_
                                  , false_
                                  , boost::recursive_wrapper<negation>
                                  , boost::recursive_wrapper<conjunction>
                                  , boost::recursive_wrapper<disjunction>
                                  , boost::recursive_wrapper<exclusive_disjonction>
                                  , boost::recursive_wrapper<implication>
                                  , boost::recursive_wrapper<equivalence>
                                  , boost::recursive_wrapper<integer_eq>
                                  , boost::recursive_wrapper<integer_ne>
                                  , boost::recursive_wrapper<integer_lt>
                                  , boost::recursive_wrapper<integer_le>
                                  , boost::recursive_wrapper<integer_gt>
                                  , boost::recursive_wrapper<integer_ge>>;

struct invariant
{
  boolean_ast expression;
};

struct impossibility
{
  boolean_ast expression;
};

struct possibility
{
  boolean_ast expression;
};

struct negation
{
  boolean_ast expression;
};

struct conjunction
{
  std::vector<boolean_ast> expressions;
};

struct disjunction
{
  std::vector<boolean_ast> expressions;
};

struct exclusive_disjonction
{
  std::vector<boolean_ast> expressions;
};

struct implication
{
  boolean_ast lhs_expression;
  boolean_ast rhs_expression;
};

struct equivalence
{
  boolean_ast lhs_expression;
  boolean_ast rhs_expression;
};

struct integer_eq
{
  integer_ast lhs_expression;
  integer_ast rhs_expression;
};

struct integer_ne
{
  integer_ast lhs_expression;
  integer_ast rhs_expression;
};

struct integer_lt
{
  integer_ast lhs_expression;
  integer_ast rhs_expression;
};

struct integer_le
{
  integer_ast lhs_expression;
  integer_ast rhs_expression;
};

struct integer_gt
{
  integer_ast lhs_expression;
  integer_ast rhs_expression;
};

struct integer_ge
{
  integer_ast lhs_expression;
  integer_ast rhs_expression;
};

/*------------------------------------------------------------------------------------------------*/

integer_ast
make_ast( const properties::integer_expression&
        , const std::unordered_map<std::string, pn::valuation_type>& bounds);

/*------------------------------------------------------------------------------------------------*/

boolean_ast
make_ast( const properties::boolean_expression&, bool has_deadlock
        , const std::set<std::string>& dead_transitions
        , const std::unordered_map<std::string, pn::valuation_type>& bounds);

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic
