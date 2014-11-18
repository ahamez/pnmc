#include <cctype> // isalnum
#include <ostream>

#include "tina.hh"

namespace pnmc { namespace translator {

/*------------------------------------------------------------------------------------------------*/

struct id_to_tina
{
  const std::string& id;

  friend
  std::ostream&
  operator<<(std::ostream& os, const id_to_tina& manip)
  {
    const auto aname = std::all_of( begin(manip.id), end(manip.id)
                                  , [](auto c){return std::isalnum(c) or c == '_' or c == '\'';});
    if (aname)
    {
      return os << manip.id;
    }
    else
    {
      auto s = std::string{};
      s.reserve(manip.id.size());
      for (auto c : manip.id)
      {
        if (c == '\\' or c == '{' or c == '}')
        {
          s.push_back('\\');
        }
        s.push_back(c);
      }
      return os << '{' << s << '}';
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

id_to_tina
tina(const std::string& id)
{
  return {id};
}

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const pn::transition::arcs_type::value_type& arc)
{
  os << tina(arc.first);
  switch (arc.second.kind)
  {
    case pn::arc::type::normal:
      if (arc.second.weight > 1) os << '*' << arc.second.weight;
      break;

    case pn::arc::type::inhibitor:
      os << "?-" << arc.second.weight;
      break;

    case pn::arc::type::read:
      os << '?' << arc.second.weight;
      break;

    case pn::arc::type::stopwatch:
      os << '!' << arc.second.weight;
      break;

    case pn::arc::type::stopwatch_inhibitor:
      os << "!-" << arc.second.weight;
      break;

    case pn::arc::type::reset:
      os << "**";
      if (arc.second.weight > 1) os << arc.second.weight;
  }
  return os;
}

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const net_to_tina& manip)
{
  for (const auto& t : manip.net.transitions())
  {
    os << "tr " << tina(t.id);

    if (t.timed() and t.low != 0 and t.high != 0)
    {
      os << " [" << t.low << ',' <<  t.high << ']';
    }

    for (const auto& a : t.pre)
    {
      os << ' ' << a;
    }
    os << " ->";
    for (const auto& a : t.post)
    {
      os << ' ' << a;
    }
    os << '\n';
  }

  for (const auto& p : manip.net.places_by_insertion())
  {
    os << "pl " << tina(p.id)<< " (" << p.marking << ")\n";
  }

  return os << "net " << tina(manip.net.name) << '\n';
}

/*------------------------------------------------------------------------------------------------*/

net_to_tina
tina(const pn::net& net)
{
  return {net};
}

/*------------------------------------------------------------------------------------------------*/

}} // pnmc::translator
