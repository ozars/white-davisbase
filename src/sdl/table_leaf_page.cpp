#include "table.hpp"

namespace white::davisbase::sdl {

TableLeafPage::TableLeafPage(Table& table, PageNo page_no,
                             std::unique_ptr<char[]> raw_data)
  : Page(table, page_no, std::move(raw_data))
{}

constexpr PayloadLength TableLeafCellHeader::length()
{
  return sizeof(payload_length) + sizeof(row_id);
}

void TableLeafCellHeader::writeTo(char* addr) const
{
  offset_cast<PayloadLength>(addr) = serialized(payload_length);
  offset_cast<RowId>(addr, sizeof(payload_length)) = serialized(row_id);
}

TableLeafCellHeader TableLeafCellHeader::readFrom(const char* addr)
{
  auto payload_length = deserialized(offset_cast<PayloadLength>(addr));
  auto row_id = deserialized(offset_cast<RowId>(addr, sizeof(payload_length)));
  return {payload_length, row_id};
}

PayloadLength TableLeafCellPayload::length() const
{
  using common::ColumnType;
  using common::NullValue;

  auto get_length_of_data = [](auto&& val) -> size_t {
    using T = std::decay_t<decltype(val)>;
    if constexpr (std::is_same_v<T, NullValue>)
      return 0;
    else if constexpr (T::column_type == ColumnType::TEXT)
      return val.get().size();
    else
      return sizeof(typename T::underlying_type);
  };

  size_t length = 0;
  for (auto& column_val : row_data)
    length += std::visit(get_length_of_data, column_val);

  return length;
}

void TableLeafCellPayload::writeTo(char* addr) const
{
  using common::ColumnType;
  using common::NullValue;
  using common::SerialTypeCode;

  auto write_type = [&addr](auto&& val) {
    using T = std::decay_t<decltype(val)>;
    if constexpr (std::is_same_v<T, NullValue>) {
      *addr++ = static_cast<uint8_t>(SerialTypeCode::NULL_TYPE);
    } else if constexpr (T::column_type == ColumnType::TEXT) {
      *addr++ = static_cast<uint8_t>(SerialTypeCode::TEXT) + val.get().size();
    } else {
      *addr++ = static_cast<uint8_t>(T::typecode);
    }
  };

  auto write_value = [&addr](auto&& val) {
    using T = std::decay_t<decltype(val)>;
    if constexpr (std::is_same_v<T, NullValue>) {
      // empty body for null values
    } else if constexpr (T::column_type == ColumnType::TEXT) {
      auto begin = val.get().cbegin();
      auto end = val.get().cend();
      std::copy(begin, end, addr);
      addr += val.get().size();
    } else {
      constexpr auto bytes = sizeof(typename T::underlying_type);
      offset_cast<typename T::underlying_type>(addr) = serialized(val.get());
      addr += bytes;
    }
  };

  *addr++ = row_data.size();

  for (auto& column_val : row_data)
    std::visit(write_type, column_val);

  for (auto& column_val : row_data)
    std::visit(write_value, column_val);
}

TableLeafCellPayload TableLeafCellPayload::readFrom(const char* addr)
{
  using common::ColumnType;
  using common::NullValue;
  using common::SerialTypeCode;

  std::vector<uint8_t> column_types;
  TableLeafCellPayload cell;

  uint8_t num_cols = *addr++;

  column_types.reserve(num_cols);
  cell.row_data.reserve(num_cols);

  for (int i = 0; i < num_cols; i++)
    column_types.push_back(*addr++);

  for (auto typecode : column_types) {
    switch (SerialTypeCode(typecode)) {
      case SerialTypeCode::NULL_TYPE:
        cell.row_data.emplace_back(NullValue());
        break;
      case SerialTypeCode::TINYINT: {
        using ColumnValue = TinyIntColumnValue;
        auto val =
          deserialized(offset_cast<ColumnValue::underlying_type>(addr));
        cell.row_data.emplace_back(ColumnValue(val));
        addr += sizeof(val);
        break;
      }
      case SerialTypeCode::SMALLINT: {
        using ColumnValue = SmallIntColumnValue;
        auto val =
          deserialized(offset_cast<ColumnValue::underlying_type>(addr));
        cell.row_data.emplace_back(ColumnValue(val));
        addr += sizeof(val);
        break;
      }
      case SerialTypeCode::INT: {
        using ColumnValue = IntColumnValue;
        auto val =
          deserialized(offset_cast<ColumnValue::underlying_type>(addr));
        cell.row_data.emplace_back(ColumnValue(val));
        addr += sizeof(val);
        break;
      }
      case SerialTypeCode::BIGINT: {
        using ColumnValue = BigIntColumnValue;
        auto val =
          deserialized(offset_cast<ColumnValue::underlying_type>(addr));
        cell.row_data.emplace_back(ColumnValue(val));
        addr += sizeof(val);
        break;
      }
      case SerialTypeCode::FLOAT: {
        using ColumnValue = FloatColumnValue;
        auto val =
          deserialized(offset_cast<ColumnValue::underlying_type>(addr));
        cell.row_data.emplace_back(ColumnValue(val));
        addr += sizeof(val);
        break;
      }
      case SerialTypeCode::YEAR: {
        using ColumnValue = YearColumnValue;
        auto val =
          deserialized(offset_cast<ColumnValue::underlying_type>(addr));
        cell.row_data.emplace_back(ColumnValue(val));
        addr += sizeof(val);
        break;
      }
      case SerialTypeCode::TIME: {
        using ColumnValue = TimeColumnValue;
        auto val =
          deserialized(offset_cast<ColumnValue::underlying_type>(addr));
        cell.row_data.emplace_back(ColumnValue(val));
        addr += sizeof(val);
        break;
      }
      case SerialTypeCode::DATETIME: {
        using ColumnValue = DateTimeColumnValue;
        auto val =
          deserialized(offset_cast<ColumnValue::underlying_type>(addr));
        cell.row_data.emplace_back(ColumnValue(val));
        addr += sizeof(val);
        break;
      }
      case SerialTypeCode::DATE: {
        using ColumnValue = DateColumnValue;
        auto val =
          deserialized(offset_cast<ColumnValue::underlying_type>(addr));
        cell.row_data.emplace_back(ColumnValue(val));
        addr += sizeof(val);
        break;
      }
      default:
        if (SerialTypeCode(typecode) < SerialTypeCode::TEXT) {
          throw std::runtime_error("Unexpected serial type code value");
        }
        using ColumnValue = TextColumnValue;
        auto len = typecode - enum_cast(SerialTypeCode::TEXT);
        auto val = ColumnValue::underlying_type(addr, len);
        cell.row_data.emplace_back(ColumnValue(val));
        addr += len;
    }
  }
  return cell;
}

TableLeafCell::TableLeafCell(const TableLeafCellHeader& header,
                             const TableLeafCellPayload& payload)
  : TableLeafCellHeader(header)
  , TableLeafCellPayload(payload)
{}

PayloadLength TableLeafCell::length() const
{
  return TableLeafCellHeader::length() + TableLeafCellPayload::length();
}

void TableLeafCell::writeTo(char* addr) const
{
  TableLeafCellHeader::writeTo(addr);
  TableLeafCellPayload::writeTo(addr + TableLeafCellHeader::length());
}

TableLeafCell TableLeafCell::readFrom(const char* addr)
{
  return {TableLeafCellHeader::readFrom(addr),
          TableLeafCellPayload::readFrom(addr + TableLeafCellHeader::length())};
}

RowId TableLeafPage::minRowId() const
{
  return getCell(0).row_id;
}

PageNo TableLeafPage::rightSiblingPageNo() const
{
  return deserialized(offset_cast<PageNo>(rawData(), 0x05));
}

void TableLeafPage::setRightSiblingPageNo(PageNo right_sibling_page_no)
{
  offset_cast<PageNo>(rawData(), 0x05) = serialized(right_sibling_page_no);
}

bool TableLeafPage::hasRightSiblingPage() const
{
  return rightSiblingPageNo() != NULL_PAGE_NO;
}

TableLeafPage TableLeafPage::rightSiblingPage()
{
  auto page_no = rightSiblingPageNo();
  if (page_no == NULL_PAGE_NO)
    throw std::runtime_error("There is no right sibling page");
  return std::get<TableLeafPage>(table().getPage(page_no));
}

TableLeafCell TableLeafPage::getCell(CellIndex index) const
{
  auto offset = cellOffset(index);
  return TableLeafCell::readFrom(rawData() + offset);
}

void TableLeafPage::appendCell(const TableLeafCell& cell)
{
  auto index = cellCount();
  auto offset = cellContentAreaOffset() - cell.length();

  setCellCount(index + 1);
  setCellContentAreaOffset(offset);

  setCellOffset(index, offset);
  cell.writeTo(rawData() + offset);
}

bool TableLeafPage::hasEnoughSpace(const TableLeafCell& cell) const
{
  const auto space_for_new_record = sizeof(CellOffset) + cell.length();
  const auto end_of_header = 0x09 + sizeof(CellOffset) * cellCount();
  return end_of_header + space_for_new_record < cellContentAreaOffset();
}

void TableLeafPage::updateRecord(const TableLeafCell&)
{
  /* TODO */
}

std::optional<TableLeafPage> TableLeafPage::appendRecord(
  const TableLeafCell& cell)
{
  if (hasEnoughSpace(cell)) {
    appendCell(cell);
    commit();
    return std::nullopt;
  }

  auto split_page_no = table().pageCount();
  auto split_page = TableLeafPage::create(table(), split_page_no);

  setRightSiblingPageNo(split_page_no);

  if (!split_page.hasEnoughSpace(cell))
    throw std::runtime_error("Cell doesn't fit into empty page");

  split_page.appendRecord(cell);
  split_page.setRightSiblingPageNo(NULL_PAGE_NO);

  split_page.commit();
  commit();

  table().setPageCount(table().pageCount() + 1);

  return split_page;
}

TableLeafPage TableLeafPage::create(Table& table, PageNo page_no)
{
  auto raw_data = std::make_unique<char[]>(table.pageLength());
  offset_cast<uint8_t>(raw_data.get()) =
    serialized(static_cast<uint8_t>(PageType::TABLE_LEAF));
  TableLeafPage page(table, page_no, std::move(raw_data));
  page.setCellCount(0);
  page.setCellContentAreaOffset(table.pageLength());
  page.setRightSiblingPageNo(NULL_PAGE_NO);
  return page;
}

} // namespace white::davisbase::sdl
