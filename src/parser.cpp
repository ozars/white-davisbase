#include "parser.hpp"

#include <boost/spirit/include/qi.hpp>

namespace qi = boost::spirit::qi;

namespace white::davisbase::parser {

using std::string;

using ast::Command;
using ast::CreateTableCommand;
using ast::DeleteFromCommand;
using ast::DropTableCommand;
using ast::InsertIntoCommand;
using ast::SelectCommand;
using ast::UpdateCommand;
using ast::ShowTablesCommand;

using ast::Column;
using ast::ColumnModifiers;
using ast::ColumnType;
using ast::LiteralValue;
using ast::OperatorType;
using ast::WhereClause;

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
      ("real", ColumnType::REAL)
      ("double", ColumnType::DOUBLE)
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

    literal = string_literal | floating_point_literal | long_long;

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

    is_null = Q("NULL");
    not_null = Q("NOT", "NULL");
    primary_key = Q("PRIMARY", "KEY");
    autoincrement = Q("AUTOINCREMENT");
    unique = Q("UNIQUE");
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

    /* TODO: Uniqueness. */
    create_index =
      no_case[lit("CREATE") >> -lit("UNIQUE") >> "INDEX" >> "ON"] >>
      table_name >> '(' >> column % ',' >> ')';

    /* DML */

    insert_into = Q("INSERT", "INTO") >> table_name >>
                  -('(' >> column_name % ',' >> ')') >> Q("VALUES") >> '(' >>
                  literal % ',' >> ')';
    delete_from = Q("DELETE", "FROM") >> table_name >> where;

    /* Parsing the update command */
    update = Q("UPDATE") >> table_name >> Q("SET") >>
             column_name >> '=' >> literal >> -where;

    /* VDL */

    select = no_case["SELECT"] >> (lit('*') | (column_name % ',')) >>
             no_case["FROM"] >> table_name >> -where;

    /* Main command */

    command = (show_tables | drop_table | create_table | insert_into | select |
               delete_from | update) >>
              ';';
  }

  qi::rule<Iterator, string()> identifier;
  qi::rule<Iterator, string()> column_name;
  qi::rule<Iterator, string()> table_name;

  qi::symbols<char, ColumnType> field_type;

  qi::rule<Iterator, LiteralValue(), Skipper> literal;
  qi::rule<Iterator, string(), Skipper> string_literal;

  qi::symbols<char, OperatorType> op;

  qi::rule<Iterator, ColumnModifiers::IsNull(), Skipper> is_null;
  qi::rule<Iterator, ColumnModifiers::NotNull(), Skipper> not_null;
  qi::rule<Iterator, ColumnModifiers::PrimaryKey(), Skipper> primary_key;
  qi::rule<Iterator, ColumnModifiers::Unique(), Skipper> unique;
  qi::rule<Iterator, ColumnModifiers::AutoIncrement(), Skipper> autoincrement;
  qi::rule<Iterator, ColumnModifiers::DefaultValue(), Skipper> default_value;
  qi::rule<Iterator, ColumnModifiers(), Skipper> column_modifiers;

  qi::rule<Iterator, Column(), Skipper> column;
  qi::rule<Iterator, WhereClause(), Skipper> where;

  qi::rule<Iterator, ShowTablesCommand(), Skipper> show_tables;
  qi::rule<Iterator, CreateTableCommand(), Skipper> create_table;
  qi::rule<Iterator, DropTableCommand(), Skipper> drop_table;
  qi::rule<Iterator, Skipper> create_index;

  qi::rule<Iterator, InsertIntoCommand(), Skipper> insert_into;
  qi::rule<Iterator, DeleteFromCommand(), Skipper> delete_from;

  qi::rule<Iterator, SelectCommand(), Skipper> select;
  qi::rule<Iterator, UpdateCommand(), Skipper> update;
  qi::rule<Iterator, Skipper> exit;

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
