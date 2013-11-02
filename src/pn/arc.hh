#ifndef _PNMC_PN_ARC_HH_
#define _PNMC_PN_ARC_HH_

#include <iosfwd>

namespace pnmc { namespace pn {
  
/*------------------------------------------------------------------------------------------------*/

struct arc
{
  /// There are different types of arcs.
  enum class type
  { normal
  , test
  , inhibitor
  , stopwatch
  , stopwatch_inhibitor
  };

  /// Typedef the type of the weight of an arc.
  typedef unsigned int weight_type;

  /// What kind of arc it is.
  enum type kind;

  /// The weight of this arc.
  weight_type weight;

  /// Constructor.
  arc(type k, weight_type w);

  /// Default constructor;
  arc();
};

/*------------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const arc& a);

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::pn

#endif // _PNMC_PN_ARC_HH_
