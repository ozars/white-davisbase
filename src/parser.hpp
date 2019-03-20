#pragma once

#include <memory>
#include <string>

#include "ast.hpp"

namespace white::davisbase::parser {

class Parser
{
private:
  struct impl;
  std::unique_ptr<impl> pimpl;

public:
  Parser();
  ~Parser();
  ast::Command parse(const std::string& cmd);
};

} // namespace white::davisbase::parser
