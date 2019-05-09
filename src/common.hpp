#pragma once

#include <array>
#include <optional>
#include <string>
#include <variant>

#include <boost/fusion/include/adapt_struct.hpp>

#include "util.hpp"

namespace white::davisbase::common {

enum class ColumnType : uint8_t
{
  TINYINT = 0,
  SMALLINT,
  INT,
  BIGINT,
  FLOAT,
  YEAR,
  TIME,
  DATETIME,
  DATE,
  TEXT,
  _FIRST = TINYINT,
  _LAST = TEXT
};

inline std::string to_string(const ColumnType& type)
{
  auto vals = std::array{"TINYINT", "SMALLINT", "INT",      "BIGINT", "FLOAT",
                         "YEAR",    "TIME",     "DATETIME", "DATE",   "TEXT"};
  if (type < ColumnType::_FIRST || type > ColumnType::_LAST)
    return "UNKNOWN";
  return vals[static_cast<size_t>(type)];
}

inline std::ostream& operator<<(std::ostream& os, const ColumnType& type)
{
  util::OutputManipulator om(os);
  return os << to_string(type);
}

enum class SerialTypeCode : uint8_t
{
  NULL_TYPE = 0x00,
  TINYINT = 0x01,
  SMALLINT = 0x02,
  INT = 0x03,
  BIGINT = 0x04,
  FLOAT = 0x05,
  YEAR = 0x06,
  TIME = 0x08,
  UNUSED = 0x09,
  DATETIME = 0x0A,
  DATE = 0x0B,
  TEXT = 0x0C
};

template<ColumnType T>
struct UnderlyingColumnTypeHelper
{};

template<ColumnType T>
using UnderlyingColumnType = typename UnderlyingColumnTypeHelper<T>::type;

template<ColumnType T>
constexpr SerialTypeCode underlying_typecode =
  UnderlyingColumnTypeHelper<T>::typecode;

template<>
struct UnderlyingColumnTypeHelper<ColumnType::TINYINT>
{
  using type = int8_t;
  static constexpr SerialTypeCode typecode = SerialTypeCode::TINYINT;
};

template<>
struct UnderlyingColumnTypeHelper<ColumnType::SMALLINT>
{
  using type = int16_t;
  static constexpr SerialTypeCode typecode = SerialTypeCode::SMALLINT;
};

template<>
struct UnderlyingColumnTypeHelper<ColumnType::INT>
{
  using type = int32_t;
  static constexpr SerialTypeCode typecode = SerialTypeCode::INT;
};

template<>
struct UnderlyingColumnTypeHelper<ColumnType::BIGINT>
{
  using type = int64_t;
  static constexpr SerialTypeCode typecode = SerialTypeCode::BIGINT;
};

template<>
struct UnderlyingColumnTypeHelper<ColumnType::FLOAT>
{
  static_assert(sizeof(double) == 8, "Double type is not 8 bytes.");
  using type = double;
  static constexpr SerialTypeCode typecode = SerialTypeCode::FLOAT;
};

template<>
struct UnderlyingColumnTypeHelper<ColumnType::YEAR>
{
  using type = int8_t;
  static constexpr SerialTypeCode typecode = SerialTypeCode::YEAR;
};

template<>
struct UnderlyingColumnTypeHelper<ColumnType::TIME>
{
  using type = int32_t;
  static constexpr SerialTypeCode typecode = SerialTypeCode::TIME;
};

template<>
struct UnderlyingColumnTypeHelper<ColumnType::DATETIME>
{
  using type = uint64_t;
  static constexpr SerialTypeCode typecode = SerialTypeCode::DATETIME;
};

template<>
struct UnderlyingColumnTypeHelper<ColumnType::DATE>
{
  using type = uint64_t;
  static constexpr SerialTypeCode typecode = SerialTypeCode::DATE;
};

template<>
struct UnderlyingColumnTypeHelper<ColumnType::TEXT>
{
  using type = std::string;
  static constexpr SerialTypeCode typecode = SerialTypeCode::TEXT;
};

enum class OperatorType
{
  LESS_EQUAL,
  LESS,
  EQUAL,
  GREATER_EQUAL,
  GREATER,
  _FIRST = LESS_EQUAL,
  _LAST = GREATER
};

inline std::string to_string(const OperatorType& type)
{
  auto vals =
    std::array{"LESS_EQUAL", "LESS", "EQUAL", "GREATER_EQUAL", "GREATER"};
  if (type < OperatorType::_FIRST || type > OperatorType::_LAST)
    return "UNKNOWN";
  return vals[static_cast<size_t>(type)];
}

inline std::ostream& operator<<(std::ostream& os, const OperatorType& op)
{
  util::OutputManipulator om(os);
  return os << to_string(op);
}

struct NullValue
{
  bool operator<(const NullValue&) const { return false; }
  bool operator==(const NullValue&) const { return true; }
};

struct LiteralValue
{
  std::variant<NullValue, std::string, long double, long long> value;

  LiteralValue() = default;
  LiteralValue(const LiteralValue&) = default;
  LiteralValue(LiteralValue&&) = default;
  LiteralValue& operator=(const LiteralValue&) = default;
  LiteralValue& operator=(LiteralValue&&) = default;

  template<typename T, std::enable_if_t<std::is_arithmetic_v<T> ||
                                        std::is_enum_v<T>>* = nullptr>
  LiteralValue(T t)
    : value(std::is_floating_point_v<T>
              ? decltype(value){static_cast<long double>(t)}
              : decltype(value){static_cast<long long>(t)})
  {}

  LiteralValue(const NullValue& val)
    : value({val})
  {}
  LiteralValue(const std::string& val)
    : value({val})
  {}
  LiteralValue(std::string&& val)
    : value({std::move(val)})
  {}
  LiteralValue(const char* val)
    : value({val})
  {}
};

inline std::ostream& operator<<(std::ostream& os, const LiteralValue& literal)
{
  util::OutputManipulator om(os);
  std::visit(
    [&](auto& value) {
      using type = std::remove_cv_t<std::remove_reference_t<decltype(value)>>;
      if constexpr (std::is_same_v<type, NullValue>)
        os << "NULL";
      else if constexpr (!std::is_arithmetic_v<type>)
        os << "\"" << value << "\"";
      else
        os << value;
    },
    literal.value);
  return os;
}

struct ColumnModifiers
{
  struct DefaultValue
  {
    LiteralValue literal;
  };

  ColumnModifiers() = default;

  bool is_null;
  bool not_null;
  bool primary_key;
  bool auto_increment;
  bool unique;
  std::optional<DefaultValue> default_value;
};

inline std::ostream& operator<<(std::ostream& os,
                                const ColumnModifiers::DefaultValue& def)
{
  util::OutputManipulator om(os);
  return os << def.literal;
}

inline std::ostream& operator<<(std::ostream& os,
                                const ColumnModifiers& modifiers)
{
  util::OutputManipulator om(os);
  os << "ColumnModifiers("
     << "is_null=" << modifiers.is_null << ", not_null=" << modifiers.not_null
     << ", primary_key=" << modifiers.primary_key
     << ", unique=" << modifiers.unique
     << ", autoincrement=" << modifiers.auto_increment << ", default_value=";
  if (modifiers.default_value.has_value())
    os << modifiers.default_value.value();
  else
    os << "null";
  os << ")";

  return os;
}

struct ColumnDefinition
{
  std::string name;
  ColumnType type;
  ColumnModifiers modifiers;
};

inline std::ostream& operator<<(std::ostream& os,
                                const ColumnDefinition& column)
{
  util::OutputManipulator om(os);
  return os << "Column(name=\"" << column.name << "\", type=" << column.type
            << ", modifiers=" << column.modifiers << ")";
}

using ColumnDefinitions = std::vector<ColumnDefinition>;

inline std::ostream& operator<<(std::ostream& os,
                                const ColumnDefinitions& column_definitions)
{
  util::OutputManipulator om(os);
  return os << "[" << util::join(column_definitions, ", ") << "]";
}

} // namespace white::davisbase::common

BOOST_FUSION_ADAPT_STRUCT(
  white::davisbase::common::ColumnModifiers::DefaultValue, literal)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::common::ColumnModifiers, is_null,
                          not_null, primary_key, auto_increment, unique,
                          default_value)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::common::ColumnDefinition, name,
                          type, modifiers)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::common::LiteralValue, value)
