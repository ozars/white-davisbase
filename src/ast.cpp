#include "ast.hpp"

#include <iostream>

#include <map>

#include "sdl/database.hpp"
#include "sdl/table.hpp"

namespace white::davisbase::ast {

using common::NullValue;
using sdl::CellIndex;
using sdl::ColumnValueVariant;
using sdl::Database;
using sdl::TableLeafCell;
using sdl::TableLeafPage;

using sdl::createColumnValue;
using sdl::createRowData;

static bool isWhereSatisfied(const ColumnValueVariant& variant,
                             const WhereClause& condition)
{
  return std::visit(
    [&](auto&& value) {
      using T = std::decay_t<decltype(value)>;
      using common::OperatorType;

      if constexpr (std::is_same_v<T, NullValue>) {
        switch (condition.op) {
          case OperatorType::LESS:
          case OperatorType::GREATER:
            return false;
          case OperatorType::LESS_EQUAL:
          case OperatorType::GREATER_EQUAL:
          case OperatorType::EQUAL:
            return std::holds_alternative<NullValue>(condition.literal.value);
          default:
            throw std::runtime_error("Unknown operator type");
        }
      } else {
        auto condition_value = std::get<T>(
          sdl::createColumnValue(T::column_type, condition.literal));
        switch (condition.op) {
          case OperatorType::LESS_EQUAL:
            return value <= condition_value;
          case OperatorType::LESS:
            return value < condition_value;
          case OperatorType::GREATER_EQUAL:
            return value >= condition_value;
          case OperatorType::GREATER:
            return value > condition_value;
          case OperatorType::EQUAL:
            return value == condition_value;
          default:
            throw std::runtime_error("Unknown operator type");
        }
      }
    },
    variant);
}

void Command::execute(Database& database)
{
  std::visit([&](auto&& arg) { arg.execute(database); }, command);
}

void ShowTablesCommand::execute(Database& database)
{
  database.mapOverTables(
    [&](TableLeafCell cell) { std::cout << cell.row_data[0] << std::endl; });
}

void DropTableCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

void CreateTableCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

void InsertIntoCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

void SelectCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

void DeleteFromCommand::execute(Database& database)
{
  using namespace white::davisbase::sdl;
  auto table_opt = database.getTable(table_name);
  if (!table_opt.has_value()) {
    throw std::runtime_error("Table doesn't exist");
  }

  if (condition == std::nullopt) {
    throw std::runtime_error("Where clause required");
  }

  auto& table = table_opt.value();
  auto& column_defs = table.columnDefinitions();
  std::map<std::string, common::ColumnDefinition> columns_map;
  std::map<std::string, int> columns_idx_map;

  for (int i = 0; i < column_defs.size(); i++) {
    columns_map[column_defs[i].name] = column_defs[i];
    columns_idx_map[column_defs[i].name] = i;
  }

  if (columns_map.find(condition->column_name) == columns_map.end()) {
    throw std::runtime_error("Column doesn't exist");
  }

  int idx = columns_idx_map[condition->column_name];

  WhereClause where_condition = condition.value();

  table.mapOverRecords(
    [&](CellIndex i, TableLeafPage& page, TableLeafCell cell) {
      if (isWhereSatisfied(cell.row_data[idx], where_condition)) {
        page.deleteRecord(i);
        return CellIndex(i - 1);
      }
      return i;
    });
}

void UpdateCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

void CreateIndexCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

} // namespace white::davisbase::ast
