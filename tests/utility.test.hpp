#pragma once

#include <sstream>
#include <tuple>

#include "catch2/catch.hpp"

/* Utility to join stuff */

template<class Str, class It>
Str join(It begin, const It end, const Str& sep)
{
  using char_type = typename Str::value_type;
  using traits_type = typename Str::traits_type;
  using allocator_type = typename Str::allocator_type;
  using ostringstream_type =
    std::basic_ostringstream<char_type, traits_type, allocator_type>;

  ostringstream_type result;

  if (begin != end)
    result << *begin++;
  while (begin != end) {
    result << sep;
    result << *begin++;
  }
  return result.str();
}

template<class Str, class Container>
Str join(Container container, Str delimiter)
{
  return join(container.begin(), container.end(), delimiter);
}

/* Utility for going over enum types */

template<typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
class EnumRangeGenerator : public Catch::Generators::IGenerator<T>
{
private:
  using underlying_t = std::underlying_type_t<T>;
  T current;
  T end;

public:
  EnumRangeGenerator(T start, T end)
    : current(start)
    , end(end)
  {
    assert(current <= end && "Range must include at least one valid value");
  }

  const T& get() const override { return current; }
  bool next() override
  {
    current = static_cast<T>(static_cast<underlying_t>(current) + 1);
    return current <= end;
  }
};

template<typename T>
Catch::Generators::GeneratorWrapper<T> enum_range(T start, T end)
{
  static_assert(std::is_enum_v<T>, "enum_range values must be of type enum");

  return Catch::Generators::GeneratorWrapper<T>(
    std::unique_ptr<Catch::Generators::IGenerator<T>>(
      new EnumRangeGenerator<T>(start, end)));
}

