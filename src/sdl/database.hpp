#pragma once

#include <filesystem>
#include <memory>

#include "common.hpp"
#include "table.hpp"

namespace white::davisbase::sdl {

class Database
{
private:
  static constexpr auto TABLE_FILE_EXT = ".tbl";
  static constexpr auto INDEX_FILE_EXT = ".ndx";

  static constexpr PageLength DEFAULT_PAGE_LENGTH = 512;

  static constexpr PageNo ROOT_PAGE = 0;
  static constexpr PageCount INITIAL_PAGE_COUNT = 1;
  static constexpr RowId INITIAL_ROW_ID = 1;

  std::filesystem::path directory_path_;
  PageLength default_page_length_;

  struct Schema
  {
    Table tables;
    Table columns;
  } schema_;

  Schema initializeSchema();

  void getTableInfo(const std::string& table_name, PageNo& root_page_no,
                    PageCount& page_count, RowId& next_row_id,
                    PageLength& page_length);

  void getTableInfo(const std::string& table_name, PageNo& root_page_no,
                    PageCount& page_count, RowId& next_row_id,
                    PageLength& page_length, Table& tables_schema);

public:
  Database(
    std::filesystem::path directory_path = std::filesystem::current_path(),
    PageLength default_page_length = DEFAULT_PAGE_LENGTH);

  Table createTable(const std::string& table_name);
  Table getTable(const std::string& table_name);
  // void removeTable(std::string table_name);
};

} // namespace white::davisbase::sdl
