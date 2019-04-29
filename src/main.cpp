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
      parser.parse(cmd).execute(database);
    } catch (const std::runtime_error& err) {
      std::cerr << "[ERROR] " << err.what() << std::endl;
    }
  }
  return 0;
}
