#include "parser.hpp"
#include "common.hpp"

#include <boost/spirit/include/qi.hpp>

namespace qi = boost::spirit::qi;

namespace white::davisbase::parser {

using std::string;

using ast::Command;

template<typename TStr>
constexpr auto Q(TStr&& str)
{
  return qi::copy(qi::no_case[std::forward<TStr>(str)] >> qi::eps);
}

template<typename TStr, typename... TArgs>
constexpr auto Q(TStr&& str, TArgs&&... args)
{
  return qi::copy(qi::no_case[std::forward<TStr>(str)] >>
                  Q(std::forward<TArgs>(args)...));
}

template<typename Iterator, typename Skipper = qi::ascii::space_type>
struct QueryGrammar : qi::grammar<Iterator, Command(), Skipper>
{
  QueryGrammar()
    : QueryGrammar::base_type(command)
  {
    using qi::char_;
    using qi::eps;
    using qi::lit;
    using qi::long_double;
    using qi::long_long;
    using qi::uint_;
    using qi::ascii::alnum;
    using qi::ascii::alpha;
    using qi::ascii::digit;

    using qi::no_case;

    using common::ColumnType;
    using common::OperatorType;

    /* Identifier Naming Rules */

    identifier = alpha >> *(alnum | char_('_'));
    column_name = identifier;
    table_name = identifier;

    /* Type Rules */

    // clang-format off
    field_type.add
      ("tinyint", ColumnType::TINYINT)
      ("smallint", ColumnType::SMALLINT)
      ("int", ColumnType::INT)
      ("bigint", ColumnType::BIGINT)
      ("long", ColumnType::BIGINT)
      ("float", ColumnType::FLOAT)
      ("real", ColumnType::FLOAT)
      ("year", ColumnType::YEAR)
      ("time", ColumnType::TIME)
      ("datetime", ColumnType::DATETIME)
      ("date", ColumnType::DATE)
      ("text", ColumnType::TEXT);
    // clang-format on

    /* Literal Value Rules */

    qi::real_parser<long double, qi::strict_real_policies<long double>>
      floating_point_literal;

    // clang-format off
    string_literal = (
      '"' >> +(('\\' >> char_("\\\"")) | ~char_('"')) >> '"' |
      '\'' >> +(('\\' >> char_("\\'")) | ~char_('\'')) >> '\''
    );
    // clang-format on

    null_value = Q("NULL");
    literal = null_value | string_literal | floating_point_literal | long_long;

    /* Operator Rules */

    // clang-format off
    op.add
      ("<", OperatorType::LESS)
      ("<=", OperatorType::LESS_EQUAL)
      ("=", OperatorType::EQUAL)
      (">", OperatorType::GREATER)
      (">=", OperatorType::GREATER_EQUAL);
    // clang-format on

    /* Other Primitive Rules */

    is_null = Q("NULL") >> qi::attr(true);
    not_null = Q("NOT", "NULL") >> qi::attr(true);
    primary_key = Q("PRIMARY", "KEY") >> qi::attr(true);
    autoincrement = Q("AUTOINCREMENT") >> qi::attr(true);
    unique = Q("UNIQUE") >> qi::attr(true);
    default_value = Q("DEFAULT") >> literal;

    column_modifiers =
      is_null ^ not_null ^ primary_key ^ autoincrement ^ unique ^ default_value;

    column = column_name >> no_case[field_type] >> -column_modifiers;

    where = Q("WHERE") >> column_name >> op >> literal;

    /* DDL */

    show_tables = Q("SHOW", "TABLES");
    create_table =
      Q("CREATE", "TABLE") >> table_name >> '(' >> column % ',' >> ')';
    drop_table = Q("DROP", "TABLE") >> table_name;

    create_index = Q("CREATE") >>
                   (Q("UNIQUE") >> qi::attr(true) | qi::attr(false)) >>
                   Q("INDEX", "ON") >> table_name >> '(' >> column_name >> ')';

    exit = Q("EXIT");

    /* DML */

    insert_into = Q("INSERT", "INTO") >> table_name >>
                  -('(' >> column_name % ',' >> ')') >> Q("VALUES") >> '(' >>
                  literal % ',' >> ')';
    delete_from = Q("DELETE", "FROM") >> table_name >> -where;
    update = Q("UPDATE") >> table_name >> Q("SET") >> column_name >> '=' >>
             literal >> -where;

    /* VDL */

    select = no_case["SELECT"] >> (lit('*') | (column_name % ',')) >>
             no_case["FROM"] >> table_name >> -where;

    /* Main command */

    command = (show_tables | drop_table | create_table | insert_into | select |
               delete_from | update | create_index | exit) >>
              ';';
  }

  qi::rule<Iterator, string()> identifier;
  qi::rule<Iterator, string()> column_name;
  qi::rule<Iterator, string()> table_name;

  qi::symbols<char, common::ColumnType> field_type;

  qi::rule<Iterator, common::NullValue(), Skipper> null_value;
  qi::rule<Iterator, common::LiteralValue(), Skipper> literal;
  qi::rule<Iterator, string(), Skipper> string_literal;

  qi::symbols<char, common::OperatorType> op;

  qi::rule<Iterator, bool, Skipper> is_null;
  qi::rule<Iterator, bool, Skipper> not_null;
  qi::rule<Iterator, bool, Skipper> primary_key;
  qi::rule<Iterator, bool, Skipper> unique;
  qi::rule<Iterator, bool, Skipper> autoincrement;
  qi::rule<Iterator, common::ColumnModifiers::DefaultValue(), Skipper>
    default_value;
  qi::rule<Iterator, common::ColumnModifiers(), Skipper> column_modifiers;

  qi::rule<Iterator, common::ColumnDefinition(), Skipper> column;
  qi::rule<Iterator, ast::WhereClause(), Skipper> where;

  qi::rule<Iterator, ast::ShowTablesCommand(), Skipper> show_tables;
  qi::rule<Iterator, ast::CreateTableCommand(), Skipper> create_table;
  qi::rule<Iterator, ast::DropTableCommand(), Skipper> drop_table;
  qi::rule<Iterator, ast::CreateIndexCommand(), Skipper> create_index;

  qi::rule<Iterator, ast::InsertIntoCommand(), Skipper> insert_into;
  qi::rule<Iterator, ast::DeleteFromCommand(), Skipper> delete_from;
  qi::rule<Iterator, ast::SelectCommand(), Skipper> select;
  qi::rule<Iterator, ast::UpdateCommand(), Skipper> update;
  qi::rule<Iterator, ast::ExitCommand(), Skipper> exit;

  qi::rule<Iterator, Command(), Skipper> command;
};

using StringQueryGrammar = QueryGrammar<std::string::const_iterator>;

struct Parser::impl
{
  Command parse(const string& cmd);

  StringQueryGrammar grammar;
};

Command Parser::impl::parse(const string& cmd)
{
  using qi::ascii::space;

  Command parsed_cmd;
  auto iter = cmd.begin();
  auto end = cmd.end();
  bool r = phrase_parse(iter, end, grammar, space, parsed_cmd);
  if (r && iter == end)
    return parsed_cmd;
  throw std::runtime_error("Couldn't parse: " + string(iter, end));
}

Parser::Parser()
  : pimpl{std::make_unique<impl>()}
{}

Parser::~Parser() = default;

Command Parser::parse(const string& cmd)
{
  return pimpl->parse(cmd);
}

} // namespace white::davisbase::parser
