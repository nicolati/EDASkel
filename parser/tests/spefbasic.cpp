// Basic tests for the SPEF parser

#define BOOST_TEST_MODULE basic SPEF tests
#include <boost/test/included/unit_test.hpp>

#define BOOST_SPIRIT_DEBUG 1

#include "../spefparser.hpp"
#include <string>
#include <iostream>
#include <sstream>

using namespace EDASkel::SpefParse;

using boost::spirit::qi::phrase_parse;

typedef boost::spirit::istream_iterator SpefIter;
spefparser<SpefIter> spefParser;
spefskipper<SpefIter> spefSkipper;

// boilerplate parsing code
void parse_check(std::string const& str, spef& result) {
  std::stringstream testspef(str);
  testspef.unsetf(std::ios::skipws);
  SpefIter beg(testspef), end;
  BOOST_CHECK( phrase_parse(beg, end, spefParser, spefSkipper, result) );  // we should match
  BOOST_CHECK( (beg == end) );                        // we should consume all input
}


void parse_check_fail(std::string const& str) {
  std::stringstream testspef(str);
  testspef.unsetf(std::ios::skipws);
  SpefIter beg(testspef), end;
  spef result;
  BOOST_CHECK( !phrase_parse(beg, end, spefParser, spefSkipper, result) );  // we should NOT match
}

BOOST_AUTO_TEST_CASE( design_name ) {

   spef result;
   parse_check("// some comment\n*DESIGN \"GreatDesign\"\n// some other comment\n", result);
   BOOST_CHECK_EQUAL( "GreatDesign", result.name );

   parse_check_fail("// initial comment\n// look, no design name here\n");

}