#include "database.hpp"

#include <array>
#include <filesystem>
#include <tuple>

namespace white::davisbase::sdl {

namespace fs = std::filesystem;

static inline common::ColumnDefinitions tables_schema_column_definitions()
{
  using common::ColumnModifiers;
  using common::ColumnType;
  return {
    {"table_name", ColumnType::TEXT, ColumnModifiers()},
    {"root_page_no", ColumnType::INT, ColumnModifiers()},
    {"page_count", ColumnType::INT, ColumnModifiers()},
    {"next_row_id", ColumnType::INT, ColumnModifiers()},
    {"page_length", ColumnType::SMALLINT, ColumnModifiers()},
  };
}

static inline common::ColumnDefinitions columns_schema_column_definitions()
{
  using common::ColumnModifiers;
  using common::ColumnType;
  return {
    {"table_name", ColumnType::TEXT, ColumnModifiers()},
    {"column_name", ColumnType::TEXT, ColumnModifiers()},
    {"data_type", ColumnType::TINYINT, ColumnModifiers()},
    {"ordinal_position", ColumnType::TINYINT, ColumnModifiers()},
    {"is_nullable", ColumnType::TINYINT, ColumnModifiers()},
    {"is_primary", ColumnType::TINYINT, ColumnModifiers()},
    {"is_unique", ColumnType::TINYINT, ColumnModifiers()},
  };
}

template<typename T>
static inline auto open_new_file(T&& path)
{
  using std::ios;
  return std::fstream(std::forward<T>(path),
                      ios::in | ios::out | ios::binary | ios::trunc);
}

template<typename T>
static inline auto open_existing_file(T&& path)
{
  using std::ios;
  return std::fstream(std::forward<T>(path), ios::in | ios::out | ios::binary);
}

Database::Database(std::filesystem::path directory_path,
                   PageLength default_page_length)
  : bootstrapping_schema_(true)
  , directory_path_(
      fs::exists(directory_path) && fs::is_directory(directory_path)
        ? directory_path
        : throw std::runtime_error(
            "Database directory not found or it's not a directory."))
  , default_page_length_(default_page_length)
  , schema_{initializeSchema()}
{
  bootstrapping_schema_ = false;
}

Database::Schema Database::initializeSchema()
{
  static const auto tables_schema_name = std::string("davisbase_tables");
  static const auto tables_path =
    directory_path_ / (tables_schema_name + TABLE_FILE_EXT);

  static const auto columns_schema_name = std::string("davisbase_columns");
  static const auto columns_path =
    directory_path_ / (columns_schema_name + TABLE_FILE_EXT);

  if (fs::exists(tables_path) && fs::exists(columns_path) &&
      fs::is_regular_file(tables_path) && fs::is_regular_file(columns_path)) {

    auto tables_schema =
      Table(*this, tables_schema_name, open_existing_file(tables_path), 0, 0, 0,
            default_page_length_, tables_schema_column_definitions());

    auto columns_schema =
      Table(*this, columns_schema_name, open_existing_file(columns_path), 0, 0,
            0, default_page_length_, columns_schema_column_definitions());

    PageNo root_page_no;
    PageCount page_count;
    RowId next_row_id;
    PageLength page_length;

    getTableInfo(tables_schema_name, root_page_no, page_count, next_row_id,
                 page_length, tables_schema);

    tables_schema =
      Table(*this, tables_schema_name, open_existing_file(tables_path),
            root_page_no, next_row_id, page_count, page_length,
            getColumnsInfo(tables_schema_name, columns_schema));

    getTableInfo(columns_schema_name, root_page_no, page_count, next_row_id,
                 page_length, tables_schema);

    columns_schema =
      Table(*this, columns_schema_name, open_existing_file(columns_path),
            root_page_no, next_row_id, page_count, page_length,
            getColumnsInfo(columns_schema_name, columns_schema));

    return {std::move(tables_schema), std::move(columns_schema)};
  }

  using common::ColumnDefinitions;
  using common::ColumnType;
  using std::get;
  using std::make_tuple;
  using util::make_array;

  auto tables_file = open_new_file(tables_path);
  if (!tables_file)
    throw std::runtime_error("Couldn't create tables schema file");

  auto columns_file = open_new_file(columns_path);
  if (!columns_file)
    throw std::runtime_error("Couldn't create columns schema file");

  auto tables_schema = Table::create(
    *this, tables_schema_name, std::move(tables_file), INITIAL_ROW_ID,
    default_page_length_, tables_schema_column_definitions());

  auto columns_schema = Table::create(
    *this, columns_schema_name, std::move(columns_file), INITIAL_ROW_ID,
    default_page_length_, columns_schema_column_definitions());

  auto tables_columns = tables_schema_column_definitions();
  auto columns_columns = columns_schema_column_definitions();

  tables_schema.appendRecord(
    {tables_schema_name, 0, 0, 0, default_page_length_});

  tables_schema.appendRecord(
    {columns_schema_name, 0, 0, 0, default_page_length_});

  for (size_t i = 0; i < tables_columns.size(); i++) {
    auto& col = tables_columns[i];
    columns_schema.appendRecord(
      {tables_schema_name, col.name, col.type, i + 1, col.modifiers.is_null,
       col.modifiers.primary_key, col.modifiers.unique});
  }

  for (size_t i = 0; i < columns_columns.size(); i++) {
    auto& col = columns_columns[i];
    columns_schema.appendRecord(
      {columns_schema_name, col.name, col.type, i + 1, col.modifiers.is_null,
       col.modifiers.primary_key, col.modifiers.unique});
  }

  tables_schema.mapOverRecords([&](TableLeafPage& page, TableLeafCell cell) {
    using std::get;
    if (get<TextColumnValue>(cell.row_data[0]).get() == tables_schema_name) {
      get<IntColumnValue>(cell.row_data[1]) = tables_schema.rootPageNo();
      get<IntColumnValue>(cell.row_data[2]) = tables_schema.pageCount();
      get<IntColumnValue>(cell.row_data[3]) = tables_schema.nextRowId();
      page.updateRecord(cell);
    } else if (get<TextColumnValue>(cell.row_data[0]).get() ==
               columns_schema_name) {
      get<IntColumnValue>(cell.row_data[1]) = columns_schema.rootPageNo();
      get<IntColumnValue>(cell.row_data[2]) = columns_schema.pageCount();
      get<IntColumnValue>(cell.row_data[3]) = columns_schema.nextRowId();
      page.updateRecord(cell);
    }
  });

  return {std::move(tables_schema), std::move(columns_schema)};
}

void Database::getTableInfo(const std::string& table_name, PageNo& root_page_no,
                            PageCount& page_count, RowId& next_row_id,
                            PageLength& page_length)
{
  getTableInfo(table_name, root_page_no, page_count, next_row_id, page_length,
               schema_.tables);
}

void Database::getTableInfo(const std::string& table_name, PageNo& root_page_no,
                            PageCount& page_count, RowId& next_row_id,
                            PageLength& page_length, Table& tables_schema)
{
  bool found = false;

  tables_schema.mapOverRecords([&](TableLeafCell cell) {
    using std::get;
    if (get<TextColumnValue>(cell.row_data[0]).get() == table_name) {
      root_page_no = get<IntColumnValue>(cell.row_data[1]).get();
      page_count = get<IntColumnValue>(cell.row_data[2]).get();
      next_row_id = get<IntColumnValue>(cell.row_data[3]).get();
      page_length = get<SmallIntColumnValue>(cell.row_data[4]).get();
      found = true;
      return false;
    }
    return true;
  });

  if (!found)
    throw std::runtime_error("Table entry is not found on schema");
}

common::ColumnDefinitions Database::getColumnsInfo(
  const std::string& table_name)
{
  return getColumnsInfo(table_name, schema_.columns);
}

common::ColumnDefinitions Database::getColumnsInfo(
  const std::string& table_name, Table& columns_schema)
{
  common::ColumnDefinitions column_definitions;

  columns_schema.mapOverRecords([&](TableLeafCell cell) {
    using std::get;
    using common::ColumnType;
    auto& row_data = cell.row_data;
    if (get<TextColumnValue>(cell.row_data[0]).get() == table_name) {
      auto& col_def = column_definitions.emplace_back();
      col_def.name = get<TextColumnValue>(row_data[1]).get();
      col_def.type =
        static_cast<ColumnType>(get<TinyIntColumnValue>(row_data[2]).get());
      col_def.modifiers.is_null = get<TinyIntColumnValue>(row_data[4]).get();
      col_def.modifiers.not_null = !get<TinyIntColumnValue>(row_data[4]).get();
      col_def.modifiers.primary_key =
        get<TinyIntColumnValue>(row_data[5]).get();
      col_def.modifiers.unique = get<TinyIntColumnValue>(row_data[6]).get();
    }
  });

  return column_definitions;
}

void Database::updatePageCount(const std::string& table_name,
                               PageCount page_count)
{
  if (bootstrapping_schema_)
    return;
  using std::get;
  mapOverTables([&](TableLeafPage& page, TableLeafCell cell) {
    if (get<TextColumnValue>(cell.row_data[0]).get() == table_name) {
      get<IntColumnValue>(cell.row_data[2]) = page_count;
      page.updateRecord(cell);
      return false;
    }
    return true;
  });
}

void Database::updateNextRowId(const std::string& table_name, RowId next_row_id)
{
  if (bootstrapping_schema_)
    return;
  using std::get;
  mapOverTables([&](TableLeafPage& page, TableLeafCell cell) {
    if (get<TextColumnValue>(cell.row_data[0]).get() == table_name) {
      get<IntColumnValue>(cell.row_data[3]) = next_row_id;
      page.updateRecord(cell);
      return false;
    }
    return true;
  });
}

Table Database::createTable(const std::string& table_name,
                            common::ColumnDefinitions&& column_definitions)
{
  const auto path = directory_path_ / (table_name + TABLE_FILE_EXT);

  if (fs::exists(path))
    throw std::runtime_error("Table file already exists");

  auto file = open_new_file(path);
  if (!file)
    throw std::runtime_error("Couldn't create table file");

  auto table =
    Table::create(*this, table_name, std::move(file), INITIAL_ROW_ID,
                  default_page_length_, std::move(column_definitions));

  schema_.tables.appendRecord({table_name, table.rootPageNo(),
                               table.pageCount(), table.nextRowId(),
                               table.pageLength()});

  for (size_t i = 0; i < table.columnDefinitions().size(); i++) {
    auto& col = table.columnDefinitions()[i];
    schema_.columns.appendRecord(
      {table_name, col.name, col.type, i + 1, !col.modifiers.not_null,
       col.modifiers.primary_key, col.modifiers.unique});
  }
  return table;
}

std::optional<Table> Database::getTable(const std::string& table_name)
{
  const auto path = directory_path_ / (table_name + TABLE_FILE_EXT);

  bool found = false;
  schema_.tables.mapOverRecords([&](TableLeafCell cell) {
    if (std::get<TextColumnValue>(cell.row_data[0]).get() == table_name) {
      found = true;
      return false;
    }
    return true;
  });

  if (!found)
    return std::nullopt;

  if (!fs::exists(path))
    throw std::runtime_error("Couldn't find table file");
  if (!fs::is_regular_file(path))
    throw std::runtime_error("Table file is not a regular file");

  auto file = open_existing_file(path);
  if (!file)
    throw std::runtime_error("Couldn't open table file");

  PageNo root_page_no;
  PageCount page_count;
  RowId next_row_id;
  PageLength page_length;

  getTableInfo(table_name, root_page_no, page_count, next_row_id, page_length);

  return Table(*this, table_name, std::move(file), root_page_no, next_row_id,
               page_count, page_length, getColumnsInfo(table_name));
}

void Database::removeTable(std::string table_name)
{
  const auto path = directory_path_ / (table_name + TABLE_FILE_EXT);

  if (!fs::exists(path))
    throw std::runtime_error("Couldn't find table file");
  if (!fs::is_regular_file(path))
    throw std::runtime_error("Table file is not a regular file");

  schema_.tables.mapOverRecords(
    [&](CellIndex i, TableLeafPage& page, TableLeafCell cell) {
      if (std::get<TextColumnValue>(cell.row_data[0]).get() == table_name) {
        page.deleteRecord(i);
        return page.cellCount();
      }
      return i;
    });
  schema_.columns.mapOverRecords(
    [&](CellIndex i, TableLeafPage& page, TableLeafCell cell) {
      if (std::get<TextColumnValue>(cell.row_data[0]).get() == table_name) {
        page.deleteRecord(i);
        return CellIndex(i - 1);
      }
      return i;
    });

  fs::remove(path);
}

void Database::makeColumnUnique(std::string table_name, std::string column_name)
{
  bool found = false;
  schema_.columns.mapOverRecords([&](TableLeafPage& page, TableLeafCell cell) {
    if (std::get<TextColumnValue>(cell.row_data[0]) == table_name &&
        std::get<TextColumnValue>(cell.row_data[1]) == column_name) {
      found = true;
      auto& is_unique = std::get<TinyIntColumnValue>(cell.row_data[6]);
      if (!is_unique.get()) {
        is_unique = 1;
        page.updateRecord(cell);
      }
      return false;
    }
    return true;
  });
  if (!found)
    throw std::runtime_error("Column not found");
}

} // namespace white::davisbase::sdl
