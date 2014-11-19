#pragma once

#include <cereal/archives/json.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/vector.hpp>

#include "conf/configuration.hh"
#include "mc/shared/results.hh"
#include "support/pn/types.hh"

namespace pnmc { namespace mc { namespace shared {

/*------------------------------------------------------------------------------------------------*/

struct place_values
{
  std::string place;
  std::vector<pn::valuation_type> values;

  place_values(const std::string& p, std::vector<pn::valuation_type>&& v)
    : place{p}, values{std::move(v)}
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
    archive(cereal::make_nvp("trace", "TODO"));
  }
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::shared
