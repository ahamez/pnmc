#include <algorithm>
#include <istream>
#include <sstream>

#include "parsers/tina.hh"
#include "pn/arc.hh"

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

struct parse_error final
  : public std::exception
{
public:

  ~parse_error()
  noexcept
  {}

  const char*
  what()
  const noexcept
  {
    return "";
  }
};

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
    throw parse_error();
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
      throw parse_error();
    }
  }
  else
  {
    throw parse_error();
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
      if (((ss >> std::ws).peek() == '#') or not (ss >> s0))
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
          throw parse_error();
        }

        // Skip time interval, if any.
        const auto c = (ss >> std::ws).peek();
        if (c == '[' or c == ']')
        {
          ss >> s1;
        }

        while (ss >> s1)
        {
          if (s1 == "->")
          {
            break;
          }
          std::tie(place_id, valuation) = place_valuation(s1);
          net.add_pre_place(s0, place_id, pn::arc(valuation));
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
          throw parse_error();
        }
      }

      // Error.
      else
      {
        throw parse_error();
      }
    }
  }
  catch (const parse_error&)
  {
    return nullptr;
  }

  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
