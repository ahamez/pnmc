#include "pn/module.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

module_node::module_node(const std::string& i)
  : nested(), id(i)
{}

/*------------------------------------------------------------------------------------------------*/

void
module_node::add_module(const module& m)
{
  nested.push_back(m);
}

/*------------------------------------------------------------------------------------------------*/

module
make_module(const place& p)
{
  return std::make_shared<boost::variant<module_node, const place*>>(&p);
}

/*------------------------------------------------------------------------------------------------*/

module
make_module(const module_node& m)
{
  return std::make_shared<boost::variant<module_node, const place*>>(m);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn
