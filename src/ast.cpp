#include "ast.hpp"
#include "sdl/table.hpp"

#include <iostream>

#include "sdl/database.hpp"

namespace white::davisbase::ast {

using common::NullValue;
using sdl::ColumnValueVariant;
using sdl::Database;
using sdl::TableLeafCell;

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
  std::cout << *this << std::endl;
}

void UpdateCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
  bool found = false;
  bool is_col_unique = false;
  bool can_update = true;

  auto table_opt = database.getTable(table_name);
  //if(!table_opt.has_value())
  if(table_opt==std::nullopt)
   throw std::runtime_error("The table doesn't exist");
  auto& table = table_opt.value();
  auto& col_def = table.columnDefinitions();
  
  std::optional<size_t> column_index_opt, column_where_index_opt;

  if (condition.has_value()) {
    auto& cond = condition.value();
    for(size_t i=0; i<col_def.size(); i++)
    {
      if(cond.column_name == col_def[i].name)
          column_where_index_opt = i;
    }

    if(!column_where_index_opt.has_value())
      throw std::runtime_error("The column doesn't exist in the table");
  }
  
  for(size_t i=0; i<col_def.size(); i++)
  {
    if(column_name == col_def[i].name){
      if (col_def[i].modifiers.unique || col_def[i].modifiers.primary_key)
        is_col_unique = true;
      column_index_opt = i;
    }
  }

  if(!column_index_opt.has_value())
   throw std::runtime_error("The column doesn't exist in the table");

  auto& column_index = column_index_opt.value();
  auto& column_where_index = column_where_index_opt.value();

  table.mapOverRecords([&](TableLeafCell cell){
    auto& cell_row_data = cell.row_data;
    if(cell_row_data[column_index]==value)
      can_update = false;
  });

  table.mapOverRecords([&](TableLeafPage& page, TableLeafCell record){
    auto& row_data = record.row_data;
    if (!condition.has_value() || isWhereSatisfied(row_data[column_where_index], condition.value()))
    {
      if(can_update)
      {
        row_data[column_index] = value;
        page.updateRecord(record);
      }
      else
          throw std::runtime_error("The column is unique and the value already exists in the table");   
    }
  });
}

void CreateIndexCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

} // namespace white::davisbase::ast
