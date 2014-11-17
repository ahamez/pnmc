#pragma once

#include <map>

namespace pnmc { namespace conf {

/*------------------------------------------------------------------------------------------------*/

static constexpr auto min_cache_size = 100'000ul;
static constexpr auto min_ut_size    =   1'000ul;

/*------------------------------------------------------------------------------------------------*/

// Caches size
static const auto default_cache_sizes = std::map<std::string, std::size_t>
  { {"hom"  , 8'000'000}
  , {"diff" , 2'000'000}
  , {"inter", 2'000'000}
  , {"sum"  , 2'000'000}};

/*------------------------------------------------------------------------------------------------*/

// Caches size
static const auto default_ut_sizes = std::map<std::string, std::size_t>
  { {"hom"   , 4'000}
  , {"sdd"   , 2'000'000}
  , {"values", 1'000}};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::conf
