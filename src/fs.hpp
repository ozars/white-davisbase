#pragma once

#include <string>
#include <memory>

#include "ast.hpp"

namespace std {
  template<class T>
  struct less;
}

namespace white::davisbase::fs {

class Table
{
private:
  struct impl;
  std::unique_ptr<impl> pimpl;

public:
  Table(std::string table_name);
  ~Table();
  Table(Table&) = delete;
  Table(Table&&) = default;
  Table& operator=(Table&) = delete;
  Table& operator=(Table&&) = default;

  /**
   * Creates table definition in the schema, table file, initializes its
   * content. Throws exception if table already exists.
   */
  void createTable();

  /**
   * Removes table file. Throws exception if table doesn't exist.
   */
  void removeTable();

  /**
   * Removes all entries in a table.
   */
  void truncateTable();

  void updateRow(std::string column_name, );

  std::string getTableName() const;
  void setTableName(std::string table_name);
};

class Index {

};

} // namespace white::davisbase::fs
