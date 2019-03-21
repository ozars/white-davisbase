#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "util.hpp"

namespace white::davisbase::ast {

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

struct LiteralValue
{
  std::variant<std::string, long double, long long> value;
};

inline std::ostream& operator<<(std::ostream& os, const LiteralValue& literal)
{
  util::OutputManipulator om(os);
  std::visit(
    [&](auto& value) {
      if (!std::is_arithmetic_v<std::remove_reference_t<decltype(value)>>)
        os << "\"" << value << "\"";
      else
        os << value;
    },
    literal.value);
  return os;
}

struct ColumnModifiers
{
  struct IsNull
  {};
  struct NotNull
  {};
  struct PrimaryKey
  {};
  struct Unique
  {};
  struct AutoIncrement
  {};
  struct DefaultValue
  {
    LiteralValue literal;
  };

  std::optional<IsNull> is_null;
  std::optional<NotNull> not_null;
  std::optional<PrimaryKey> primary_key;
  std::optional<AutoIncrement> auto_increment;
  std::optional<Unique> unique;
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
     << "is_null=" << modifiers.is_null.has_value()
     << ", not_null=" << modifiers.not_null.has_value()
     << ", primary_key=" << modifiers.primary_key.has_value()
     << ", unique=" << modifiers.unique.has_value()
     << ", autoincrement=" << modifiers.auto_increment.has_value()
     << ", default_value=";
  if (modifiers.default_value.has_value())
    os << modifiers.default_value.value();
  else
    os << "null";
  os << ")";

  return os;
}

struct Column
{
  std::string name;
  ColumnType type;
  ColumnModifiers modifiers;
};

inline std::ostream& operator<<(std::ostream& os, const Column& column)
{
  util::OutputManipulator om(os);
  return os << "Column(name=\"" << column.name << "\", type=" << column.type
            << ", modifiers=" << column.modifiers << ")";
}

struct ShowTablesCommand
{
  void execute();
};

inline std::ostream& operator<<(std::ostream& os, __attribute__((unused))
                                                  const ShowTablesCommand& cmd)
{
  util::OutputManipulator om(os);
  return os << "ShowTablesCommand()";
}

struct DropTableCommand
{
  void execute();

  std::string table_name;
};
inline std::ostream& operator<<(std::ostream& os, const DropTableCommand& cmd)
{
  util::OutputManipulator om(os);
  return os << "DropTableCommand(table_name=\"" << cmd.table_name << "\")";
}

struct CreateTableCommand
{
  void execute();

  std::string table_name;
  std::vector<ast::Column> columns;
};

inline std::ostream& operator<<(std::ostream& os, const CreateTableCommand& cmd)
{
  using util::join;
  util::OutputManipulator om(os);
  return os << "CreateTableCommand(table_name=\"" << cmd.table_name
            << "\", columns=[" << join(cmd.columns, ", ") << "])";
}

struct InsertIntoCommand
{
  void execute();

  std::string table_name;
  std::vector<std::string> column_names;
  std::vector<LiteralValue> values;
};

inline std::ostream& operator<<(std::ostream& os, const InsertIntoCommand& cmd)
{
  using util::join;
  util::OutputManipulator om(os);
  return os << "InsertIntoCommand(table_name=\"" << cmd.table_name
            << "\", column_names=[" << join(cmd.column_names, ", ")
            << "], values=[" << join(cmd.values, ", ") << "])";
}

struct Command
{
  void execute();

  std::variant<ShowTablesCommand, DropTableCommand, CreateTableCommand,
               InsertIntoCommand>
    command;
};

} // namespace white::davisbase::ast

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::Command, command)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::ShowTablesCommand)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::DropTableCommand, table_name)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::CreateTableCommand, table_name,
                          columns)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::InsertIntoCommand, table_name,
                          column_names, values)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::ColumnModifiers::IsNull)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::ColumnModifiers::NotNull)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::ColumnModifiers::AutoIncrement)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::ColumnModifiers::Unique)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::ColumnModifiers::PrimaryKey)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::ColumnModifiers::DefaultValue,
                          literal)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::ColumnModifiers, is_null,
                          not_null, primary_key, auto_increment, unique,
                          default_value)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::Column, name, type, modifiers)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::LiteralValue, value)
