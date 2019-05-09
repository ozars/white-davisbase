#include <iostream>

#include "parser.hpp"
#include "sdl/database.hpp"

int main()
{
  std::string cmd;
  white::davisbase::parser::Parser parser;
  white::davisbase::sdl::Database database;

  while ((std::cout << "davisbase> "), getline(std::cin, cmd)) {
    try {
      using white::davisbase::ast::ExitCommand;
      auto parsed_cmd = parser.parse(cmd);
      if (std::holds_alternative<ExitCommand>(parsed_cmd.command))
        break;
      parsed_cmd.execute(database);
    } catch (const std::exception& err) {
      std::cerr << "[ERROR] " << err.what() << std::endl;
    }
  }
  return 0;
}
