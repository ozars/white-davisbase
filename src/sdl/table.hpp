#pragma once

#include <fstream>
#include <variant>

#include "column_value.hpp"
#include "common.hpp"
#include "page.hpp"

namespace white::davisbase::sdl {

using RowData = std::vector<ColumnValueVariant>;

struct TableInteriorCell
{
  PageNo left_child_page_no;
  RowId row_id;

  static constexpr PayloadLength length();
  void writeTo(char* addr) const;
  static TableInteriorCell readFrom(const char* addr);
};

struct TableLeafCellHeader
{
  PayloadLength payload_length;
  RowId row_id;

  static constexpr PayloadLength length();
  void writeTo(char* addr) const;
  static TableLeafCellHeader readFrom(const char* addr);
};

struct TableLeafCellPayload
{
  RowData row_data;

  PayloadLength length() const;
  void writeTo(char* addr) const;
  static TableLeafCellPayload readFrom(const char* addr);
};

struct TableLeafCell
  : public TableLeafCellHeader
  , public TableLeafCellPayload
{
  TableLeafCell(const TableLeafCellHeader& header,
                const TableLeafCellPayload& payload);

  PayloadLength length() const;
  void writeTo(char* addr) const;
  static TableLeafCell readFrom(const char* addr);
};

class TableInteriorPage : public Page
{
public:
  TableInteriorPage(Table& table, PageNo page_no,
                    std::unique_ptr<char[]> raw_data);

  RowId minRowId() const;

  PageNo rightmostChildPageNo() const;
  void setRightmostChildPageNo(PageNo rightmost_child_page_no);

  PageNo getChildPageNoByRowId(RowId row_id) const;

  bool hasEnoughSpace() const;

  TableInteriorCell getCell(CellIndex index) const;
  void appendCell(const TableInteriorCell& cell);

  std::optional<TableInteriorPage> appendRecord(const TableLeafCell& cell);

  static TableInteriorPage create(Table& table, PageNo page_no);
};

class TableLeafPage : public Page
{
public:
  TableLeafPage(Table& table, PageNo page_no, std::unique_ptr<char[]> raw_data);

  RowId minRowId() const;

  PageNo rightSiblingPageNo() const;
  void setRightSiblingPageNo(PageNo right_sibling_page_no);
  bool hasRightSiblingPage() const;
  TableLeafPage rightSiblingPage();

  bool hasEnoughSpace(const TableLeafCell& cell) const;

  TableLeafCell getCell(CellIndex index) const;
  void appendCell(const TableLeafCell& cell);

  void updateRecord(const TableLeafCell& cell);
  std::optional<TableLeafPage> appendRecord(const TableLeafCell& cell);

  static TableLeafPage create(Table& table, PageNo page_no);
};

class Database;

class Table
{
private:
  Database* database_;
  std::string name_;
  std::fstream file_;
  PageNo root_page_no_;
  RowId next_row_id_;
  PageCount page_count_;
  PageLength page_length_;

public:
  Table(Database& database, std::string name, std::fstream file,
        PageNo root_page_no, RowId next_row_id, PageCount page_count,
        PageLength page_length);

  PageNo rootPageNo() const;
  RowId nextRowId() const;
  PageLength pageLength() const;
  PageCount pageCount() const;

  void setRootPageNo(PageNo page_no);
  void setNextRowId(RowId next_row_id);
  void setPageCount(PageCount count);

  std::variant<TableInteriorPage, TableLeafPage> getPage(PageNo page_no);

  void appendRecord(const RowData& rows);
  void appendRecord(RowData&& rows);
  void appendRecord(const TableLeafCell& cell);
  void commitPage(const Page& page);

  TableLeafPage leftmostLeafPage();

  static Table create(Database& database, std::string name, std::fstream file,
                      RowId next_row_id, PageLength page_length);
};

} // namespace white::davisbase::sdl
