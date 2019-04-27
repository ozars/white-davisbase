#include "database.hpp"

#include <array>
#include <filesystem>
#include <tuple>

namespace white::davisbase::sdl {

namespace fs = std::filesystem;

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
  : directory_path_(
      fs::exists(directory_path) && fs::is_directory(directory_path)
        ? directory_path
        : throw std::runtime_error(
            "Database directory not found or it's not a directory."))
  , default_page_length_(default_page_length)
  , schema_{initializeSchema()}
{}

Database::Schema Database::initializeSchema()
{
  /* davisbase_tables
   *
   * table_name TEXT,
   * root_page_no INT,
   * page_count INT,
   * next_row_id INT,
   * page_length SMALLINT
   *
   * davisbase_columns
   *
   * table_name TEXT,
   * column_name TEXT,
   * data_type TINYINT,
   * ordinal_position TINYINT,
   * is_nullable TINYINT,
   * is_primary TINYINT
   */

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
            default_page_length_);

    auto columns_schema =
      Table(*this, columns_schema_name, open_existing_file(tables_path), 0, 0,
            0, default_page_length_);

    PageNo root_page_no;
    PageCount page_count;
    RowId next_row_id;
    PageLength page_length;

    getTableInfo(tables_schema_name, root_page_no, page_count, next_row_id,
                 page_length, tables_schema);

    tables_schema.setRootPageNo(root_page_no);
    tables_schema.setPageCount(page_count);
    tables_schema.setNextRowId(next_row_id);

    getTableInfo(columns_schema_name, root_page_no, page_count, next_row_id,
                 page_length, tables_schema);

    columns_schema.setRootPageNo(root_page_no);
    columns_schema.setPageCount(page_count);
    columns_schema.setNextRowId(next_row_id);

    return {std::move(tables_schema), std::move(columns_schema)};
  }

  using common::ColumnType;
  using std::get;
  using std::make_tuple;
  using util::make_array;

  // clang-format off
  static const auto default_columns = make_array(
    make_tuple(1, tables_schema_name, "table_name", ColumnType::TEXT, 1, false, false),
    make_tuple(2, tables_schema_name, "root_page_no", ColumnType::INT, 2, false, false),
    make_tuple(3, tables_schema_name, "page_count", ColumnType::INT, 3, false, false),
    make_tuple(4, tables_schema_name, "next_row_id", ColumnType::INT, 4, false, false),
    make_tuple(5, tables_schema_name, "page_length", ColumnType::SMALLINT, 5, false, false),
    make_tuple(6, columns_schema_name, "table_name", ColumnType::TEXT, 1, false, false),
    make_tuple(7, columns_schema_name, "column_name", ColumnType::TEXT, 2, false, false),
    make_tuple(8, columns_schema_name, "data_type", ColumnType::TINYINT, 3, false, false),
    make_tuple(9, columns_schema_name, "ordinal_position", ColumnType::TINYINT, 4, false, false),
    make_tuple(10, columns_schema_name, "is_nullable", ColumnType::TINYINT, 5, false, false),
    make_tuple(11, columns_schema_name, "is_primary", ColumnType::TINYINT, 6, false, false)
  );

  static const auto default_tables = make_array(
    make_tuple(1, tables_schema_name, 3),
    make_tuple(2, columns_schema_name, 12)
  );
  // clang-format on

  auto append_table_to_schema = [](auto& table, auto&& table_info) {
    constexpr auto ROOT_PAGE = 0;
    constexpr auto PAGE_COUNT = 1;
    auto& row_id = get<0>(table_info);
    auto& table_name = get<1>(table_info);
    auto& next_row_id = get<2>(table_info);

    TableLeafCellPayload payload{
      RowData{TextColumnValue(table_name), IntColumnValue(ROOT_PAGE),
              IntColumnValue(PAGE_COUNT), IntColumnValue(next_row_id),
              SmallIntColumnValue(table.pageLength())}};

    TableLeafCell cell({payload.length(), row_id}, payload);

    table.appendRecord(cell);
  };

  auto append_column_to_schema = [](auto& table, auto&& column_info) {
    auto& row_id = get<0>(column_info);
    auto& table_name = get<1>(column_info);
    auto& column_name = get<2>(column_info);
    auto data_type =
      static_cast<TinyIntColumnValue::underlying_type>(get<3>(column_info));
    auto& ordinal_position = get<4>(column_info);
    auto& is_nullable = get<5>(column_info);
    auto& is_primary = get<6>(column_info);

    TableLeafCellPayload payload{RowData{
      TextColumnValue(table_name), TextColumnValue(column_name),
      TinyIntColumnValue(data_type), TinyIntColumnValue(ordinal_position),
      TinyIntColumnValue(is_nullable), TinyIntColumnValue(is_primary)}};

    TableLeafCell cell({payload.length(), row_id}, payload);

    table.appendRecord(cell);
  };

  auto tables_file = open_new_file(tables_path);
  if (!tables_file)
    throw std::runtime_error("Couldn't create tables schema file");

  auto columns_file = open_new_file(columns_path);
  if (!columns_file)
    throw std::runtime_error("Couldn't create columns schema file");

  auto tables_schema =
    Table::create(*this, tables_schema_name, std::move(tables_file),
                  default_tables.size(), default_page_length_);

  auto columns_schema =
    Table::create(*this, columns_schema_name, std::move(columns_file),
                  default_columns.size(), default_page_length_);

  for (auto& table_info : default_tables)
    append_table_to_schema(tables_schema, table_info);

  for (auto& column_info : default_columns)
    append_column_to_schema(columns_schema, column_info);

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
  using std::get;

  root_page_no = 0;

  auto page = tables_schema.leftmostLeafPage();
  while (true) {
    for (CellIndex i = 0; i < page.cellCount(); i++) {
      auto cell = page.getCell(i);
      if (get<TextColumnValue>(cell.row_data[0]).get() == table_name) {
        page_count = get<IntColumnValue>(cell.row_data[2]).get();
        next_row_id = get<IntColumnValue>(cell.row_data[3]).get();
        page_length = get<SmallIntColumnValue>(cell.row_data[4]).get();
        return;
      }
    }
    if (!page.hasRightSiblingPage())
      throw std::runtime_error("Table entry is not found on schema");
    page = page.rightSiblingPage();
  }
}

Table Database::createTable(const std::string& table_name)
{
  static const auto path = directory_path_ / (table_name + TABLE_FILE_EXT);

  if (fs::exists(path))
    throw std::runtime_error("Table file already exists");

  auto file = open_new_file(path);
  if (!file)
    throw std::runtime_error("Couldn't create table file");

  auto table = Table::create(*this, table_name, std::move(file), INITIAL_ROW_ID,
                             default_page_length_);

  RowData row_data{
    TextColumnValue(table_name), IntColumnValue(table.rootPageNo()),
    IntColumnValue(table.pageCount()), IntColumnValue(table.nextRowId()),
    SmallIntColumnValue(table.pageLength())};

  schema_.tables.appendRecord(row_data);

  return table;
}

Table Database::getTable(const std::string& table_name)
{
  static const auto path = directory_path_ / (table_name + TABLE_FILE_EXT);

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
               page_count, page_length);
}

// void Database::removeTable(string table_name) {}

} // namespace white::davisbase::sdl
