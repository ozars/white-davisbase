#include "table.hpp"

namespace white::davisbase::sdl {

constexpr PayloadLength TableInteriorCell::length()
{
  return sizeof(left_child_page_no) + sizeof(row_id);
}

void TableInteriorCell::writeTo(char* addr) const
{
  offset_cast<PageNo>(addr) = serialized(left_child_page_no);
  offset_cast<RowId>(addr, sizeof(left_child_page_no)) = serialized(row_id);
}

TableInteriorCell TableInteriorCell::readFrom(const char* addr)
{
  auto left_child_page_no = deserialized(offset_cast<PageNo>(addr));
  auto row_id =
    deserialized(offset_cast<RowId>(addr, sizeof(left_child_page_no)));
  return {left_child_page_no, row_id};
}

TableInteriorPage::TableInteriorPage(Table& table, PageNo page_no,
                                     std::unique_ptr<char[]> raw_data)
  : Page(table, page_no, std::move(raw_data))
{}

RowId TableInteriorPage::minRowId() const
{
  return getCell(0).row_id;
}

PageNo TableInteriorPage::rightmostChildPageNo() const
{
  return deserialized(offset_cast<PageNo>(rawData(), 0x05));
}

void TableInteriorPage::setRightmostChildPageNo(PageNo rightmost_child_page_no)
{
  offset_cast<PageNo>(rawData(), 0x05) = serialized(rightmost_child_page_no);
}

PageNo TableInteriorPage::getChildPageNoByRowId(RowId row_id) const
{
  auto count = cellCount();
  for (CellIndex i = 0; i < count; i++) {
    auto cell = getCell(i);
    if (row_id < cell.row_id)
      return cell.left_child_page_no;
  }
  return rightmostChildPageNo();
}

TableInteriorCell TableInteriorPage::getCell(CellIndex index) const
{
  auto offset = cellOffset(index);
  return TableInteriorCell::readFrom(rawData() + offset);
}

void TableInteriorPage::appendCell(const TableInteriorCell& cell)
{
  auto index = cellCount();
  auto offset = cellContentAreaOffset() - cell.length();

  if (offset + cell.length() > table().pageLength())
    throw std::logic_error("Cell offset overflows beyond page data");

  setCellCount(index + 1);
  setCellContentAreaOffset(offset);

  setCellOffset(index, offset);
  cell.writeTo(rawData() + offset);
}

bool TableInteriorPage::hasEnoughSpace() const
{
  constexpr auto space_for_new_record =
    sizeof(CellOffset) + TableInteriorCell::length();
  const auto end_of_header = 0x09 + sizeof(CellOffset) * cellCount();
  return end_of_header + space_for_new_record < cellContentAreaOffset();
}

std::optional<TableInteriorPage> TableInteriorPage::appendRecord(
  const TableLeafCell& leaf_cell)
{
  auto append_record_on_child =
    [&](auto&& child_page) -> std::optional<TableInteriorPage> {
    auto child_split_page_opt = child_page.appendRecord(leaf_cell);
    if (child_split_page_opt.has_value()) {
      auto& child_split_page = child_split_page_opt.value();
      auto cell =
        TableInteriorCell{rightmostChildPageNo(), child_split_page.minRowId()};
      if (hasEnoughSpace()) {
        appendCell(cell);
        setRightmostChildPageNo(child_split_page.pageNo());
        commit();
        return std::nullopt;
      }
      auto split_page_no = table().pageCount();
      auto split_page = TableInteriorPage::create(table(), split_page_no);

      split_page.appendCell(cell);
      split_page.setRightmostChildPageNo(child_split_page.pageNo());

      setRightmostChildPageNo(NULL_PAGE_NO);

      split_page.commit();
      commit();

      table().setPageCount(table().pageCount() + 1);

      return split_page;
    }
    return std::nullopt;
  };

  auto child_page_no = getChildPageNoByRowId(leaf_cell.row_id);
  auto child_page_variant = table().getPage(child_page_no);
  return std::visit(append_record_on_child, child_page_variant);
}

TableInteriorPage TableInteriorPage::create(Table& table, PageNo page_no)
{
  auto raw_data = std::make_unique<char[]>(table.pageLength());
  offset_cast<uint8_t>(raw_data.get()) =
    static_cast<uint8_t>(PageType::TABLE_INTERIOR);
  TableInteriorPage page(table, page_no, std::move(raw_data));
  page.setCellCount(0);
  page.setCellContentAreaOffset(table.pageLength());
  page.setRightmostChildPageNo(NULL_PAGE_NO);
  return page;
}

std::ostream& operator<<(std::ostream& os, const TableInteriorPage& page)
{
  return os << "TableInteriorPage(" << static_cast<const Page&>(page)
            << ", rightmost_child_page_no=" << page.rightmostChildPageNo()
            << ")";
}

} // namespace white::davisbase::sdl
