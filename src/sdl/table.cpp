#include "table.hpp"

#include "database.hpp"

namespace white::davisbase::sdl {

Table::Table(Database& database, std::string name, std::fstream file,
             PageNo root_page_no, RowId next_row_id, PageCount page_count,
             PageLength page_length)
  : database_(&database)
  , name_(std::move(name))
  , file_(std::move(file))
  , root_page_no_(root_page_no)
  , next_row_id_(next_row_id)
  , page_count_(page_count)
  , page_length_(page_length)
{}

PageNo Table::rootPageNo() const
{
  return root_page_no_;
}

RowId Table::nextRowId() const
{
  return next_row_id_;
}

PageLength Table::pageLength() const
{
  return page_length_;
}

PageCount Table::pageCount() const
{
  return page_count_;
}

void Table::setRootPageNo(PageNo page_no)
{
  root_page_no_ = page_no;
}

void Table::setNextRowId(RowId next_row_id)
{
  next_row_id_ = next_row_id;
}

void Table::setPageCount(PageCount count)
{
  page_count_ = count;
}

std::variant<TableInteriorPage, TableLeafPage> Table::getPage(PageNo page_no)
{
  auto raw_data = std::make_unique<char[]>(page_length_);

  file_.seekg(page_no * page_length_);
  file_.read(raw_data.get(), page_length_);
  if (file_.eof())
    throw std::runtime_error("EOF reached while reading page from file");
  if (file_.fail())
    throw std::runtime_error("Error while reading page from file");

  switch (PageType(deserialized(raw_data[0]))) {
    case PageType::TABLE_LEAF:
      return TableLeafPage(*this, page_no, std::move(raw_data));
    case PageType::TABLE_INTERIOR:
      return TableInteriorPage(*this, page_no, std::move(raw_data));
    default:
      throw std::runtime_error("Unknown page type was read for a table");
  }
}

void Table::appendRecord(const TableLeafCell& cell)
{
  auto append_record = [&](auto& current_root_page) {
    auto new_page_optional = current_root_page.appendRecord(cell);
    if (new_page_optional.has_value()) {
      auto& new_page = new_page_optional.value();
      auto new_root = TableInteriorPage::create(*this, pageCount());
      new_root.appendCell({current_root_page.pageNo(), new_page.minRowId()});
      new_root.setRightmostChildPageNo(new_page.pageNo());
      new_root.commit();
      setRootPageNo(new_root.pageNo());
      setPageCount(pageCount() + 1);
    }
  };

  auto root_page_variant = getPage(rootPageNo());
  std::visit(append_record, root_page_variant);
}

void Table::appendRecord(const RowData& rows)
{
  const TableLeafCellPayload payload{rows};
  const auto payload_length = payload.length();
  const TableLeafCellHeader header{payload_length, nextRowId()};

  appendRecord(TableLeafCell(header, payload));
}

void Table::appendRecord(RowData&& rows)
{
  const TableLeafCellPayload payload{std::move(rows)};
  const auto payload_length = payload.length();
  const TableLeafCellHeader header{payload_length, nextRowId()};

  appendRecord(TableLeafCell(header, payload));
}

void Table::commitPage(const Page& page)
{
  file_.seekp(page.pageNo() * page_length_);
  file_.write(page.rawData(), page_length_);
  // file_.flush();
  if (file_.fail())
    throw std::runtime_error("Error while writing page to file");
}

TableLeafPage Table::leftmostLeafPage()
{
  using std::holds_alternative;
  using std::get;

  auto page = getPage(rootPageNo());
  while (holds_alternative<TableInteriorPage>(page))
    page = getPage(get<TableInteriorPage>(page).getCell(0).left_child_page_no);
  return std::move(get<TableLeafPage>(page));
}

Table Table::create(Database& database, std::string name, std::fstream file,
                    RowId next_row_id, PageLength page_length)
{
  auto table =
    Table(database, name, std::move(file), 0, next_row_id, 1, page_length);
  TableLeafPage::create(table, 0).commit();
  return table;
}

} // namespace white::davisbase::sdl
