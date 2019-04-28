#include "ast.hpp"

#include <iostream>

#include "sdl/database.hpp"

namespace white::davisbase::ast {

using sdl::Database;

void Command::execute(Database& database)
{
  std::visit([&](auto&& arg) { arg.execute(database); }, command);
}

void ShowTablesCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

void DropTableCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

void CreateTableCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

void InsertIntoCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

void SelectCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

void DeleteFromCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

void UpdateCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

void CreateIndexCommand::execute(Database& database)
{
  std::cout << *this << std::endl;
}

} // namespace white::davisbase::ast
