#pragma once

#include <fstream>
#include <variant>

#include "column_value.hpp"
#include "common.hpp"
#include "page.hpp"

namespace white::davisbase::sdl {

std::ostream& operator<<(std::ostream& os, const RowData& row_data);

struct TableInteriorCell
{
  PageNo left_child_page_no;
  RowId row_id;

  static constexpr PayloadLength length();
  void writeTo(char* addr) const;
  static TableInteriorCell readFrom(const char* addr);

  friend std::ostream& operator<<(std::ostream& os,
                                  const TableInteriorCell& cell);
};

struct TableLeafCellHeader
{
  PayloadLength payload_length;
  RowId row_id;

  static constexpr PayloadLength length();
  void writeTo(char* addr) const;
  static TableLeafCellHeader readFrom(const char* addr);

  friend std::ostream& operator<<(std::ostream& os,
                                  const TableLeafCellHeader& header);
};

struct TableLeafCellPayload
{
  RowData row_data;

  PayloadLength length() const;
  void writeTo(char* addr) const;
  static TableLeafCellPayload readFrom(const char* addr);

  friend std::ostream& operator<<(std::ostream& os,
                                  const TableLeafCellPayload& payload);
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

  friend std::ostream& operator<<(std::ostream& os, const TableLeafCell& cell);
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

  friend std::ostream& operator<<(std::ostream& os,
                                  const TableInteriorPage& page);
};

class TableLeafPage : public Page
{
private:
  TableLeafCell getCellByOffset(CellOffset offset) const;

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
  using Page::deleteRecord;

  void updateRecord(const TableLeafCell& cell);
  std::optional<TableLeafPage> appendRecord(const TableLeafCell& cell);

  template<typename Mapper>
  bool mapOverRecords(Mapper&& mapper);

  static TableLeafPage create(Table& table, PageNo page_no);

  friend std::ostream& operator<<(std::ostream& os, const TableLeafPage& page);
};

template<typename Mapper>
bool TableLeafPage::mapOverRecords(Mapper&& mapper)
{
  auto call_mapper = [&]([[maybe_unused]] CellIndex i, auto&& cell) {
    if constexpr (std::is_invocable_v<Mapper, CellIndex, TableLeafPage&,
                                      TableLeafCell>) {
      if constexpr (std::is_same_v<void, std::invoke_result_t<Mapper, CellIndex,
                                                              TableLeafPage&,
                                                              TableLeafCell>>)
        mapper(i, *this, std::forward<decltype(cell)>(cell));
      else
        return mapper(i, *this, std::forward<decltype(cell)>(cell));
    } else if constexpr (std::is_invocable_v<Mapper, TableLeafPage&,
                                             TableLeafCell>) {
      if constexpr (std::is_same_v<void,
                                   std::invoke_result_t<Mapper, TableLeafPage&,
                                                        TableLeafCell>>)
        mapper(*this, std::forward<decltype(cell)>(cell));
      else
        return mapper(*this, std::forward<decltype(cell)>(cell));
    } else if constexpr (std::is_invocable_v<Mapper, CellIndex,
                                             TableLeafCell>) {
      if constexpr (std::is_same_v<void, std::invoke_result_t<Mapper, CellIndex,
                                                              TableLeafCell>>)
        mapper(i, std::forward<decltype(cell)>(cell));
      else
        return mapper(i, std::forward<decltype(cell)>(cell));
    } else {
      static_assert(std::is_invocable_v<Mapper, TableLeafCell>);
      if constexpr (std::is_same_v<void,
                                   std::invoke_result_t<Mapper, TableLeafCell>>)
        mapper(std::forward<decltype(cell)>(cell));
      else
        return mapper(std::forward<decltype(cell)>(cell));
    }
  };

  CellIndex i;

  for (i = 0; i < cellCount(); i++) {
    using MappingReturnType =
      std::invoke_result_t<decltype(call_mapper), CellIndex, TableLeafCell&&>;
    if constexpr (std::is_same_v<MappingReturnType, void>) {
      call_mapper(i, getCell(i));
    } else if constexpr (std::is_same_v<MappingReturnType, bool>) {
      if (!call_mapper(i, getCell(i)))
        return false;
    } else if constexpr (std::is_same_v<MappingReturnType, CellIndex>) {
      i = call_mapper(i, getCell(i));
    } else {
      throw std::logic_error("Inapplicable mapper function");
    }
  }
  if (i > cellCount())
    return false;
  return true;
}

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
  common::ColumnDefinitions column_definitions_;

  void appendRecord(const TableLeafCell& cell);
  TableLeafPage leftmostLeafPage();
  TableLeafPage leafPageByRowId(RowId row_id);

public:
  Table(Database& database, std::string name, std::fstream file,
        PageNo root_page_no, RowId next_row_id, PageCount page_count,
        PageLength page_length, common::ColumnDefinitions&& column_definitions);

  PageNo rootPageNo() const;
  RowId nextRowId() const;
  PageLength pageLength() const;
  PageCount pageCount() const;
  const common::ColumnDefinitions& columnDefinitions() const;

  void setRootPageNo(PageNo page_no);
  void setNextRowId(RowId next_row_id);
  void setPageCount(PageCount count);

  std::variant<TableInteriorPage, TableLeafPage> getPage(PageNo page_no);

  void appendRecord(const RowData& rows);
  void appendRecord(RowData&& rows);
  void appendRecord(const std::vector<common::LiteralValue>& values);
  void commitPage(const Page& page);

  template<typename Mapper>
  void mapOverLeafPages(Mapper&& mapper);

  template<typename Mapper>
  void mapOverLeafPages(TableLeafPage&& initialPage, Mapper&& mapper);

  template<typename Mapper>
  void mapOverRecords(Mapper&& mapper);

  template<typename Mapper>
  void mapOverRecords(TableLeafPage&& initialPage, Mapper&& mapper);

  static Table create(Database& database, std::string name, std::fstream file,
                      RowId next_row_id, PageLength page_length,
                      common::ColumnDefinitions&& column_definitions);

  friend std::ostream& operator<<(std::ostream& os, const Table& table);
};

template<typename Mapper>
void Table::mapOverLeafPages(Mapper&& mapper)
{
  mapOverLeafPages(leftmostLeafPage(), std::forward<Mapper>(mapper));
}

template<typename Mapper>
void Table::mapOverLeafPages(TableLeafPage&& page, Mapper&& mapper)
{
  while (true) {
    if (!mapper(page) || !page.hasRightSiblingPage())
      return;
    page = page.rightSiblingPage();
  }
}

template<typename Mapper>
void Table::mapOverRecords(Mapper&& mapper)
{
  mapOverLeafPages([&](auto&& page) { return page.mapOverRecords(mapper); });
}

template<typename Mapper>
void Table::mapOverRecords(TableLeafPage&& initialPage, Mapper&& mapper)
{
  mapOverLeafPages(std::move(initialPage),
                   [&](auto&& page) { return page.mapOverRecords(mapper); });
}

} // namespace white::davisbase::sdl
