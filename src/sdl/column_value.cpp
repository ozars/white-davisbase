#define COLUMN_VALUE_EXPLICIT_IMPLEMENTATION
#include "column_value.hpp"

namespace white::davisbase::sdl {

using common::ColumnType;

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
