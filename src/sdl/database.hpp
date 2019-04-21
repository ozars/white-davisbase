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
  static constexpr RowId INITIAL_ROW_ID = 1;
  static constexpr PageLength DEFAULT_PAGE_LENGTH = 512;

  std::filesystem::path directory_path_;
  PageLength default_page_length_;

  struct Schema
  {
    Table tables;
    Table columns;
  } schema_;

  Schema initializeSchema();

public:
  Database(
    std::filesystem::path directory_path = std::filesystem::current_path(),
    PageLength default_page_length = DEFAULT_PAGE_LENGTH);

  // Table createTable(std::string table_name);
  Table getTable(std::string table_name);
  // void removeTable(std::string table_name);
};

} // namespace white::davisbase::sdl
