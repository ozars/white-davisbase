#include "ast.hpp"

#include <iostream>
#include <vector>

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
  database.createTable(table_name, std::move(columns));
}

void InsertIntoCommand::execute(Database& database)
{
  using sdl::RowData;

  auto table = database.getTable(table_name);

  if (!table.has_value())
    throw std::runtime_error("Table doesn't exist");

  auto& column_defs = table->columnDefinitions();
  RowData new_row_data;

  if (column_names.size() > 0) {
    if (column_names.size() != values.size())
      throw std::runtime_error(
        "Column names list should have same length with values list");

    for (size_t i = 0; i < column_defs.size(); i++)
      new_row_data.push_back(NullValue());

    for (size_t j = 0; j < column_names.size(); j++) {
      bool found = false;
      for (size_t i = 0; i < column_defs.size() && !found; i++) {
        if (column_names[j] == column_defs[i].name) {
          new_row_data[i] = createColumnValue(column_defs[i].type, values[j]);
          found = true;
        }
      }
      if (!found)
        throw std::runtime_error("Column name not found");
    }

  } else { // if column names aren't explicitly given
    if (column_defs.size() != values.size())
      throw std::runtime_error(
        "Number of given values doesn't match number of columns");

    new_row_data = createRowData(column_defs, values);
  }

  /* Null check */
  for (size_t i = 0; i < column_defs.size(); i++) {
    if (column_defs[i].modifiers.not_null &&
        std::holds_alternative<NullValue>(new_row_data[i])) {
      throw std::runtime_error("Null value for non-null column");
    }
  }

  /* Uniquness check */
  table->mapOverRecords([&](TableLeafCell cell) {
    for (size_t i = 0; i < column_defs.size(); i++) {
      if (column_defs[i].modifiers.primary_key ||
          column_defs[i].modifiers.unique) {
        if (new_row_data[i] == cell.row_data[i]) {
          throw std::runtime_error("Unique key violation");
        }
      }
    }
  });

  table->appendRecord(new_row_data);
}

void SelectCommand::execute(Database& database)
{
  auto table = database.getTable(table_name);

  if (!table.has_value())
    throw std::runtime_error("Table not found");

  auto& column_defs = table->columnDefinitions();

  std::optional<size_t> where_column_idx;

  if (condition.has_value()) {
    for (size_t i = 0; i < column_defs.size(); i++) {
      if (condition->column_name == column_defs[i].name) {
        where_column_idx = i;
        break;
      }
    }
    if (!where_column_idx.has_value())
      throw std::runtime_error("Column name is not known in where clause");

    /* This is just to throw if value cannot be constructed from literal. */
    createColumnValue(column_defs[*where_column_idx].type, condition->literal);
  }

  if (column_names.size() == 0) {

    std::cout << "rowid|"
              << util::join(table->columnDefinitions(), "|",
                            [](auto& col_def) { return col_def.name; })
              << std::endl;

    if (!condition.has_value()) {
      table->mapOverRecords([&](TableLeafCell cell) {
        std::cout << cell.row_id << "|" << util::join(cell.row_data, "|")
                  << std::endl;
      });
    } else {
      table->mapOverRecords([&](TableLeafCell cell) {
        if (isWhereSatisfied(cell.row_data[*where_column_idx], *condition)) {
          std::cout << cell.row_id << "|" << util::join(cell.row_data, "|")
                    << std::endl;
        }
      });
    }

  } else {
    std::vector<size_t> column_idx;

    for (size_t i = 0; i < column_names.size(); i++) {
      bool found = false;
      for (size_t idx = 0; idx < column_defs.size(); idx++) {
        if (column_names[i] == column_defs[idx].name) {
          column_idx.push_back(idx);
          found = true;
          break;
        }
      }
      if (!found)
        throw std::runtime_error("Requested column name is not found");
    }

    std::cout << "rowid|" << util::join(column_names, "|") << std::endl;

    if (!condition.has_value()) {
      table->mapOverRecords([&](TableLeafCell cell) {
        std::cout << cell.row_id;
        for (size_t i = 0; i < column_idx.size(); i++)
          std::cout << "|" << cell.row_data[column_idx[i]];
        std::cout << std::endl;
      });
    } else {
      table->mapOverRecords([&](TableLeafCell cell) {
        if (isWhereSatisfied(cell.row_data[*where_column_idx], *condition)) {
          std::cout << cell.row_id;
          for (size_t i = 0; i < column_idx.size(); i++)
            std::cout << "|" << cell.row_data[column_idx[i]];
          std::cout << std::endl;
        }
      });
    }
  }
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
