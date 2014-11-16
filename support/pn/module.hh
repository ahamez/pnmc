#pragma once

#include <functional> // reference_wrapper
#include <string>
#include <vector>

#include <boost/variant.hpp>

#include "support/pn/place.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

class module;

/*------------------------------------------------------------------------------------------------*/

class module_node
{
public:

  using modules_list = std::vector<module>;
  using places_list  = std::vector<std::reference_wrapper<const place>>;
  using nodes_list   = std::vector<std::reference_wrapper<const module_node>>;

private:

  std::string id_;
  modules_list all_;
  places_list places_;
  nodes_list nodes_;

public:

  module_node(const std::string& id)
    : id_{id}, all_{}, places_{}, nodes_{}
  {}

  const std::string&  id()     const noexcept {return id_;}
  const modules_list& all()    const noexcept {return all_;}
  const places_list&  places() const noexcept {return places_;}
  const nodes_list&   nodes()  const noexcept {return nodes_;}

  void
  add_submodule(const module&);
};

/*------------------------------------------------------------------------------------------------*/

using module_variant = boost::variant<const module_node, std::reference_wrapper<const place>>;

/*------------------------------------------------------------------------------------------------*/

class module
{
private:

  std::shared_ptr<const module_variant> ptr_;

public:

  module(const std::shared_ptr<module_variant>& ptr)
    : ptr_{ptr}
  {}

  const module_variant& variant() const noexcept {return *ptr_;}
};

/*------------------------------------------------------------------------------------------------*/

inline
module
make_module(const place& p)
{
  return {std::make_shared<module_variant>(p)};
}

/*------------------------------------------------------------------------------------------------*/

inline
module
make_module(const module_node& m)
{
  return {std::make_shared<module_variant>(m)};
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
