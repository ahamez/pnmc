#pragma once

#include <set>
#include <string>
#include <vector>

#include <boost/variant.hpp>

namespace pnmc { namespace properties {

/*------------------------------------------------------------------------------------------------*/

// Integer formulae :: Arithmetic operators
struct integer_constant;
struct integer_sum;
struct integer_product;
struct integer_difference;
struct integer_division;

// Integer formulae :: Petri net operators
struct place_bound
{
  std::vector<std::string> places;
};

struct tokens_count
{
  std::vector<std::string> places;
};

using integer_expression = boost::variant< integer_constant
                                         , boost::recursive_wrapper<integer_sum>
                                         , boost::recursive_wrapper<integer_product>
                                         , boost::recursive_wrapper<integer_difference>
                                         , boost::recursive_wrapper<integer_division>
                                         , place_bound
                                         , tokens_count>;

// Integer formulae :: Arithmetic operators
struct integer_constant
{
  int value;
};

struct integer_sum
{
  std::vector<integer_expression> expressions;
};

struct integer_product
{
  std::vector<integer_expression> expressions;
};

struct integer_difference
{
  integer_expression lhs_expression;
  integer_expression rhs_expression;
};

struct integer_division
{
  integer_expression lhs_expression;
  integer_expression rhs_expression;
};

/*------------------------------------------------------------------------------------------------*/

// Boolean formulae :: Reachability operators
struct invariant;
struct impossibility;
struct possibility;

// Boolean formulae :: Petri net operators
struct deadlock {};
struct is_fireable
{
  std::vector<std::string> transitions;
};

// Boolean formulae :: Boolean operators
struct true_ {};
struct false_ {};
struct negation;
struct conjunction;
struct disjunction;
struct exclusive_disjonction;
struct implication;
struct equivalence;

// Boolean formulae :: Comparison operators
struct integer_eq;
struct integer_ne;
struct integer_lt;
struct integer_le;
struct integer_gt;
struct integer_ge;

using boolean_expression = boost::variant< boost::recursive_wrapper<invariant>
                                         , boost::recursive_wrapper<impossibility>
                                         , boost::recursive_wrapper<possibility>
                                         , deadlock
                                         , is_fireable
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

//// Boolean formulae :: Reachability operators
struct invariant
{
  boolean_expression expression;
};

struct impossibility
{
  boolean_expression expression;
};

struct possibility
{
  boolean_expression expression;
};

// Boolean formulae :: Boolean operators
struct negation
{
  boolean_expression expression;
};

struct conjunction
{
  std::vector<boolean_expression> expressions;
};

struct disjunction
{
  std::vector<boolean_expression> expressions;
};

struct exclusive_disjonction
{
  std::vector<boolean_expression> expressions;
};

struct implication
{
  boolean_expression lhs_expression;
  boolean_expression rhs_expression;
};

struct equivalence
{
  boolean_expression lhs_expression;
  boolean_expression rhs_expression;
};

// Boolean formulae :: Comparison operators
struct integer_eq
{
  integer_expression lhs_expression;
  integer_expression rhs_expression;
};

struct integer_ne
{
  integer_expression lhs_expression;
  integer_expression rhs_expression;
};

struct integer_lt
{
  integer_expression lhs_expression;
  integer_expression rhs_expression;
};

struct integer_le
{
  integer_expression lhs_expression;
  integer_expression rhs_expression;
};

struct integer_gt
{
  integer_expression lhs_expression;
  integer_expression rhs_expression;
};

struct integer_ge
{
  integer_expression lhs_expression;
  integer_expression rhs_expression;
};

/*------------------------------------------------------------------------------------------------*/

using formula = boost::variant<boolean_expression, integer_expression>;

/*------------------------------------------------------------------------------------------------*/

struct formulae
{
  bool compute_deadlock;
  std::set<std::string> places_bounds;
  std::set<std::string> fireable_transitions;

  struct boolean
  {
    std::string id;
    std::set<std::string> targets;
    boolean_expression expression;
  };

  struct integer
  {
    std::string id;
    std::set<std::string> targets;
    integer_expression expression;
  };

  std::vector<boolean> booleans;
  std::vector<integer> integers;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::properties
