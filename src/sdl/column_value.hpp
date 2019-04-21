#pragma once

#include <boost/numeric/conversion/cast.hpp>

#include "../common.hpp"

namespace white::davisbase::sdl {

template<common::ColumnType T>
class ColumnValue
{
public:
  static constexpr auto column_type = T;
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
};

template<common::ColumnType T>
std::ostream& operator<<(std::ostream os, const ColumnValue<T>& val);

template<common::ColumnType T>
template<typename CastedType>
ColumnValue<T>::ColumnValue(CastedType&& value)
  : value_(std::is_arithmetic_v<CastedType>
             ? boost::numeric_cast<underlying_type>(value)
             : underlying_type(std::forward<CastedType>(value)))
{}

template<common::ColumnType T>
template<typename CastedType>
ColumnValue<T>& ColumnValue<T>::operator=(CastedType&& value)
{
  if constexpr (std::is_arithmetic_v<CastedType>)
    value_ = boost::numeric_cast<underlying_type>(value);
  else
    value_ = underlying_type(std::forward<CastedType>(value));
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
