# Example Interface

```cpp
{
  using namespace white::davisbase::sdl;
  auto table_opt = database.getTable("test");
  if (!table_opt.has_value()) {
    auto table = database.createTable("test");
    table_opt = std::move(table);
  }
  auto& table = table_opt.value();
  std::cerr << table << std::endl;
  for (int i = 0; i < 100; i++) {
    table.appendRecord(RowData{TextColumnValue("<RECORD>"),
                                TinyIntColumnValue(65),
                                TextColumnValue("</RECORD>")});
  }

  /* Variation 1 */
  table.mapOverRecords(
    [&](TableLeafCell record) {
      std::cerr << record << std::endl;
    });

  /* Variation 2 */
  table.mapOverRecords([&](TableLeafPage& page, TableLeafCell record) {
    std::cerr << record << std::endl;
    std::cerr << page << std::endl;
  });

  /* Variation 3 */
  table.mapOverRecords([&](CellIndex i, TableLeafPage& page, TableLeafCell record) {
    std::cerr << record << std::endl;
    std::cerr << page << std::endl;
  });
}
```

