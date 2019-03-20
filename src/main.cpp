#include <iostream>

#include "parser.hpp"

int main()
{
  std::string cmd;
  white::davisbase::parser::Parser parser;
  while (getline(std::cin, cmd)) {
    try {
      parser.parse(cmd).execute();
    } catch (const std::runtime_error& err) {
      std::cerr << "[ERROR] " << err.what() << std::endl;
    }
  }
  return 0;
}
