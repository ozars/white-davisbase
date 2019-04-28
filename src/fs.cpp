#include <memory>

#include "fs.hpp"

namespace white::davisbase::fs {

template <typename T>
T read_date(const void *

struct Cell::impl {
  Page& page;
  const char* base_addr;
};

Cell::Cell(Page& page, const char* base_addr) : pimpl(base_addr ? std::make_unique<impl>(page, base_addr) : throw std::runtime_error("Null value was provided for base address of a cell")){
}

PageNo Cell::leftChildPageNo() const {
  if (pimpl->page.isLeaf())
    throw std::runtime_error(
      "Left child page no is valid only for cells in interior pages");
  return *reinterpret_cast<PageNo*>(pimpl->base_addr);
}

PayloadSize Cell::payloadSize() const {
  if (pimpl->page.isTable()) {
    if (pimpl->page.isLeaf())
      return *reinterpret_cast<PayloadSize*> else throw std::runtime_error(
        "Payload size is not valid for interior table pages");
  if (pimpl->page.
  }
  RowId Cell::rowid() const {}

  IndexData Cell::indexData() const {}
  RowData Cell::rowData() const {}

  void Cell::writeIndexData(const IndexData& index_data) {}
  void Cell::writeRowData(const RowData& row_data) {}

  struct Page::impl
  {};

  Page::Page(Table & table, std::unique_ptr<char[]> raw_data,
             PageNo parent_page_no)
  {}

  PageNo Page::pageNo() const {}
  PageNo Page::parentPageNo() const {}
  PageNo Page::rightmostChildPageNo() const {}
  PageNo Page::rightSiblingPageNo() const {}

  void Page::setPageNo(PageNo page_no) {}
  void Page::setParentPageNo(PageNo parent_page_no) {}
  void Page::setRightmostChildPageNo(PageNo rightmost_child_page_no) {}
  void Page::setRightSiblingPageNo(PageNo right_sibling_page_no) {}

  bool Page::isRoot() const {}
  bool Page::isLeaf() const {}
  bool Page::isTable() const {}

  int Page::cellCount() const {}
  Cell Page::getCell(int cell_index) const {}

  bool Page::hasEnoughSpace(const RowData& row_data) {}
  Cell Page::addCell(const RowData& row_data, RowId row_id) {}

  bool Page::hasEnoughSpace() {}
  Cell Page::addCell(PageNo left_child_page, RowId row_id) {}

  bool Page::hasEnoughSpace(const IndexData& index_data) {}
  Cell Page::addCell(const IndexData& index_data, PageNo left_child_page_no) {}

  const char* Page::rawData() const {}
  void Page::commit() {}

  struct Table::impl
  {
  }

  Table::Table(Database & database, std::string name, std::istream file,
               PageNo root_page_no, RowId next_row_id, PageSize page_size)
  {}

  PageNo Table::rootPageNo() const {}
  RowId Table::nextRowId() const {}
  PageSize Table::pageSize() const {}
  PageCount Table::pageCount() const {}

  Page Table::getPage(PageNo page_no) const {}

  void Table::commitPage(const Page& page) {}
  void Table::setRootPageNo(PageNo page_no) {}
  void Table::setNextRowId(RowId next_row_id) {}

  static Page Table::createInteriorPage(
    PageNo page_no, PageNo rightmost_child_page_no, PageNo parent_page_no)
  {}

  static Page Table::createLeafPage(
    PageNo page_no, PageNo right_sibling_page_no, PageNo parent_page_no)
  {}

  struct Database::impl
  {};
  Database::Database(std::string directory_path, PageSize default_page_size) {}

  Table Database::createTable(std::string table_name) {}
  Table Database::getTable(std::string table_name) const {}
  void Database::removeTable(std::string table_name) {}

} // namespace white::davisbase::fs
