#include <algorithm> // std::equal
#include <iostream>
#include <list>
#include <sstream>
#include <unordered_map>

#include "parsers/helpers.hh"
#include "parsers/parse_error.hh"
#include "parsers/prod.hh"

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

namespace {

unsigned int
marking(std::string::const_iterator cit, std::string::const_iterator cend)
{
  const auto distance = std::distance(cit, cend);
  if (distance == 4)
  {
    static const std::string mk("<..>");
    if (std::equal(cit, cend, mk.cbegin()))
    {
      return 1;
    }
    else
    {
      throw parse_error("Invalid marking token :" + std::string(cit, cend));
    }
  }
  else if (distance > 4)
  {
    try
    {
      return std::stoi(std::string(cit, cend));
    }
    catch (const std::invalid_argument&)
    {
      throw parse_error("Expected a value, got " + std::string(cit, cend));
    }
  }
  else
  {
    throw parse_error("Invalid marking token :" + std::string(cit, cend));
  }
}

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
prod(std::istream& in)
{
  std::shared_ptr<pn::net> net_ptr = std::make_shared<pn::net>();
  auto& net = *net_ptr;

  std::string line, s0, s1, s2;
  line.reserve(1024);

  std::list<pn::module_node> modules;
  std::unordered_map<std::string, std::list<pn::module_node>::iterator> index;

  while (std::getline(in, line))
  {
    std::istringstream ss(line);

    ss >> s0;

    if (s0 == "#place")
    {
      // module, if any
      bool has_module = false;
      if (((ss >> std::ws).peek() == std::char_traits<char>::to_int_type('(')))
      {
        ss >> s0;
        if (*std::prev(s0.cend()) != ')')
        {
          throw parse_error("Expected a module specification got " + s0);
        }
        has_module = true;
      }

      // default marking
      unsigned int m = 0;
      if (not(ss >> s1))// place name
      {
        throw parse_error("Place with no identifier ");
      }
      if (ss >> s2) // have marking
      {
        static const std::string mk("mk(");
        if (  s2.size() < 8 or not std::equal(mk.cbegin(), mk.cend(), s2.cbegin())
            or *(s2.cend() - 1) != ')')
        {
          throw parse_error("Invalid marking, got " + s2);
        }
        m = marking(s2.cbegin() + 3, s2.cend() - 1);
      }

      const auto& p = net.add_place(s1, m);
      if (has_module)
      {
        const auto search = index.find(s0);
        if (search == index.cend())
        {
          modules.emplace_back(pn::module_node(s0));
          index[s0] = std::prev(modules.end());
        }
        index[s0]->add_module(pn::make_module(p));
      }
    }

    else if (s0 == "#trans")
    {
      static auto read_line = [&]()
                                 {
                                   if (not std::getline(in,line))
                                     throw parse_error("Incomplete transition.");
                                   return std::istringstream(line);
                                 };
      if (not (ss >> s0))
      {
        throw parse_error("Transition with no identifier ");
      }

      net.add_transition(s0);

      // pre
      auto l = read_line();
      if (not (l >> kw("in") >> s1))
      {
        throw parse_error("Invalid pre for transition " + s0 + " : " + s1);
      }

      if (not (*s1.cbegin() == '{' and *(s1.cend() - 1) == '}'))
      {
        throw parse_error("Missing '{' or '}' " + s1);
      }

      for (const auto& arc_str : split(s1.cbegin() + 1, s1.cend() - 1, ';'))
      {
        const auto arc = split(arc_str.cbegin(), arc_str.cend(), ':');
        if (arc.size() != 2)
        {
          throw parse_error("Incorrect arc " + arc_str);
        }
        net.add_pre_place(s0, arc[0], marking(arc[1].cbegin(), arc[1].cend()));
      }

      // post
      l = read_line();
      if (not (l >> kw("out") >> s1))
      {
        throw parse_error("Invalid post for transition " + s0 + " : " + s1);
      }

      if (not (*s1.cbegin() == '{' and *(s1.cend() - 1) == '}'))
      {
        throw parse_error("Missing '{' or '}' " + s1);
      }

      for (const auto& arc_str : split(s1.cbegin() + 1, s1.cend() - 1, ';'))
      {
        const auto arc = split(arc_str.cbegin(), arc_str.cend(), ':');
        if (arc.size() != 2)
        {
          throw parse_error("Incorrect arc " + arc_str);
        }
        net.add_post_place(s0, arc[0], marking(arc[1].cbegin(), arc[1].cend()));
      }

      // end of transition
      l = read_line();
      l >> kw("#endtr");
    }

    else
    {
      throw parse_error("Invalid line, got " + line);
    }
  }

  if (not modules.empty())
  {
    pn::module_node root("root");
    for (const auto& m : modules)
    {
      root.add_module(pn::make_module(m));
    }
    net.modules = pn::make_module(root);
  }

  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
