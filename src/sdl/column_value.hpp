#pragma once

#include <variant>

#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include "../common.hpp"

namespace white::davisbase::sdl {

template<common::ColumnType T>
class ColumnValue
{
public:
  static constexpr auto column_type = T;
  static constexpr auto typecode = common::underlying_typecode<T>;
  using underlying_type = common::UnderlyingColumnType<T>;

private:
  underlying_type value_;

public:
  ColumnValue() = default;
  ColumnValue(const ColumnValue&) = default;
  ColumnValue(ColumnValue&&) = default;
  ColumnValue& operator=(const ColumnValue&) = default;
  ColumnValue& operator=(ColumnValue&&) = default;

  ColumnValue(const underlying_type& value) noexcept;
  ColumnValue(underlying_type&& value) noexcept;

  template<typename CastedType>
  ColumnValue(CastedType&& value);

  ColumnValue& operator=(const underlying_type& value) noexcept;
  ColumnValue& operator=(underlying_type&& value) noexcept;

  template<typename CastedType>
  ColumnValue& operator=(CastedType&& value);

  const underlying_type& get() const;
  explicit operator underlying_type() const;

  bool operator==(const ColumnValue& rhs) const { return get() == rhs.get(); }
  bool operator<(const ColumnValue& rhs) const { return get() < rhs.get(); }
  bool operator>(const ColumnValue& rhs) const { return get() > rhs.get(); }
  bool operator<=(const ColumnValue& rhs) const { return get() <= rhs.get(); }
  bool operator>=(const ColumnValue& rhs) const { return get() >= rhs.get(); }
  bool operator!=(const ColumnValue& rhs) const { return get() != rhs.get(); }
};

template<common::ColumnType T>
std::ostream& operator<<(std::ostream& os, const ColumnValue<T>& val);

template<typename U, typename T>
U smart_cast(T&& t)
{
  using BareU = std::remove_reference_t<std::remove_cv_t<U>>;
  using BareT = std::remove_reference_t<std::remove_cv_t<T>>;
  [[maybe_unused]] constexpr auto u_is_arithmetic =
    std::is_arithmetic_v<BareU> || std::is_enum_v<BareU>;
  [[maybe_unused]] constexpr auto t_is_arithmetic =
    std::is_arithmetic_v<BareT> || std::is_enum_v<BareT>;
  [[maybe_unused]] constexpr auto u_is_string =
    std::is_same_v<BareU, std::string>;
  [[maybe_unused]] constexpr auto t_is_string =
    std::is_same_v<BareT, std::string>;

  if constexpr (u_is_arithmetic && t_is_arithmetic)
    return boost::numeric_cast<U>(std::forward<T>(t));
  else if constexpr ((u_is_arithmetic && t_is_string) ||
                     (t_is_arithmetic && u_is_string))
    return boost::lexical_cast<U>(std::forward<T>(t));
  else
    return std::forward<T>(t);
}

template<common::ColumnType T>
template<typename CastedType>
ColumnValue<T>::ColumnValue(CastedType&& value)
  : value_(smart_cast<underlying_type>(std::forward<CastedType>(value)))
{}

template<common::ColumnType T>
template<typename CastedType>
ColumnValue<T>& ColumnValue<T>::operator=(CastedType&& value)
{
  value_ = smart_cast<underlying_type>(std::forward<CastedType>(value));
  return *this;
}

#ifndef COLUMN_VALUE_EXPLICIT_IMPLEMENTATION

extern template class ColumnValue<common::ColumnType::TINYINT>;
extern template class ColumnValue<common::ColumnType::SMALLINT>;
extern template class ColumnValue<common::ColumnType::INT>;
extern template class ColumnValue<common::ColumnType::BIGINT>;
extern template class ColumnValue<common::ColumnType::FLOAT>;
extern template class ColumnValue<common::ColumnType::YEAR>;
extern template class ColumnValue<common::ColumnType::TIME>;
extern template class ColumnValue<common::ColumnType::DATETIME>;
extern template class ColumnValue<common::ColumnType::DATE>;
extern template class ColumnValue<common::ColumnType::TEXT>;

#endif

// clang-format off
using ColumnValueVariant = std::variant<
  common::NullValue,
  ColumnValue<common::ColumnType::TINYINT>,
  ColumnValue<common::ColumnType::SMALLINT>,
  ColumnValue<common::ColumnType::INT>,
  ColumnValue<common::ColumnType::BIGINT>,
  ColumnValue<common::ColumnType::FLOAT>,
  ColumnValue<common::ColumnType::YEAR>,
  ColumnValue<common::ColumnType::TIME>,
  ColumnValue<common::ColumnType::DATETIME>,
  ColumnValue<common::ColumnType::DATE>,
  ColumnValue<common::ColumnType::TEXT>>;
// clang-format on

std::ostream& operator<<(std::ostream& os, const ColumnValueVariant& variant);

using RowData = std::vector<ColumnValueVariant>;

ColumnValueVariant createColumnValue(common::ColumnType column_type,
                                     const common::LiteralValue& literal_value);

RowData createRowData(const common::ColumnDefinitions& column_definitions,
                      const std::vector<common::LiteralValue>& literal_values);

using TinyIntColumnValue = ColumnValue<common::ColumnType::TINYINT>;
using SmallIntColumnValue = ColumnValue<common::ColumnType::SMALLINT>;
using IntColumnValue = ColumnValue<common::ColumnType::INT>;
using BigIntColumnValue = ColumnValue<common::ColumnType::BIGINT>;
using FloatColumnValue = ColumnValue<common::ColumnType::FLOAT>;
using YearColumnValue = ColumnValue<common::ColumnType::YEAR>;
using TimeColumnValue = ColumnValue<common::ColumnType::TIME>;
using DateTimeColumnValue = ColumnValue<common::ColumnType::DATETIME>;
using DateColumnValue = ColumnValue<common::ColumnType::DATE>;
using TextColumnValue = ColumnValue<common::ColumnType::TEXT>;

} // namespace white::davisbase::sdl
