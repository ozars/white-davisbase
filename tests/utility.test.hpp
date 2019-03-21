#pragma once

#include <sstream>
#include <tuple>

#include "catch2/catch.hpp"

#include "util.hpp"

/* Utility for going over enum types */

template<typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
class EnumRangeGenerator : public Catch::Generators::IGenerator<T>
{
private:
  using underlying_t = std::underlying_type_t<T>;
  T m_current;
  T m_last;

public:
  EnumRangeGenerator(T first, T last)
    : m_current(first)
    , m_last(last)
  {
    assert(m_current <= m_last &&
           "Range must include at least one valid value");
  }

  const T& get() const override { return m_current; }
  bool next() override
  {
    m_current = static_cast<T>(static_cast<underlying_t>(m_current) + 1);
    return m_current <= m_last;
  }
};

template<typename T>
Catch::Generators::GeneratorWrapper<T> enum_range(T first, T last)
{
  static_assert(std::is_enum_v<T>, "enum_range values must be of type enum");

  return Catch::Generators::GeneratorWrapper<T>(
    std::unique_ptr<Catch::Generators::IGenerator<T>>(
      new EnumRangeGenerator<T>(first, last)));
}

/* Utility for inferring Catch's map parameter. */
/* Alternative for: https://github.com/catchorg/Catch2/pull/1576 */

template<typename Func, typename U, typename T = std::result_of_t<Func(U)>>
auto mapf(Func&& function, Catch::Generators::GeneratorWrapper<U>&& generator)
{
  static_assert(std::is_invocable_r_v<T, Func, U>,
                "Map function must take only one parameter of same type with "
                "the output of the generator");
  return Catch::Generators::map<T, U, Func>(std::forward<Func>(function),
                                            std::move(generator));
}

/* Allow capture by copy in generators.*/
/* Alternative for: https://github.com/catchorg/Catch2/pull/1566 */

#ifdef GENERATE
#undef GENERATE
#endif

#define GENERATE(...)                                                          \
  Catch::Generators::generate(CATCH_INTERNAL_LINEINFO, [=] {                   \
    using namespace Catch::Generators;                                         \
    return makeGenerators(__VA_ARGS__);                                        \
  })
