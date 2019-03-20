#pragma once

#include <iomanip>

namespace white::util {

class OutputManipulator
{
public:
  OutputManipulator(std::ostream& stream)
    : stream_(stream)
    , old_flags_(stream_.flags())
  {
    stream << std::boolalpha << std::showpoint;
  }

  ~OutputManipulator() { stream_.setf(old_flags_); }

private:
  std::ostream& stream_;
  std::ostream::fmtflags old_flags_;
};

} // namespace white::util
