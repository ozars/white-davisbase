#include "ast.hpp"

#include <iostream>

namespace white::davisbase::ast {

void Command::execute()
{
  std::visit([](auto&& arg) { arg.execute(); }, command);
}

void ShowTablesCommand::execute()
{
  std::cout << *this << std::endl;
}

void DropTableCommand::execute()
{
  std::cout << *this << std::endl;
}

void CreateTableCommand::execute()
{
  std::cout << *this << std::endl;
}

void InsertIntoCommand::execute()
{
  std::cout << *this << std::endl;
}

void UpdateCommand::execute()
{
  std::cout << *this << std::endl;
}

void SelectCommand::execute()
{
  std::cout << *this << std::endl;
}

} // namespace white::davisbase::ast
