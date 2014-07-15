#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

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
        default: throw parse_error("Invalid suffix, got '" + std::string(cit, cend) + "'");
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
  auto pos = s.find_first_of("*?!");

  if (pos == std::string::npos)
  {
    return std::make_pair(s, 1);
  }

  if (pos == s.length() - 1)
  {
    throw parse_error("Valuation expected, got '" + s + "'");
  }

  switch (s[pos])
  {
      case '*': break;

      case '?':
      {
        if (s[pos + 1] == '-')
        {
          ++pos;
        }
        break;
      }

      case '!':
      {
        if (s[pos +1] == '-')
        {
          ++pos;
        }
        break;
      }

      default :
      {
        std::stringstream ss;
        ss << "Expected '*', '?' or '!', got '" << s[pos] << "'";
        throw parse_error(ss.str());
      }
  }

  const auto valuation = value(s.cbegin() + pos + 1, s.cend());
  return std::make_pair(s.substr(0, pos), valuation);
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
  // Known limitations:
  // - there's must be a transition or place per line
  // - labels are ignored
  // - time intervals are not completely validated (e.g. [0, 2a] is wrongly validated

  std::shared_ptr<pn::net> net_ptr = std::make_shared<pn::net>();
  auto& net = *net_ptr;

  std::string line, s0, s1, s2, ignore;
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
      ss >> s0;
      net_ptr->name = s0;
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

      // Ignore label, if any.
      if ((ss >> std::ws).peek() == std::char_traits<char>::to_int_type(':'))
      {
        ss >> ignore >> ignore; // ':' <label>
      }

      // Time interval.
      const auto peek = (ss >> std::ws).peek();
      if (   peek == std::char_traits<char>::to_int_type('[')
          or peek == std::char_traits<char>::to_int_type(']'))
      {
        s1.clear();
        char c = ss.get(); // '[' or ']';
        while (ss.good())
        {
          c = ss.get();
          if (c == '[' or c == ']')
          {
            break;
          }
          s1.push_back(c);
        }

        if (not ss.good())
        {
          throw parse_error("Unexpected end of file. Missing '[' or ']'?");
        }

        std::istringstream ss1(s1);
        std::getline(ss1, s2, ','); // split on ','
        unsigned int first, last;
        try
        {
          first = std::stoi(s2);
        }
        catch (const std::invalid_argument&)
        {
          throw parse_error("Expected a value, got: '" + s2 + "'");
        }
        ss1 >> s2;
        try
        {
          last = std::stoi(s2);
        }
        catch (const std::invalid_argument& e)
        {
          if (s2 == "w")
          {
            last = std::numeric_limits<unsigned int>::max();
          }
          else
          {
            throw parse_error("Expected a value, got: '" + s2 + "'");
          }
        }

        net_ptr->add_time_interval(s0, first, last);
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
        // Ignore label, if any.
        if ((ss >> std::ws).peek() == std::char_traits<char>::to_int_type(':'))
        {
          ss >> ignore >> ignore; // ':' <label>
        }

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
