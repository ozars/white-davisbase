#pragma once

#include <array>
#include <optional>
#include <string>

#include <boost/fusion/include/adapt_struct.hpp>

#include "util.hpp"

namespace white::davisbase::common {

enum class ColumnType
{
  TINYINT,
  SMALLINT,
  INT,
  BIGINT,
  REAL,
  DOUBLE,
  DATETIME,
  DATE,
  TEXT,
  _FIRST = TINYINT,
  _LAST = TEXT
};

inline std::string to_string(const ColumnType& type)
{
  auto vals = std::array{"TINYINT", "SMALLINT", "INT",  "BIGINT", "REAL",
                         "DOUBLE",  "DATETIME", "DATE", "TEXT"};
  if (type < ColumnType::_FIRST || type > ColumnType::_LAST)
    return "UNKNOWN";
  return vals[static_cast<size_t>(type)];
}

inline std::ostream& operator<<(std::ostream& os, const ColumnType& type)
{
  util::OutputManipulator om(os);
  return os << to_string(type);
}

template<ColumnType T>
struct UnderlyingColumnType
{};

template<>
struct UnderlyingColumnType<ColumnType::TINYINT>
{
  using type = int8_t;
};

template<>
struct UnderlyingColumnType<ColumnType::SMALLINT>
{
  using type = int16_t;
};

template<>
struct UnderlyingColumnType<ColumnType::INT>
{
  using type = int32_t;
};

template<>
struct UnderlyingColumnType<ColumnType::BIGINT>
{
  using type = int64_t;
};

template<>
struct UnderlyingColumnType<ColumnType::REAL>
{
  static_assert(sizeof(float) == 4, "Float type is not 4 bytes.");
  using type = float;
};

template<>
struct UnderlyingColumnType<ColumnType::DOUBLE>
{
  static_assert(sizeof(float) == 4, "Float type is not 4 bytes.");
  using type = double;
};

template<>
struct UnderlyingColumnType<ColumnType::DATETIME>
{
  using type = uint64_t;
};

template<>
struct UnderlyingColumnType<ColumnType::DATE>
{
  using type = uint64_t;
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
{};

struct LiteralValue
{
  std::variant<NullValue, std::string, long double, long long> value;
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

} // namespace white::davisbase::common

BOOST_FUSION_ADAPT_STRUCT(
  white::davisbase::common::ColumnModifiers::DefaultValue, literal)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::common::ColumnModifiers, is_null,
                          not_null, primary_key, auto_increment, unique,
                          default_value)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::common::ColumnDefinition, name,
                          type, modifiers)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::common::LiteralValue, value)
