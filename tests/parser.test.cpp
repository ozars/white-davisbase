#include <array>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <variant>

#include "catch2/catch.hpp"

#include "ast.cpp"
#include "parser.cpp"

#include "utility.test.hpp"

using std::array;
using std::get;
using std::holds_alternative;
using std::string;
using std::tuple;

using white::davisbase::ast::Command;
using white::davisbase::parser::Parser;
using white::util::join;
using white::util::make_array;

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

  SECTION("Type tests")
  {

    SECTION("Simple cases")
    {
      CHECK_NOTHROW(parser.parse("CREATE TABLE test (id INT);"));
      CHECK_NOTHROW(parser.parse("CREATE TABLE test(id INT);"));
      CHECK_NOTHROW(parser.parse("CREATE TABLE test(id int);"));
      CHECK_NOTHROW(parser.parse("Create Table test(id int);"));
      CHECK_THROWS(parser.parse("CREATE TABLE test (id INT)"));
    }

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
      auto sql =
        string("CREATE TABLE ") + table_name + " (" + column_sql + ");";

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
  }

  SECTION("Modifier tests")
  {
    using white::davisbase::ast::Column;
    using white::davisbase::ast::ColumnModifiers;

    using modifiers_list_t = tuple<string, std::function<bool(const Column&)>>;
    auto modifiers_list = make_array<modifiers_list_t>(
      tuple{string("PRIMARY KEY"),
            [](const Column& column) {
              return column.modifiers.primary_key.has_value();
            }},
      tuple{string("AUTOINCREMENT"),
            [](const Column& column) {
              return column.modifiers.auto_increment.has_value();
            }},
      tuple{string("NOT NULL"),
            [](const Column& column) {
              return column.modifiers.not_null.has_value();
            }},
      tuple{string("NULL"),
            [](const Column& column) {
              return column.modifiers.is_null.has_value();
            }},
      tuple{string("DEFAULT 'omer'"),
            [](const Column& column) {
              auto& def = column.modifiers.default_value;
              return def.has_value() &&
                     holds_alternative<string>(def.value().literal.value) &&
                     get<string>(def.value().literal.value) == "omer";
            }},
      tuple{string("DEFAULT '\\'omer\\''"),
            [](const Column& column) {
              auto& def = column.modifiers.default_value;
              return def.has_value() &&
                     holds_alternative<string>(def.value().literal.value) &&
                     get<string>(def.value().literal.value) == "'omer'";
            }},
      tuple{string("DEFAULT \"ozarslan\""),
            [](const Column& column) {
              auto& def = column.modifiers.default_value;
              return def.has_value() &&
                     holds_alternative<string>(def.value().literal.value) &&
                     get<string>(def.value().literal.value) == "ozarslan";
            }},
      tuple{string("DEFAULT \"\\\"ozarslan\\\"\""),
            [](const Column& column) {
              auto& def = column.modifiers.default_value;
              return def.has_value() &&
                     holds_alternative<string>(def.value().literal.value) &&
                     get<string>(def.value().literal.value) == "\"ozarslan\"";
            }},
      tuple{string("DEFAULT 1337"),
            [](const Column& column) {
              auto& def = column.modifiers.default_value;
              return def.has_value() &&
                     holds_alternative<long long>(def.value().literal.value) &&
                     get<long long>(def.value().literal.value) == 1337;
            }},
      tuple{string("DEFAULT 13.37"),
            [](const Column& column) {
              auto& def = column.modifiers.default_value;
              return def.has_value() &&
                     holds_alternative<long double>(
                       def.value().literal.value) &&
                     get<long double>(def.value().literal.value) == 13.37L;
            }},
      tuple{string("UNIQUE"), [](const Column& column) {
              return column.modifiers.unique.has_value();
            }});

    SECTION("Simple cases")
    {
      REQUIRE_NOTHROW(parser.parse("CREATE TABLE test (id INT PRIMARY KEY);"));

      REQUIRE_NOTHROW(
        parser.parse("CREATE TABLE test (id INT PRIMARY KEY UNIQUE);"));

      REQUIRE_NOTHROW(
        parser.parse("CREATE TABLE test (id INT NOT NULL NULL);"));

      REQUIRE_NOTHROW(
        parser.parse("CREATE TABLE test (id INT NULL NOT NULL);"));

      REQUIRE_NOTHROW(
        parser.parse("CREATE TABLE test (id INT NULL NOT NULL PRIMARY KEY "
                     "AUTOINCREMENT UNIQUE DEFAULT 3);"));

      REQUIRE_NOTHROW(
        parser.parse("CREATE TABLE test (id INT NULL NOT NULL PRIMARY KEY "
                     "AUTOINCREMENT UNIQUE DEFAULT 3.14);"));
      REQUIRE_NOTHROW(
        parser.parse("CREATE TABLE test (id INT NULL NOT NULL PRIMARY KEY "
                     "AUTOINCREMENT UNIQUE DEFAULT 'omer');"));
      REQUIRE_NOTHROW(
        parser.parse("CREATE TABLE test (id INT NULL NOT NULL PRIMARY KEY "
                     "AUTOINCREMENT UNIQUE DEFAULT \"omer\");"));
    }

    SECTION("Combinations of types and modifiers in one column")
    {
      auto type = GENERATE(enum_range(ColumnType::_FIRST, ColumnType::_LAST));
      auto ind = GENERATE(
        range<decltype(modifiers_list)::size_type>(0, modifiers_list.size()));
      auto& modifier = modifiers_list[ind];
      auto& modifier_str = get<0>(modifier);

      auto sql = string() + "CREATE TABLE " + table_name + " (" +
                 column_name_prefix + " " + to_string(type) + " " +
                 modifier_str + ");";

      INFO("Parsing SQL '" << sql << "'");
      REQUIRE_NOTHROW(parsed_cmd = parser.parse(sql));

      REQUIRE(holds_alternative<CreateTableCommand>(parsed_cmd.command));
      REQUIRE_NOTHROW(actual_cmd = get<CreateTableCommand>(parsed_cmd.command));

      CHECK(actual_cmd.table_name == table_name);
      REQUIRE(actual_cmd.columns.size() == 1);

      auto& column = actual_cmd.columns[0];

      CHECK(column.name == column_name_prefix);
      CHECK(column.type == type);

      for (auto& possible_modifier : modifiers_list) {
        INFO("Inspecting modifier " << get<0>(possible_modifier));
        auto modifier_check = get<1>(possible_modifier);
        if (&possible_modifier == &modifier)
          CHECK(modifier_check(column));
        else
          CHECK_FALSE(modifier_check(column));
      }
    }

    SECTION("Combinations of types and modifiers in multiple columns")
    {
      /* Note that the number of poissble cases increases n^this number. */
      constexpr int column_count = 2;

      std::array<ColumnType, column_count> types;
      std::array<int, column_count> modifier_indices;

      for (auto& type : types)
        type = GENERATE(enum_range(ColumnType::_FIRST, ColumnType::_LAST));

      for (auto& ind : modifier_indices)
        ind = GENERATE(
          range<decltype(modifiers_list)::size_type>(0, modifiers_list.size()));

      std::array<string, column_count> each_column_sql;
      std::array<string, column_count> each_column_modifier;

      for (int i = 0; i < column_count; i++) {
        auto& modifier_str = get<0>(modifiers_list[modifier_indices[i]]);
        each_column_sql[i] = column_name_prefix + std::to_string(i) + " " +
                             to_string(types[i]) + " " + modifier_str;
      }

      auto column_sql = join(each_column_sql, string(", "));

      auto sql =
        string() + "CREATE TABLE " + table_name + " (" + column_sql + ");";

      INFO("Parsing SQL '" << sql << "'");
      REQUIRE_NOTHROW(parsed_cmd = parser.parse(sql));

      REQUIRE(holds_alternative<CreateTableCommand>(parsed_cmd.command));
      REQUIRE_NOTHROW(actual_cmd = get<CreateTableCommand>(parsed_cmd.command));

      CHECK(actual_cmd.table_name == table_name);
      REQUIRE(actual_cmd.columns.size() == column_count);

      for (int i = 0; i < column_count; i++) {
        auto& column = actual_cmd.columns[i];
        auto& column_modifier = modifiers_list[modifier_indices[i]];

        CHECK(column.name == column_name_prefix + std::to_string(i));
        CHECK(column.type == types[i]);

        for (auto& modifier : modifiers_list) {
          INFO("Inspecting modifier " << get<0>(modifier));
          auto modifier_check = get<1>(modifier);
          if (&modifier == &column_modifier)
            CHECK(modifier_check(column));
          else
            CHECK_FALSE(modifier_check(column));
        }
      }
    }

    SECTION("Check some combination of modifiers")
    {
      auto sql =
        string() + "CREATE TABLE " + table_name +
        " (id int primary key autoincrement, name text default 'omer' unique);";

      INFO("Parsing SQL '" << sql << "'");
      REQUIRE_NOTHROW(parsed_cmd = parser.parse(sql));

      REQUIRE(holds_alternative<CreateTableCommand>(parsed_cmd.command));
      REQUIRE_NOTHROW(actual_cmd = get<CreateTableCommand>(parsed_cmd.command));

      CHECK(actual_cmd.table_name == table_name);
      REQUIRE(actual_cmd.columns.size() == 2);

      {
        auto& column = actual_cmd.columns[0];

        CHECK(column.name == "id");
        CHECK(column.type == ColumnType::INT);

        CHECK(column.modifiers.primary_key.has_value());
        CHECK(column.modifiers.auto_increment.has_value());

        CHECK_FALSE(column.modifiers.unique.has_value());
        CHECK_FALSE(column.modifiers.is_null.has_value());
        CHECK_FALSE(column.modifiers.not_null.has_value());
        CHECK_FALSE(column.modifiers.default_value.has_value());
      }
      {
        auto& column = actual_cmd.columns[1];

        CHECK(column.name == "name");
        CHECK(column.type == ColumnType::TEXT);

        CHECK(column.modifiers.default_value.has_value());
        CHECK(column.modifiers.unique.has_value());
        CHECK_FALSE(column.modifiers.primary_key.has_value());
        CHECK_FALSE(column.modifiers.is_null.has_value());
        CHECK_FALSE(column.modifiers.not_null.has_value());
        CHECK_FALSE(column.modifiers.auto_increment.has_value());

        using white::davisbase::ast::LiteralValue;

        LiteralValue literal;
        REQUIRE_NOTHROW(literal =
                          column.modifiers.default_value.value().literal);
        REQUIRE_NOTHROW(get<string>(literal.value) == "omer");
      }
    }
  }
}

TEST_CASE("Parse inserting into table", "[parser][insert_into]")
{
  using white::davisbase::ast::ColumnType;
  using white::davisbase::ast::InsertIntoCommand;

  Parser parser;
  Command parsed_cmd;

  SECTION("Simple cases")
  {
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test VALUES(1);"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test VALUES('omer');"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test VALUES(3.14);"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test VALUES(3);"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test VALUES(\"omer\");"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test VALUES('omer');"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test VALUES('\\'quoted\\'');"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test VALUES('\\\"quoted\\\"');"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test VALUES(1, 2);"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test VALUES(1,2);"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test VALUES(1, 2, 3,4);"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test(col1) VALUES(1);"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test (col1) VALUES(1);"));
    REQUIRE_NOTHROW(
      parser.parse("INSERT INTO test (col1, col2) VALUES(1, 2);"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test (col1,col2) VALUES(1, 2);"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test (col1,col2) VALUES(1,2);"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test (col1) VALUES(1, 2, 3);"));
    REQUIRE_NOTHROW(parser.parse("INSERT INTO test (col1) VALUES(1, 2,3);"));

    REQUIRE_THROWS(parser.parse("INSERT INTO test VALUES();"));
    REQUIRE_THROWS(parser.parse("INSERT INTO test() VALUES;"));
    REQUIRE_THROWS(parser.parse("INSERT INTO test() VALUES();"));
    REQUIRE_THROWS(parser.parse("INSERT INTO test () VALUES();"));
    REQUIRE_THROWS(parser.parse("INSERT INTO test (col1) VALUES();"));
    REQUIRE_THROWS(parser.parse("INSERT INTO test () VALUES(1);"));
  }
}

TEST_CASE("Parse select from table", "[parser][select]")
{
  using white::davisbase::ast::OperatorType;
  using white::davisbase::ast::SelectCommand;
  using white::davisbase::ast::WhereClause;

  Parser parser;
  Command parsed_cmd;

  SECTION("Simple cases")
  {
    REQUIRE_NOTHROW(parser.parse("SELECT * FROM test;"));
    REQUIRE_NOTHROW(parser.parse("SELECT tcol FROM test;"));
    REQUIRE_NOTHROW(parser.parse("SELECT tcol1, tcol2 FROM test;"));
    REQUIRE_NOTHROW(parser.parse("SELECT * FROM test WHERE tcol > 3.14;"));
    REQUIRE_NOTHROW(parser.parse("SELECT tcol FROM test WHERE tcol > 3.14;"));
    REQUIRE_NOTHROW(parser.parse("SELECT tcol1 FROM test WHERE tcol2 > 3.14;"));
    REQUIRE_NOTHROW(
      parser.parse("SELECT tcol1, tcol2 FROM test WHERE tcol3 > 3.14;"));
    REQUIRE_NOTHROW(parser.parse("SELECT * FROM test WHERE tcol = \"viraj\";"));

    REQUIRE_THROWS(parser.parse("SELECT (tcol) FROM test;"));
    REQUIRE_THROWS(parser.parse("SELECT tcol() FROM test;"));
    REQUIRE_THROWS(parser.parse("SELECT tcol() FROM test();"));
    REQUIRE_THROWS(parser.parse("SELECT () FROM test;"));
    REQUIRE_THROWS(parser.parse("SELECT () FROM test WHERE 20 < tcol;"));
    REQUIRE_THROWS(parser.parse("SELECT () FROM test WHERE tcol < (20);"));
    REQUIRE_THROWS(parser.parse("SELECT () FROM test WHERE tcol < ()20;"));
    REQUIRE_THROWS(parser.parse("SELECT (tcol) FROM test WHERE tcol < (20);"));
    REQUIRE_THROWS(parser.parse("SELECT tcol() FROM test WHERE tcol < (20);"));
    REQUIRE_THROWS(
      parser.parse("SELECT tcol() FROM test() WHERE tcol < (20);"));
    REQUIRE_THROWS(parser.parse("SELECT tcol() FROM test WHERE tcol < ()20;"));
  }

  SECTION("Parsed content")
  {
    SelectCommand actual_cmd;

    SECTION("Select all columns")
    {
      REQUIRE_NOTHROW(parsed_cmd = parser.parse("SELECT * FROM test;"));
      REQUIRE(holds_alternative<SelectCommand>(parsed_cmd.command));
      REQUIRE_NOTHROW(actual_cmd = get<SelectCommand>(parsed_cmd.command));

      CHECK(actual_cmd.table_name == "test");
      CHECK(actual_cmd.column_names.size() == 0);
      CHECK_FALSE(actual_cmd.condition.has_value());
    }

    SECTION("Select all columns with where")
    {
      WhereClause where;

      REQUIRE_NOTHROW(parsed_cmd =
                        parser.parse("SELECT * FROM test WHERE tcol > 3.14;"));
      REQUIRE(holds_alternative<SelectCommand>(parsed_cmd.command));
      REQUIRE_NOTHROW(actual_cmd = get<SelectCommand>(parsed_cmd.command));

      CHECK(actual_cmd.table_name == "test");
      CHECK(actual_cmd.column_names.size() == 0);

      REQUIRE(actual_cmd.condition.has_value());
      where = actual_cmd.condition.value();

      CHECK(where.column_name == "tcol");
      CHECK(where.op == OperatorType::GREATER);
      REQUIRE(holds_alternative<long double>(where.literal.value));
      REQUIRE_NOTHROW(get<long double>(where.literal.value) == 3.14L);
    }

    SECTION("Select one column")
    {
      REQUIRE_NOTHROW(parsed_cmd = parser.parse("SELECT tcol1 FROM test;"));
      REQUIRE(holds_alternative<SelectCommand>(parsed_cmd.command));
      REQUIRE_NOTHROW(actual_cmd = get<SelectCommand>(parsed_cmd.command));

      CHECK(actual_cmd.table_name == "test");
      CHECK_FALSE(actual_cmd.condition.has_value());

      REQUIRE(actual_cmd.column_names.size() == 1);
      CHECK(actual_cmd.column_names[0] == "tcol1");
    }

    SECTION("Select some columns")
    {
      REQUIRE_NOTHROW(parsed_cmd =
                        parser.parse("SELECT tcol1, tcol2 FROM test;"));
      REQUIRE(holds_alternative<SelectCommand>(parsed_cmd.command));
      REQUIRE_NOTHROW(actual_cmd = get<SelectCommand>(parsed_cmd.command));

      CHECK(actual_cmd.table_name == "test");
      CHECK_FALSE(actual_cmd.condition.has_value());

      REQUIRE(actual_cmd.column_names.size() == 2);
      CHECK(actual_cmd.column_names[0] == "tcol1");
      CHECK(actual_cmd.column_names[1] == "tcol2");
    }

    SECTION("Select some columns with where")
    {
      WhereClause where;

      REQUIRE_NOTHROW(parsed_cmd = parser.parse(
                        "SELECT tcol1, tcol2 FROM test WHERE tcol > 3.14;"));
      REQUIRE(holds_alternative<SelectCommand>(parsed_cmd.command));
      REQUIRE_NOTHROW(actual_cmd = get<SelectCommand>(parsed_cmd.command));

      CHECK(actual_cmd.table_name == "test");

      REQUIRE(actual_cmd.column_names.size() == 2);
      CHECK(actual_cmd.column_names[0] == "tcol1");
      CHECK(actual_cmd.column_names[1] == "tcol2");

      REQUIRE(actual_cmd.condition.has_value());
      where = actual_cmd.condition.value();

      CHECK(where.column_name == "tcol");
      CHECK(where.op == OperatorType::GREATER);
      REQUIRE(holds_alternative<long double>(where.literal.value));
      REQUIRE_NOTHROW(get<long double>(where.literal.value) == 3.14L);
    }
  }
}

TEST_CASE("Parse WHERE Clause", "[parser][where_clause]")
{
  using white::davisbase::ast::OperatorType;
  using white::davisbase::ast::WhereClause;
  using white::davisbase::parser::StringQueryGrammar;
  using white::util::test_phrase_parser_attr;

  StringQueryGrammar grammar;
  WhereClause where;

  REQUIRE(test_phrase_parser_attr("WHERE col1 = 5", grammar.where, where));
  CHECK(where.column_name == "col1");
  CHECK(where.op == OperatorType::EQUAL);
  CHECK(get<long long>(where.literal.value) == 5);

  where = WhereClause();
  REQUIRE(test_phrase_parser_attr("WHERE col2 < \"5\"", grammar.where, where));
  CHECK(where.column_name == "col2");
  CHECK(where.op == OperatorType::LESS);
  CHECK(get<std::string>(where.literal.value) == "5");

  where = WhereClause();
  REQUIRE(test_phrase_parser_attr("WHERE col3 <= 4.5", grammar.where, where));
  CHECK(where.column_name == "col3");
  CHECK(where.op == OperatorType::LESS_EQUAL);
  CHECK(get<long double>(where.literal.value) == 4.5L);

  where = WhereClause();
  REQUIRE(
    test_phrase_parser_attr("WHERE col4 > 'harambe'", grammar.where, where));
  CHECK(where.column_name == "col4");
  CHECK(where.op == OperatorType::GREATER);
  CHECK(get<std::string>(where.literal.value) == "harambe");

  where = WhereClause();
  long long big_number = 0x1FFFFFFFFFFFFFFF;
  REQUIRE(test_phrase_parser_attr("WHERE col5 >= " + std::to_string(big_number),
                                  grammar.where, where));
  CHECK(where.column_name == "col5");
  CHECK(where.op == OperatorType::GREATER_EQUAL);
  CHECK(get<long long>(where.literal.value) == big_number);

  where = WhereClause();
  REQUIRE(test_phrase_parser_attr(
    "WHERE col6 >= " + std::to_string(-big_number), grammar.where, where));
  CHECK(where.column_name == "col6");
  CHECK(where.op == OperatorType::GREATER_EQUAL);
  CHECK(get<long long>(where.literal.value) == -big_number);

  REQUIRE_FALSE(
    test_phrase_parser_attr("WHERE col > harambe", grammar.where, where));
  REQUIRE_FALSE(test_phrase_parser_attr(
    "WHERE col > 999999999999999999999999999", grammar.where, where));
  REQUIRE_FALSE(test_phrase_parser_attr("WHERE col", grammar.where, where));
  REQUIRE_FALSE(test_phrase_parser_attr("WHERE true", grammar.where, where));
}
