#include <istream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

#include "parsers/tina.hh"
#include "pn/arc.hh"

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;

/*------------------------------------------------------------------------------------------------*/

template <typename Iterator>
struct tina_parser
  : qi::grammar<Iterator, void(), ascii::space_type>
{
  qi::rule< Iterator, void(), ascii::space_type>
          start;

  qi::rule< Iterator, std::string()>
          id;

  qi::rule< Iterator, std::string(), ascii::space_type>
          label;

  qi::rule< Iterator, void(), qi::locals<std::string, std::string>
          , ascii::space_type>
          place;

  qi::rule< Iterator, void(), ascii::space_type>
          time_interval;

  qi::rule< Iterator, pn::arc(), ascii::space_type>
          arc;

  qi::rule< Iterator, unsigned int(), ascii::space_type>
          valuation;

  qi::rule< Iterator, void(), qi::locals<std::string, std::string, pn::arc, std::string>
          , ascii::space_type>
          transition;

  qi::rule< Iterator, void(), ascii::space_type>
          net;

  tina_parser(pn::net& n)
    : tina_parser::base_type(start)
  {
    using ascii::char_;

    using phoenix::bind;
    using phoenix::construct;

    using qi::_1;
    using qi::_a;
    using qi::_b;
    using qi::_c;
    using qi::_d;
    using qi::_val;
    using qi::eps;
    using qi::space;
    using qi::lexeme;
    using qi::lit;
    using qi::uint_;

    id %= +(qi::alpha|qi::digit|'\''|'_');

    label %= lit(':') >> id;

    place =
       lit("pl")
    >> id[_a = _1]
    >> -label[_b = _1]
    >> lit('(')
    >> uint_[bind(&pn::net::add_place, n, _a, _b, _1)]
    >> lit(')')
    ;

    time_interval =
       (lit('[') | lit(']'))
    >> uint_
    >> lit(',')
    >> (uint_ | lit('w'))
    >> (lit('[') | lit(']'))
    ;

    valuation = uint_[_val = _1] >> -(lit('K')[_val *= 1000]|lit('M')[_val *= 1000000]);

    arc = ( lit('*') | lit('?') | lit("?-") | lit('!') | lit("!-") )
          >> valuation [_val = construct<pn::arc>(_1)];

    transition =
       lit("tr")
    >> id[_a = _1]
    >> eps[_d = ""]
    >> -label[_d = _1]
    >> eps[bind(&pn::net::add_transition, n, _a, _d)]
    >> -time_interval
    >> *(id     [_b = _1]
        >> eps    [_c = construct<pn::arc>()] // default arc if no one is specified
        >> -arc [_c = _1]               // an arc was found, use it
        )         [bind(&pn::net::add_pre_place, n, _a, _b, _c)]
    >> lit("->")
    >> *((id      [_b=_1]
          >> eps    [_c=construct<pn::arc>()]
          >> -arc [_c=_1]
         )          [bind(&pn::net::add_post_place, n, _a, _b, _c)]
         - lexeme[((lit("pl") |lit("tr")|lit("net")) >> +space)]
        )
    ;

    net = lit("net") >> id/*[bind(&net::name, n, _1)]*/;

    start = +(place | transition | net);
  }
};

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
tina(std::istream& in)
{
  std::shared_ptr<pn::net> net_ptr = std::make_shared<pn::net>();

  // Don't skip whitespaces, the parser will do it.
  in.unsetf(std::ios::skipws);

  boost::spirit::istream_iterator cit(in);
  boost::spirit::istream_iterator end;

  tina_parser<boost::spirit::istream_iterator> parser(*net_ptr);

  const bool r = qi::phrase_parse(cit, end, parser, ascii::space);

  if (not r or cit != end)
  {
    // Parse failed.
    return nullptr;
  }

  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
