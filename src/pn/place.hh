#ifndef _PNMC_PN_PLACE_HH_
#define _PNMC_PN_PLACE_HH_

#include <iosfwd>
#include <string>

#include "pn/arc.hh"

namespace pnmc { namespace pn {

/*------------------------------------------------------------------------------------------------*/

struct place
{
  /// @brief This place's id.
  const std::string id;

  /// @brief This place's label.
  std::string label;

  /// @brief This place's marking.
  unsigned int marking;

  place(const std::string& id, const std::string& label, unsigned int m)
    : id(id)
    , label(label)
    , marking(m)
  {}
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Compare two places using their ids.
bool
operator<(const place& left, const place& right);

/*------------------------------------------------------------------------------------------------*/

/// @brief Export a place to an output stream.
std::ostream&
operator<<(std::ostream& os, const place& p);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn

#endif // _PNMC_PN_PLACE_HH_
