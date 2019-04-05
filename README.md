# white-davisbase
[![Build Status](https://travis-ci.org/ozars/white-davisbase.svg?branch=master)](https://travis-ci.org/ozars/white-davisbase)
[![codecov](https://codecov.io/gh/ozars/white-davisbase/branch/master/graph/badge.svg)](https://codecov.io/gh/ozars/white-davisbase)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/a6e2be8a68384dd6ac23734ad6b3dc0f)](https://www.codacy.com/app/ozars/white-davisbase) [![Join the chat at https://gitter.im/white-davisbase/community](https://badges.gitter.im/white-davisbase/community.svg)](https://gitter.im/white-davisbase/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

A simple SQL database implemented with C++17.

## Requirements

- CMake 3.8 or newer.
- A modern C++ compiler supporting C++17 features. g++ 8 or newer works. `<variant>` and `<optional>` headers should be supported without experimental prefix.
- Boost library. Tests are run with Boost 1.67.

## Compiling

```console
$ git clone --recursive https://github.com/ozars/white-davisbase
$ cd white-database
$ mkdir build
$ cd build
$ cmake ../ && make && cd bin
$ ./davisbase
```

or using Docker:

```console
$ git clone --recursive https://github.com/ozars/white-davisbase
$ cd white-database
$ sudo docker build -t davisbase .
$ sudo docker run -it davisbase bin/davisbase
```

## Testing

After compiling:

```console
$ ctest --output-on-failure
```

or using Docker:

```console
$ sudo docker run -it davisbase ctest --output-on-failure
```

## Contribution

See [contribution document](./docs/CONTRIBUTING.md) for details.
