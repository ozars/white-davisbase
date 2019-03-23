#pragma once

#include <boost/spirit/include/qi.hpp>
#include <iomanip>

namespace white::util {

class OutputManipulator
{
public:
  explicit OutputManipulator(std::ostream& stream)
    : stream_(stream)
    , old_flags_(stream_.flags())
  {
    stream << std::boolalpha << std::showpoint;
  }

  ~OutputManipulator() { stream_.setf(old_flags_); }

private:
  std::ostream& stream_;
  std::ostream::fmtflags old_flags_;
};

/* Utility to create std::array. Polyfilled from C++20 spec. */

template<typename Dest = void, typename... Arg>
constexpr auto make_array(Arg&&... arg)
{
  if constexpr (std::is_same<void, Dest>::value)
    return std::array<std::common_type_t<std::decay_t<Arg>...>, sizeof...(Arg)>{
      {std::forward<Arg>(arg)...}};
  else
    return std::array<Dest, sizeof...(Arg)>{{std::forward<Arg>(arg)...}};
}

/* Utility to join stuff */

template<class Str, class It, typename Accessor>
auto join(It begin, const It end, const Str& sep, Accessor&& accessor)
{
  using ReturnType = std::conditional_t<std::is_convertible_v<Str, const char*>,
                                        std::string, Str>;
  using char_type = typename ReturnType::value_type;
  using traits_type = typename ReturnType::traits_type;
  using allocator_type = typename ReturnType::allocator_type;
  using ostringstream_type =
    std::basic_ostringstream<char_type, traits_type, allocator_type>;

  ostringstream_type result;

  if (begin != end)
    result << accessor(*begin++);
  while (begin != end) {
    result << sep;
    result << accessor(*begin++);
  }
  return result.str();
}

template<class Str, class Container, class Accessor>
auto join(Container container, const Str& sep, Accessor&& accessor)
{
  return join(container.begin(), container.end(), sep,
              std::forward<Accessor>(accessor));
}

template<class Str, class It>
auto join(It begin, const It end, const Str& sep)
{
  return join(begin, end, sep, [](auto& val) { return val; });
}

template<class Str, class Container>
auto join(Container container, const Str& sep)
{
  return join(container.begin(), container.end(), sep);
}

/* boost::qi debugging helpers */

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

} // namespace white::util
