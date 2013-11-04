#include <istream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_scope.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>

#include "parsers/parse_error.hh"
#include "parsers/prod.hh"

namespace pnmc { namespace parsers {

/*------------------------------------------------------------------------------------------------*/

namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;

/*------------------------------------------------------------------------------------------------*/

template <typename Iterator>
struct prod_parser
	: qi::grammar<Iterator, void(), ascii::space_type>
{
  qi::rule< Iterator, void(), ascii::space_type>
          start;

  qi::rule<Iterator, std::string(), ascii::space_type, qi::locals<std::string, unsigned int>>
          place;

  qi::rule< Iterator, std::string(), ascii::space_type
          , qi::locals<std::string, std::string, unsigned int>>
          trans;

  qi::rule<Iterator, std::string()>
          id;

  qi::rule<Iterator, unsigned int(), ascii::space_type>
          marking;

  prod_parser(pn::net& n)
		: prod_parser::base_type(start)
  {
    using phoenix::bind;
    using phoenix::construct;

    using qi::_1;
    using qi::_a;
    using qi::_b;
    using qi::_c;
    using qi::_val;
    using qi::eps;
    using qi::lit;
    using qi::uint_;
    using qi::lexeme;

    using pn::net;

    id %= +(qi::alpha|qi::digit|'_');

    trans =  lit("#trans")
          >> id[bind(&net::add_transition, n, _1) , _a = _1]
          >> lit("in")
          >> lit('{')
    				>> *( id [_b = _1]
			         >> lit(':')
        			 >> -(uint_[_c = _1] >> -lit('*'))
               >> lit("<..>")
			         >> lit(';')[bind(&net::add_pre_place, n, _a, _b, _c )]
               )
          >> lit('}')
          >> lit("out")
				    >> lit('{')
    					>> *( id[_b = _1]
         				 >> lit(':')
                 >> -(uint_[_c = _1] >> -lit('*') )
                 >> lit("<..>")
                 >> lit(';') [bind(&net::add_post_place, n, _a, _b, _c )]
                 )
          >> lit('}')
          >> lit("#endtr")
          ;

    marking = eps[_val=1] >> lit("mk(") >> -uint_[_val=_1] >> lit("<..>)");

    place = lit("#place")
          >> -(lit('(') >> id >> lit(')'))
          >> id[_a = _1]
          >> -marking[_b = _1]
          >> eps[bind(&net::add_place, n, _a, _b)]
          ;

    start = +(place|trans);
  }
};

/*------------------------------------------------------------------------------------------------*/

std::shared_ptr<pn::net>
prod(std::istream& input)
{
  std::shared_ptr<pn::net> net_ptr = std::make_shared<pn::net>();

  // Don't skip spaces, the parser will do it.
  input.unsetf(std::ios::skipws);

  boost::spirit::istream_iterator cit(input);
  boost::spirit::istream_iterator end;

  prod_parser<boost::spirit::istream_iterator> parser(*net_ptr);

  const bool r = phrase_parse(cit, end, parser, ascii::space);

  if (not r or cit != end)
  {
    throw parse_error();;
  }

  return net_ptr;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace pnmc::parsers
