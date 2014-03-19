#ifndef _PNMC_PN_PLACE_HH_
#define _PNMC_PN_PLACE_HH_

#include <iosfwd>
#include <map>
#include <string>

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

struct place
{
  /// @brief This place's id.
  const std::string id;

  /// @brief This place's marking.
  unsigned int marking;

  /// @brief Pre transitions.
  std::multimap<std::string, unsigned int> pre;

  /// @brief Post transitions.
  std::multimap<std::string, unsigned int> post;

  /// @brief Constructor.
  place(const std::string& id, unsigned int m);

  /// @brief Tell if this place is connected to one or more transitions.
  bool connected() const noexcept;
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Compare two places using their ids.
bool
operator<(const place& left, const place& right) noexcept;

/*------------------------------------------------------------------------------------------------*/

/// @brief Export a place to an output stream.
std::ostream&
operator<<(std::ostream& os, const place& p);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn

#endif // _PNMC_PN_PLACE_HH_
