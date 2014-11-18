#pragma once

#include <map>

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

// Caches size
static constexpr auto min_cache_size = 100'000ul;
static const auto default_cache_sizes = std::map<std::string, std::size_t>
  { {"hom"   , 8'000'000}
  , {"diff"  , 2'000'000}
  , {"inter" , 2'000'000}
  , {"sum"   , 2'000'000}};

/*------------------------------------------------------------------------------------------------*/

// Unicity tables size
static constexpr auto min_ut_size    =   1'000ul;
static const auto default_ut_sizes = std::map<std::string, std::size_t>
  { {"hom"    , 4'000}
  , {"sdd"    , 2'000'000}
  , {"values" , 1'000}};

/*------------------------------------------------------------------------------------------------*/

enum class filename { dot_m0, dot_final, dot_h, dot_h_opt, dot_force
                    , json_results, json_h, json_h_opt, json_stats, json_order};

// Output files names
static const auto default_filenames = std::map<filename, std::string>
  { {filename::dot_m0          , "initial_state.dot"}
  , {filename::dot_final       , "state_space.dot"}
  , {filename::dot_h           , "relation_transition.dot"}
  , {filename::dot_h_opt       , "relation_transition_opt.dot"}
  , {filename::dot_force       , "force.dot"}
  , {filename::json_results    , "results.json"}
  , {filename::json_h          , "relation_transition.json"}
  , {filename::json_h_opt      , "relation_transition_opt.json"}
  , {filename::json_stats      , "stats.json"}
  , {filename::json_order      , "order.json"}
  };

/*------------------------------------------------------------------------------------------------*/

static constexpr auto max_shown_states = 10u;

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
