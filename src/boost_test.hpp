#pragma once

#include <boost/spirit/include/qi.hpp>

template<typename P>
bool test_parser(char const* input, P const& p, bool full_match = true)
{
  using boost::spirit::qi::parse;

  char const* f(input);
  char const* l(f + strlen(f));
  return (parse(f, l, p) && (!full_match || (f == l)));
}

template<typename P>
bool test_phrase_parser(char const* input, P const& p, bool full_match = true)
{
  using boost::spirit::qi::phrase_parse;
  using boost::spirit::qi::ascii::space;

  const std::string f(input);
  auto b = f.begin(), e = f.end();
  return (phrase_parse(b, e, p, space) && (!full_match || (b == e)));
}

template<typename P, typename T>
bool test_parser_attr(char const* input, P const& p, T& attr,
                      bool full_match = true)
{
  using boost::spirit::qi::parse;

  char const* f(input);
  char const* l(f + strlen(f));
  return (parse(f, l, p, attr) && (!full_match || (f == l)));
}

template<typename P, typename T>
bool test_phrase_parser_attr(char const* input, P const& p, T& attr,
                             bool full_match = true)
{
  using boost::spirit::qi::phrase_parse;
  using boost::spirit::qi::ascii::space;

  const std::string f(input);
  auto b = f.begin(), e = f.end();
  return (phrase_parse(b, e, p, space, attr) && (!full_match || (b == e)));
}
