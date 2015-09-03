/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2013-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <cereal/archives/json.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/set.hpp>

#include "conf/configuration.hh"
#include "mc/shared/results.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace mc { namespace shared {

/*------------------------------------------------------------------------------------------------*/

struct place_values
{
  std::string place;
  std::vector<pn::valuation_type> values;

  place_values(std::string  p, std::vector<pn::valuation_type>&& v)
    : place{std::move(p)}, values{std::move(v)}
  {}
};

template <typename Archive>
void
save(Archive& archive, const place_values& p)
{
  archive( cereal::make_nvp("place", p.place)
         , cereal::make_nvp("values", p.values));
}

/*------------------------------------------------------------------------------------------------*/

struct transition_state
{
  std::string transition;
  std::deque<place_values> state;
};

template <typename Archive>
void
save(Archive& archive, const transition_state& t)
{
  archive( cereal::make_nvp("transition", t.transition)
         , cereal::make_nvp("state", t.state));
}

/*------------------------------------------------------------------------------------------------*/

struct property_result
{
  std::string id;
  bool result;
};

template <typename Archive>
void
save(Archive& archive, const property_result& p)
{
  archive( cereal::make_nvp("property", p.id)
         , cereal::make_nvp("result", p.result));
}

/*------------------------------------------------------------------------------------------------*/

struct evaluation_result
{
  std::string id;
  int result;
};

template <typename Archive>
void
save(Archive& archive, const evaluation_result& e)
{
  archive( cereal::make_nvp("evaluation", e.id)
         , cereal::make_nvp("result", e.result));
}

/*------------------------------------------------------------------------------------------------*/

template <typename Archive, typename C>
void
save(Archive& archive, const results<C>& r)
{
  if (r.states)
  {
    archive(cereal::make_nvp("states", r.states->size().template convert_to<long double>()));
  }
  if (r.max_token_markings)
  {
    archive(cereal::make_nvp("max number of tokens per marking", *r.max_token_markings));
  }
  if (r.max_token_places)
  {
    archive(cereal::make_nvp("max number of tokens in a place", *r.max_token_places));
  }
  if (r.dead_transitions)
  {
    archive(cereal::make_nvp("dead transitions", *r.dead_transitions));
  }

  // Get the identifier of each level (SDD::paths() doesn't give this information).
  std::deque<std::reference_wrapper<const std::string>> identifiers;
  r.order->flat(std::back_inserter(identifiers));

  if (r.dead_states)
  {
    std::deque<std::deque<place_values>> deads;

    auto path_generator = r.dead_states->paths();
    while (path_generator)
    {
      deads.emplace_back();
      const auto& path = path_generator.get();
      path_generator(); // advance generator
      auto id_cit = begin(identifiers);
      auto path_cit = begin(path);
      for (; path_cit != end(path); ++path_cit, ++id_cit)
      {
        std::vector<pn::valuation_type> values{path_cit->cbegin(), path_cit->cend()};
        deads.back().emplace_back(id_cit->get(), std::move(values));
      }
    }
    archive(cereal::make_nvp("dead states", deads));
  }

  if (r.trace)
  {
    std::deque<transition_state> trace;
    for (const auto& transition_sdd : *r.trace)
    {
      auto id_cit = begin(identifiers);
      auto path_generator = transition_sdd.second.paths();
      const auto& path = path_generator.get(); // get only first path

      trace.emplace_back();
      trace.back().transition = transition_sdd.first;
      for (const auto& values : path)
      {
        auto vec = std::vector<pn::valuation_type>(values.cbegin(), values.cend());
        trace.back().state.emplace_back(*id_cit++, std::move(vec));
      }
    }
    archive(cereal::make_nvp("trace", trace));
  }

  if (r.reachability)
  {
    std::deque<property_result> results;
    for (const auto& kv : *r.reachability)
    {
      results.emplace_back(property_result{kv.first, kv.second});
    }
    archive(cereal::make_nvp("properties", results));
  }

  if (r.evaluations)
  {
    std::deque<evaluation_result> results;
    for (const auto& kv : *r.evaluations)
    {
      results.emplace_back(evaluation_result{kv.first, kv.second});
    }
    archive(cereal::make_nvp("evaluations", results));

  }
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::shared
