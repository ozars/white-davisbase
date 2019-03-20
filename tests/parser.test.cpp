#include <array>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <variant>

#include "catch2/catch.hpp"

#include "ast.cpp"
#include "parser.cpp"

#include "utility.test.hpp"

using std::get;
using std::holds_alternative;
using std::string;

using white::davisbase::ast::Command;
using white::davisbase::parser::Parser;

TEST_CASE("Parse showing tables", "[parser][show_tables]")
{
  using white::davisbase::ast::ShowTablesCommand;

  Command parsed_cmd;
  ShowTablesCommand actual_cmd;

  auto sql = string() + "SHOW TABLES;";

  INFO("Parsing SQL '" << sql << "'");
  REQUIRE_NOTHROW(parsed_cmd = Parser().parse(sql));

  REQUIRE(holds_alternative<ShowTablesCommand>(parsed_cmd.command));
  REQUIRE_NOTHROW(actual_cmd = get<ShowTablesCommand>(parsed_cmd.command));
}

TEST_CASE("Parse dropping a table", "[parser][drop_table]")
{
  using white::davisbase::ast::DropTableCommand;

  Command parsed_cmd;
  DropTableCommand actual_cmd;

  constexpr const char* table_name = "test_123";

  auto sql = string() + "DROP TABLE " + table_name + ";";

  INFO("Parsing SQL '" << sql << "'");
  REQUIRE_NOTHROW(parsed_cmd = Parser().parse(sql));

  REQUIRE(holds_alternative<DropTableCommand>(parsed_cmd.command));
  REQUIRE_NOTHROW(actual_cmd = get<DropTableCommand>(parsed_cmd.command));

  REQUIRE(actual_cmd.table_name == table_name);
}

TEST_CASE("Parse creating table", "[parser][create_table]")
{
  using white::davisbase::ast::ColumnType;
  using white::davisbase::ast::CreateTableCommand;

  Parser parser;
  Command parsed_cmd;
  CreateTableCommand actual_cmd;

  constexpr const char* table_name = "test_123";
  constexpr const char* column_name_prefix = "column";

  SECTION("Require at least one column")
  {
    auto sql = GENERATE(as<std::string>{}, "CREATE TABLE test ();",
                        "CREATE TABLE test ( );", "CREATE TABLE test;");

    INFO("Parsing SQL '" << sql << "'");
    REQUIRE_THROWS(parsed_cmd = parser.parse(sql));
  }

  SECTION("Parse one column with any type")
  {
    auto type = GENERATE(enum_range(ColumnType::_FIRST, ColumnType::_LAST));

    auto sql = string() + "CREATE TABLE " + table_name + " (" +
               column_name_prefix + " " + to_string(type) + ");";

    INFO("Parsing SQL '" << sql << "'");
    REQUIRE_NOTHROW(parsed_cmd = parser.parse(sql));

    REQUIRE(holds_alternative<CreateTableCommand>(parsed_cmd.command));
    REQUIRE_NOTHROW(actual_cmd = get<CreateTableCommand>(parsed_cmd.command));

    CHECK(actual_cmd.table_name == table_name);
    REQUIRE(actual_cmd.columns.size() == 1);

    auto& column = actual_cmd.columns[0];

    CHECK(column.name == column_name_prefix);
    CHECK(column.type == type);

    CHECK_FALSE(column.modifiers.primary_key.has_value());
    CHECK_FALSE(column.modifiers.unique.has_value());
    CHECK_FALSE(column.modifiers.is_null.has_value());
    CHECK_FALSE(column.modifiers.not_null.has_value());
    CHECK_FALSE(column.modifiers.auto_increment.has_value());
    CHECK_FALSE(column.modifiers.default_value.has_value());
  }

  SECTION("Combinations of columns with each type")
  {
    constexpr int column_count = 3;

    std::array<ColumnType, column_count> types;

    for (auto& type : types)
      type = GENERATE(enum_range(ColumnType::_FIRST, ColumnType::_LAST));

    std::array<string, column_count> each_column_sql;

    for (int i = 0; i < column_count; i++)
      each_column_sql[i] =
        column_name_prefix + std::to_string(i) + " " + to_string(types[i]);

    auto column_sql = join(each_column_sql, string(", "));
    auto sql = string("CREATE TABLE ") + table_name + " (" + column_sql + ");";

    INFO("Parsing SQL '" << sql << "'");
    REQUIRE_NOTHROW(parsed_cmd = parser.parse(sql));

    REQUIRE(holds_alternative<CreateTableCommand>(parsed_cmd.command));
    REQUIRE_NOTHROW(actual_cmd = get<CreateTableCommand>(parsed_cmd.command));

    CHECK(actual_cmd.table_name == table_name);
    REQUIRE(actual_cmd.columns.size() == column_count);

    for (int i = 0; i < column_count; i++) {
      auto& column = actual_cmd.columns[i];

      CHECK(column.name == column_name_prefix + std::to_string(i));
      CHECK(column.type == types[i]);

      CHECK_FALSE(column.modifiers.primary_key.has_value());
      CHECK_FALSE(column.modifiers.unique.has_value());
      CHECK_FALSE(column.modifiers.is_null.has_value());
      CHECK_FALSE(column.modifiers.not_null.has_value());
      CHECK_FALSE(column.modifiers.auto_increment.has_value());
      CHECK_FALSE(column.modifiers.default_value.has_value());
    }
  }

  SECTION("One column")
  {
    parsed_cmd = parser.parse("CREATE TABLE test (id INT);");
  }
}
