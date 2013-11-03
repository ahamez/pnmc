#include <algorithm>
#include <istream>
#include <sstream>

#include "parsers/parse_error.hh"
#include "parsers/tina.hh"
#include "pn/arc.hh"

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

std::pair<std::string, unsigned int>
place_valuation(const std::string& s)
{
  const auto star_cit = std::find(s.cbegin(), s.cend(), '*');
  if (star_cit == s.cend())
  {
    return std::make_pair(s, 1);
  }
  try
  {
    const auto valuation = std::stoi(std::string(star_cit + 1, s.cend()));
    return std::make_pair(std::string(s.cbegin(), star_cit), valuation);
  }
  catch (const std::invalid_argument&)
  {
    throw parse_error("Valuation '" + std::string(star_cit + 1, s.cend()) + "' is not a value");
  }
}

/*------------------------------------------------------------------------------------------------*/

unsigned int
marking(const std::string& s)
{
  if (*s.cbegin() == '(' and *std::prev(s.cend()) == ')')
  {
    try
    {
      return std::stoi(std::string(std::next(s.cbegin()), std::prev(s.cend())));
    }
    catch (const std::invalid_argument&)
    {
      throw parse_error( "Marking '" + std::string(std::next(s.cbegin()), std::prev(s.cend()))
                       + "' is not a value");
    }
  }
  else
  {
    throw parse_error("Invalid marking format: " + s);
  }
}

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
tina(std::istream& in)
{
  std::shared_ptr<pn::net> net_ptr = std::make_shared<pn::net>();
  auto& net = *net_ptr;

  try
  {
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
          net.add_transition(s0, "");
        }
        else
        {
          throw parse_error("Invalid transition: " + line);
        }

        // Skip time interval, if any.
        const auto c = (ss >> std::ws).peek();
        if (c == '[' or c == ']')
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
          net.add_pre_place(s0, place_id, pn::arc(valuation));
        }

        if (not found_arrow)
        {
          throw parse_error("Invalid transition (missing '->'): " + line);
        }

        while (ss >> s1)
        {
          std::tie(place_id, valuation) = place_valuation(s1);
          net.add_post_place(s0, place_id, pn::arc(valuation));
        }
      }

      // Places
      else if (s0 == "pl")
      {
        if (ss >> s1 >> s2)
        {
          net.add_place(s1, "", marking(s2));
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
  }
  catch (const parse_error& p)
  {
    std::cerr << p.what() << std::endl;
    return nullptr;
  }

  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
