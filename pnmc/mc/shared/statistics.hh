/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <chrono>
#include <deque>

#include <boost/optional.hpp>

#include <sdd/tools/manager_statistics.hh>
#include <sdd/tools/sdd_statistics.hh>

#include "support/pn/statistics.hh"

namespace pnmc { namespace mc { namespace shared {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct statistics
{
  bool interrupted;

  boost::optional<std::chrono::duration<double>> max_time;

  std::chrono::duration<double> total_duration;
  std::chrono::duration<double> relation_duration;
  std::chrono::duration<double> rewrite_duration;
  std::chrono::duration<double> state_space_duration;

  boost::optional<std::chrono::duration<double>> tokens_duration;
  boost::optional<std::chrono::duration<double>> force_duration;
  boost::optional<std::chrono::duration<double>> dead_states_duration;
  boost::optional<std::chrono::duration<double>> trace_duration;
  boost::optional<std::chrono::duration<double>> reachability_duration;

  boost::optional<std::deque<unsigned int>> sdd_ut_size;
  boost::optional<std::deque<double>> force_spans;
  boost::optional<pn::statistics> pn_statistics;

  boost::optional<sdd::tools::manager_statistics<C>> manager_statistics;
  boost::optional<sdd::tools::sdd_statistics<C>> sdd_statistics;
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::shared
