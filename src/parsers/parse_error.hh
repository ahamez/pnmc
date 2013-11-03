#include <exception>

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

struct parse_error final
  : public std::exception
{
public:

  ~parse_error() noexcept;

  const char* what() const noexcept;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
