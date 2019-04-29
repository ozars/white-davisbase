#include "page.hpp"

#include <iomanip>
#include <ostream>

#include "table.hpp"

namespace white::davisbase::sdl {

/* protected: */

Page::Page(Table& table, PageNo page_no, std::unique_ptr<char[]> raw_data)
  : table_(&table)
  , page_no_(page_no)
  , raw_data_(std::move(raw_data))
{}

CellOffset Page::cellOffset(CellIndex index) const
{
  if (index >= cellCount())
    throw std::logic_error("Requested cell index is out of range for the page");

  auto offset =
    deserialized(reinterpret_cast<const CellOffset*>(rawData() + 0x09)[index]);

  if (offset >= table().pageLength())
    throw std::logic_error(
      "Cell offset for requested index is beyond page boundaries");

  return offset;
}

CellOffset Page::cellContentAreaOffset() const
{
  return deserialized(offset_cast<CellOffset>(rawData(), 0x03));
}

void Page::deleteRecord(CellIndex index)
{
  if (index >= cellCount())
    throw std::logic_error("Requested cell index is out of range for the page");

  auto cell_address = reinterpret_cast<CellOffset*>(rawData() + 0x09) + index;
  std::copy(cell_address + 1, cell_address + cellCount() - index, cell_address);
  setCellCount(cellCount() - 1);
  commit();
}

void Page::setCellOffset(CellIndex index, CellOffset offset)
{
  if (index >= cellCount())
    throw std::logic_error("Requested cell index is out of range for the page");

  if (offset >= table().pageLength())
    throw std::logic_error(
      "Cell offset for requested index is beyond page boundaries");

  reinterpret_cast<CellOffset*>(rawData() + 0x09)[index] = serialized(offset);
}

void Page::setCellContentAreaOffset(CellOffset offset)
{
  offset_cast<CellOffset>(rawData(), 0x03) = serialized(offset);
}

void Page::setCellCount(CellCount count)
{
  offset_cast<CellOffset>(rawData(), 0x01) = serialized(count);
}

char* Page::rawData()
{
  return raw_data_.get();
}

Table& Page::table()
{
  return *table_;
}

const Table& Page::table() const
{
  return *table_;
}

/* public: */

PageNo Page::pageNo() const
{
  return page_no_;
}

const char* Page::rawData() const
{
  return raw_data_.get();
}

CellCount Page::cellCount() const
{
  return deserialized(offset_cast<CellOffset>(rawData(), 0x01));
}

void Page::setPageNo(PageNo page_no)
{
  page_no_ = page_no;
}

void Page::setRawData(std::unique_ptr<char[]> raw_data)
{
  raw_data_ = std::move(raw_data);
}

void Page::commit()
{
  table().commitPage(*this);
}

std::ostream& operator<<(std::ostream& os, const Page& page)
{
  return os << "Page(type=" << std::hex << std::setfill('0') << std::setw(2)
            << int(page.rawData()[0]) << std::dec << std::setfill(' ')
            << ", page_no=" << page.pageNo()
            << ", cell_count=" << page.cellCount()
            << ", content_area_offset=" << page.cellContentAreaOffset() << ")";
}

} // namespace white::davisbase::sdl
