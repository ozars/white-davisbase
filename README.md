# white-davisbase
[![Build Status](https://travis-ci.org/ozars/white-davisbase.svg?branch=master)](https://travis-ci.org/ozars/white-davisbase)
[![codecov](https://codecov.io/gh/ozars/white-davisbase/branch/master/graph/badge.svg)](https://codecov.io/gh/ozars/white-davisbase)

A simple SQL database implemented with C++17.

## Requirements

- CMake 3.5 or newer.
- A modern C++ compiler supporting C++17 features. g++ 8 or newer works. `<variant>` and `<optional>` headers should be supported without experimental prefix.
- Boost library. Tests are run with Boost 1.67.

## Compiling

```
$ git clone --recursive https://github.com/ozars/white-davisbase
$ cd white-database
$ mkdir build
$ cd build
$ cmake ../ && make && cd bin
$ ./davisbase
```

## Testing

```
$ make tests
$ ctest --output-on-failure
```
