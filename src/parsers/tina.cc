#include <algorithm>
#include <iostream>
#include <sstream>

#include "parsers/parse_error.hh"
#include "parsers/tina.hh"

namespace pnmc { namespace parsers {

namespace {

/*------------------------------------------------------------------------------------------------*/

unsigned int
value(std::string::const_iterator cit, std::string::const_iterator cend)
{
  std::string s(cit, cend);
  try
  {
    std::size_t pos;
    auto value = std::stoi(s, &pos);
    std::advance(cit, pos);
    if (std::distance(cit, cend) == 1)
    {
      switch (*cit)
      {
        case 'K': value *= 1000; break;
        case 'M': value *= 1000000; break;
        default: throw parse_error("Invalid suffix, got " + std::string(*cit, 1));
      }
    }
    else if (std::distance(cit, cend) > 1)
    {
      throw parse_error("Invalid suffix, got " + std::string(cit, cend));
    }
    return value;
  }
  catch (const std::invalid_argument&)
  {
    throw parse_error("Expected a value, got " + s);
  }
}

/*------------------------------------------------------------------------------------------------*/

std::pair<std::string, unsigned int>
place_valuation(const std::string& s)
{
  const auto star_cit = std::find(s.cbegin(), s.cend(), '*');
  if (star_cit == s.cend())
  {
    return std::make_pair(s, 1);
  }
  const auto valuation = value(star_cit + 1, s.cend());
  return std::make_pair(std::string(s.cbegin(), star_cit), valuation);
}

/*------------------------------------------------------------------------------------------------*/

unsigned int
marking(const std::string& s)
{
  if (*s.cbegin() == '(' and *std::prev(s.cend()) == ')')
  {
    return value(std::next(s.cbegin()), std::prev(s.cend()));
  }
  else
  {
    throw parse_error("Invalid marking format: " + s);
  }
}

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
tina(std::istream& in)
{
  std::shared_ptr<pn::net> net_ptr = std::make_shared<pn::net>();
  auto& net = *net_ptr;

  std::string line, s0, s1, s2;
  line.reserve(1024);

  while (std::getline(in, line))
  {
    std::istringstream ss(line);

    // Empty line or comment.
    if (((ss >> std::ws).peek() == std::char_traits<char>::to_int_type('#')) or not (ss >> s0))
    {
      continue;
    }

    // Net
    if (s0 == "net")
    {
      continue;
    }

    // Transitions
    else if (s0 == "tr")
    {
      std::string place_id;
      unsigned int valuation;

      if (ss >> s0)
      {
        net.add_transition(s0);
      }
      else
      {
        throw parse_error("Invalid transition: " + line);
      }

      // Skip time interval, if any.
      // We don't check if the interval is well-formed...
      const auto c = (ss >> std::ws).peek();
      if (  c == std::char_traits<char>::to_int_type('[')
          or c == std::char_traits<char>::to_int_type(']'))
      {
        ss >> s1;
      }

      bool found_arrow = false;
      while (ss >> s1)
      {
        if (s1 == "->")
        {
          found_arrow = true;
          break;
        }
        std::tie(place_id, valuation) = place_valuation(s1);
        net.add_pre_place(s0, place_id, valuation);
      }

      if (not found_arrow)
      {
        throw parse_error("Invalid transition (missing '->'): " + line);
      }

      while (ss >> s1)
      {
        std::tie(place_id, valuation) = place_valuation(s1);
        net.add_post_place(s0, place_id, valuation);
      }
    }

    // Places
    else if (s0 == "pl")
    {
      if (ss >> s1)
      {
        if (ss >> s2)
        {
          net.add_place(s1, marking(s2));
        }
        else
        {
          net.add_place(s1, 0);
        }
      }
      else
      {
        throw parse_error("Invalid place: " + line);
      }
    }

    // Error.
    else
    {
      throw parse_error("Invalid line: " + line);
    }
  }

  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
