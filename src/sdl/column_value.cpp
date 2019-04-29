#define COLUMN_VALUE_EXPLICIT_IMPLEMENTATION
#include "column_value.hpp"

namespace white::davisbase::sdl {

using common::ColumnType;
using common::LiteralValue;

template<ColumnType T>
ColumnValue<T>::ColumnValue(const underlying_type& value) noexcept
  : value_(value)
{}

template<ColumnType T>
ColumnValue<T>::ColumnValue(underlying_type&& value) noexcept
  : value_(std::move(value))
{}

template<ColumnType T>
ColumnValue<T>& ColumnValue<T>::operator=(const underlying_type& value) noexcept
{
  value_ = value;
  return *this;
}

template<ColumnType T>
ColumnValue<T>& ColumnValue<T>::operator=(underlying_type&& value) noexcept
{
  value_ = std::move(value);
  return *this;
}

template<ColumnType T>
const typename ColumnValue<T>::underlying_type& ColumnValue<T>::get() const
{
  return value_;
}

template<ColumnType T>
ColumnValue<T>::operator underlying_type() const
{
  return value_;
}

template<ColumnType T>
std::ostream& operator<<(std::ostream& os, const ColumnValue<T>& val)
{
  return os << val.get();
}

std::ostream& operator<<(std::ostream& os,
                         const ColumnValue<ColumnType::TINYINT>& val)
{
  return os << int(val.get());
}

std::ostream& operator<<(std::ostream& os,
                         const ColumnValue<ColumnType::YEAR>& val)
{
  return os << 2000 + val.get();
}

std::ostream& operator<<(std::ostream& os, const ColumnValueVariant& variant)
{
  using common::NullValue;
  std::visit(
    [&os](auto& col) {
      if constexpr (std::is_same_v<NullValue, std::decay_t<decltype(col)>>)
        os << "NULL";
      else
        os << col;
    },
    variant);
  return os;
}

/* TODO: Specialize operator<< for TIME, DATETIME and DATE. */

ColumnValueVariant createColumnValue(ColumnType column_type,
                                     const LiteralValue& literal_value)
{
  using common::NullValue;

  auto get_column_value_variant = [&](auto& val) -> ColumnValueVariant {
    using T = std::decay_t<decltype(val)>;
    if constexpr (std::is_same_v<T, NullValue>)
      return NullValue();
    else {
      switch (column_type) {
        case ColumnType::TINYINT: {
          using ColumnValue = TinyIntColumnValue;
          return ColumnValue(std::move(val));
        }
        case ColumnType::SMALLINT: {
          using ColumnValue = SmallIntColumnValue;
          return ColumnValue(std::move(val));
        }
        case ColumnType::INT: {
          using ColumnValue = IntColumnValue;
          return ColumnValue(std::move(val));
        }
        case ColumnType::BIGINT: {
          using ColumnValue = BigIntColumnValue;
          return ColumnValue(std::move(val));
        }
        case ColumnType::FLOAT: {
          using ColumnValue = FloatColumnValue;
          return ColumnValue(std::move(val));
        }
        case ColumnType::YEAR: {
          using ColumnValue = YearColumnValue;
          return ColumnValue(std::move(val));
        }
        case ColumnType::TIME: {
          using ColumnValue = TimeColumnValue;
          return ColumnValue(std::move(val));
        }
        case ColumnType::DATETIME: {
          using ColumnValue = DateTimeColumnValue;
          return ColumnValue(std::move(val));
        }
        case ColumnType::DATE: {
          using ColumnValue = DateColumnValue;
          return ColumnValue(std::move(val));
        }
        case ColumnType::TEXT: {
          using ColumnValue = TextColumnValue;
          return ColumnValue(std::move(val));
        }
        default:
          throw std::runtime_error("Unknown column type");
      }
    };
  };

  return std::visit(get_column_value_variant, literal_value.value);
}

RowData createRowData(const common::ColumnDefinitions& column_definitions,
                      const std::vector<common::LiteralValue>& literal_values)
{
  if (column_definitions.size() != literal_values.size())
    throw std::runtime_error(
      "Column definitions should be same size with literal values");

  RowData row_data;
  row_data.reserve(column_definitions.size());
  for (size_t i = 0; i < column_definitions.size(); i++)
    row_data.push_back(
      createColumnValue(column_definitions[i].type, literal_values[i]));
  return row_data;
}

template class ColumnValue<common::ColumnType::TINYINT>;
template class ColumnValue<common::ColumnType::SMALLINT>;
template class ColumnValue<common::ColumnType::INT>;
template class ColumnValue<common::ColumnType::BIGINT>;
template class ColumnValue<common::ColumnType::FLOAT>;
template class ColumnValue<common::ColumnType::YEAR>;
template class ColumnValue<common::ColumnType::TIME>;
template class ColumnValue<common::ColumnType::DATETIME>;
template class ColumnValue<common::ColumnType::DATE>;
template class ColumnValue<common::ColumnType::TEXT>;

} // namespace white::davisbase::sdl
