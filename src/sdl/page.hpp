#pragma once

#include <memory>

#include "common.hpp"

namespace white::davisbase::sdl {

class Table;

class Page
{
private:
  Table* table_;
  PageNo page_no_;
  std::unique_ptr<char[]> raw_data_;

protected:
  Page(Table& table, PageNo page_no, std::unique_ptr<char[]> raw_data);

  CellOffset cellOffset(CellIndex index) const;
  CellOffset cellContentAreaOffset() const;
  void deleteRecord(CellIndex index);

  void setCellOffset(CellIndex index, CellOffset offset);
  void setCellContentAreaOffset(CellOffset offset);
  void setCellCount(CellCount count);
  char* rawData();
  Table& table();
  const Table& table() const;

public:
  PageNo pageNo() const;
  const char* rawData() const;
  CellCount cellCount() const;

  void setPageNo(PageNo page_no);
  void setRawData(std::unique_ptr<char[]> raw_data);

  void commit();
  friend std::ostream& operator<<(std::ostream& os, const Page& page);
};

} // namespace white::davisbase::sdl
