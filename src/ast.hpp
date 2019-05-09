#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "common.hpp"
#include "sdl/database.hpp"
#include "util.hpp"

namespace white::davisbase::ast {

struct WhereClause
{
  std::string column_name;
  common::OperatorType op;
  common::LiteralValue literal;
};

inline std::ostream& operator<<(std::ostream& os, const WhereClause& where)
{
  util::OutputManipulator om(os);
  return os << "WhereClause(column_name=\"" << where.column_name
            << "\", op=" << where.op << ", literal=" << where.literal << ")";
}

struct ShowTablesCommand
{
  void execute(sdl::Database& database);
};

inline std::ostream& operator<<(std::ostream& os, __attribute__((unused))
                                                  const ShowTablesCommand& cmd)
{
  util::OutputManipulator om(os);
  return os << "ShowTablesCommand()";
}

struct DropTableCommand
{
  void execute(sdl::Database& database);

  std::string table_name;
};

inline std::ostream& operator<<(std::ostream& os, const DropTableCommand& cmd)
{
  util::OutputManipulator om(os);
  return os << "DropTableCommand(table_name=\"" << cmd.table_name << "\")";
}

struct CreateTableCommand
{
  void execute(sdl::Database& database);

  std::string table_name;
  std::vector<common::ColumnDefinition> columns;
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
  void execute(sdl::Database& database);

  std::string table_name;
  std::vector<std::string> column_names;
  std::vector<common::LiteralValue> values;
};

inline std::ostream& operator<<(std::ostream& os, const InsertIntoCommand& cmd)
{
  using util::join;
  util::OutputManipulator om(os);
  return os << "InsertIntoCommand(table_name=\"" << cmd.table_name
            << "\", column_names=[" << join(cmd.column_names, ", ")
            << "], values=[" << join(cmd.values, ", ") << "])";
}

struct SelectCommand
{
  void execute(sdl::Database& database);

  std::vector<std::string> column_names;
  std::string table_name;
  std::optional<WhereClause> condition;
};

inline std::ostream& operator<<(std::ostream& os, const SelectCommand& cmd)
{
  using util::join;
  util::OutputManipulator om(os);
  os << "SelectCommand(table_name=\"" << cmd.table_name << "\", column_names=["
     << join(cmd.column_names, ", ") << "]";
  if (cmd.condition.has_value())
    os << ", condition=" << cmd.condition.value();
  return os << ")";
}

struct DeleteFromCommand
{
  void execute(sdl::Database& database);

  std::string table_name;
  std::optional<WhereClause> condition;
};

inline std::ostream& operator<<(std::ostream& os, const DeleteFromCommand& cmd)
{
  using util::join;
  util::OutputManipulator om(os);
  os << "DeleteFromCommand(table_name=\"" << cmd.table_name << "\"";
  if (cmd.condition.has_value())
    os << ", condition=" << cmd.condition.value();
  return os << ")";
}

struct UpdateCommand
{
  void execute(sdl::Database& database);

  std::string table_name;
  std::string column_name;
  common::LiteralValue value;
  std::optional<WhereClause> condition;
};

inline std::ostream& operator<<(std::ostream& os, const UpdateCommand& cmd)
{
  util::OutputManipulator om(os);
  os << "UpdateTableCommand(table_name=\"" << cmd.table_name
     << "\", column_name=" << cmd.column_name << ", value=" << cmd.value;
  if (cmd.condition.has_value())
    os << ", condition=" << cmd.condition.value();
  return os << ")";
}

struct CreateIndexCommand
{
  void execute(sdl::Database& database);

  bool is_unique;
  std::string table_name;
  std::string column_name;
};

inline std::ostream& operator<<(std::ostream& os, const CreateIndexCommand& cmd)
{
  using util::join;
  util::OutputManipulator om(os);
  return os << "CreateIndexCommand(table_name=\"" << cmd.table_name
            << "\", column_name=\"" << cmd.column_name << "\"])";
}

struct ExitCommand
{};

struct Command
{
  void execute(sdl::Database& database);

  std::variant<ShowTablesCommand, DropTableCommand, CreateTableCommand,
               InsertIntoCommand, SelectCommand, DeleteFromCommand,
               UpdateCommand, CreateIndexCommand, ExitCommand>
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

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::UpdateCommand, table_name,
                          column_name, value, condition)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::SelectCommand, column_names,
                          table_name, condition)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::DeleteFromCommand, table_name,
                          condition)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::CreateIndexCommand, is_unique,
                          table_name, column_name)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::ExitCommand)

BOOST_FUSION_ADAPT_STRUCT(white::davisbase::ast::WhereClause, column_name, op,
                          literal)
