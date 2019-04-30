#include "ast.hpp"

#include <iostream>

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
  using namespace white::davisbase::sdl;
  using common::NullValue;
  using std::holds_alternative;

  auto table_opt = database.getTable(table_name);
  if (!table_opt.has_value()) {
    throw std::runtime_error("Table doesn't exist");
  }

  auto& table = table_opt.value();
  auto column_defs = table.columnDefinitions();
  std::map<std::string, common::ColumnDefinition> columns_map;

  for (int i = 0; i < column_defs.size(); i++) {
    columns_map[column_defs[i].name] = column_defs[i];
  }

  if (column_names.size() > 0) {
    for (int i = 0; i < column_names.size(); i++) {

      if (columns_map.find(column_names[i]) != columns_map.end()) {

        auto col_val_variant =
          createColumnValue(columns_map[column_names[i]].type, values[i]);

        // checking if the column allows nulls if the value is null
        if (holds_alternative<NullValue>(col_val_variant) &&
            !columns_map[column_names[i]].modifiers.is_null) {
          throw std::runtime_error(column_names[i] + " column cannot be null");
        }
        // checking for uniqueness if the column is PK or unique
        if (columns_map[column_names[i]].modifiers.primary_key ||
            columns_map[column_names[i]].modifiers.unique) {
          bool unique = true;
          int col_index = 0;
          for (int j = 0; j < column_defs.size(); j++) {
            if (column_defs[j].name == columns_map[column_names[i]].name) {
              col_index = j;
              break;
            }
          }
          table.mapOverRecords([&](TableLeafCell record) {
            if (record.row_data[col_index] == col_val_variant) {
              unique = false;
              return false;
            }
          });
          if (!unique)
            throw std::runtime_error(column_names[i] +
                                     " value of the column is not unique");
        }
      }
    }
  } else {
    // if no column names
    for (int i = 0; i < column_defs.size(); i++) {
      auto col_val_variant = createColumnValue(column_defs[i].type, values[i]);
      // checking if the column allows nulls if the value is null
      if (holds_alternative<NullValue>(col_val_variant) &&
          !column_defs[i].modifiers.is_null) {
        throw std::runtime_error(column_names[i] + " column cannot be null");
      }

      // checking for uniqueness if the column is PK or unique
      if (column_defs[i].modifiers.primary_key ||
          column_defs[i].modifiers.unique) {
        bool unique = true;

        table.mapOverRecords([&](TableLeafCell record) {
          if (record.row_data[i] == col_val_variant) {
            unique = false;
            return false;
          }
        });

        if (!unique)
          throw std::runtime_error(column_names[i] +
                                   " value of the column is not unique");
      }
    }
  }

  table.appendRecord(values);
}

void SelectCommand::execute(Database& database)
{
  // using namespace white::davisbase::sdl;
  //   using common::NullValue;
  //   using std::holds_alternative;

  //   auto table_opt = database.getTable(table_name);
  //    if (!table_opt.has_value()) {
  //     throw std::runtime_error("Table doesn't exist");
  //   }

  //   auto& table = table_opt.value();
  //   auto column_defs = table.columnDefinitions();
  // std::cout << *this << std::endl;
}

void DeleteFromCommand::execute(Database& database)
{
  auto table = database.getTable(table_name);

  if (!table.has_value())
    throw std::runtime_error("Table doesn't exist");

  if (condition == std::nullopt)
    throw std::runtime_error("Where clause required");

  auto& column_defs = table->columnDefinitions();
  std::optional<size_t> idx;

  for (size_t i = 0; i < column_defs.size(); i++)
    if (column_defs[i].name == condition->column_name)
      idx = i;

  if (!idx.has_value())
    throw std::runtime_error("Column doesn't exist");

  table->mapOverRecords(
    [&](CellIndex i, TableLeafPage& page, TableLeafCell cell) {
      if (isWhereSatisfied(cell.row_data[*idx], *condition)) {
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
