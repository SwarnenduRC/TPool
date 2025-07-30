# TPool (C++)

A lightweight, efficient, and easy-to-use thread pool utility library for C++ projects, developed on C++-20. This aims to provide essential thread pooling feature with minimal dependencies and overhead, suitable for both small and large scale applications.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usages)
- [Tests](#tests)
- [Documentation](#documentation)
- [Extension](#extension)
- [Bonus](#bonus)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

## Overview

It is designed to be lightweight, performant, and straightforward to integrate into any C++ project (min version C++-20) without complicated setup or heavy dependencies. Please note that this currently supports only MacOS and Linux platforms.

## Features

- TBD

## Installation

### Prerequisits

- C++ compiler supporting `C++-20` (e.g., GCC 10+, Clang 10+)
- make for building the library (currently build process is make based and uses `clang++` for compilation)
- Google Test framework for running tests (optional, but recommended)

Clone the repository:

`git clone https://github.com/SwarnenduRC/TPool.git TPool`

### Build the library and install (manual)

Run the following commands to build the library:

```bash
make clean
make
```

> **Note**: The library will be built as a static library by default. You can change the `LIB_TYPE` variable in the `makefile` to `shared` if you want to build it as a shared library.
> Release build will be created by default. You can change the `BUILD_TYPE` variable in the `makefile` to `debug` if you want to build it as a debug library. `all` target will build both release and debug libraries.
> By default Tests are not built. You can change the `BUILD_TESTS` variable in the `makefile` to `yes` to build the tests.
> If you want to do this all in one line rather than changing the `makefile`, you can run the following command:

```bash
make clean; make LIB_TYPE=shared BUILD_TYPE=debug BUILD_TESTS=yes
```

#### Install the library

To install the library, run the following commands if you are unsure for now where to install at present:

```bash
# Create target directories if not exist
sudo mkdir -p "$INSTALL_LIB_DIR"
sudo mkdir -p "$INSTALL_INCLUDE_DIR"

# Copy library file recursively
sudo cp -r "$LIB_DIR"/* "$INSTALL_LIB_DIR"

# Copy header files recursively
sudo cp -r "$HEADER_DIR"/* "$INSTALL_INCLUDE_DIR"
```

### Build the library and install (automatic)

You can also run the provided script to install the library automatically:

```bash
./buildNinstall.sh -BUILD_TYPE=release -LIB_TYPE=shared -FILE_LOGGING=yes -LOG_FILE_NAME=IntegrationTesting.log
```

Where `buildNinstall.sh` is a script that will build the library and install it to the default locations. You can also pass the following options to the script:

- `-BUILD_TYPE`: Set the build type (release or debug). Default is release.
- `-LIB_TYPE`: Set the library type (static or shared). Default is static.
- `-FILE_LOGGING`: Enable file logging. Default is no.
- `-LOG_FILE_NAME`: Set the log file name. Default is Logger.log.
- `-BUILD_TESTS`: Enable building tests. Default is no.

> **Note**: The script will automatically download and install the `fmt` library if it is not already installed.
> The default installation locations are:
>
> - Library files: `/usr/local/lib`
> - Header files: `/usr/local/include/logger`

For more details on the script, you can run

```bash
./buildNinstall.sh -h
```

## Configuration

You can configure the logger by modifying the `Logger.h` file or by passing configuration options during build and installation. The logger supports various configuration options such as cconsole logging/file logging, log file name (if file logging is enabled), max log file size, msg format, and more.

Please refer to buildNinstall.sh script for more details on the available configuration options or alternatively run

```bash
./buildNinstall.sh -h
```

## Usages

To use the TPool in your C++ project, follow these steps:

1. Include the header file in your source code:

## Tests

The library is having numerous unit test cases which uses `Google Unit test framework`. If you have built the test app too while building then you can run the test cases
by running following command:

```bash
./bin/TestThreadPool_d --gtest_shuffle --gtest_repeat=3
```

- `--gtest_shuffle` Shuffles the test cases in each iteration (optional)
- `--gtest_repeat=3` Repeats the test cases at least 3 times (optional)

![Test cases run](docs/TestCases.png)

## Documentation

For detailed documentation on the Logger library, including API references, configuration options, and examples, please generate the documentation using Doxygen. You can find the Doxygen configuration file in the root directory of the project.
To generate the documentation, run the following command:

```bash
doxygen Doxyfile
```

The generated `.html` files will be under `docs` folder which also contains a rudimentary class diagram of this library for a quick look.

## Contributing

Contributions, issues, and feature requests are welcome! Please follow these steps:

1. Fork this repository.
2. Create a feature branch (`git checkout -b feature-name`).
3. Commit your changes (`git commit -m "Add feature"`).
4. Push to the branch (`git push origin feature-name`).
5. Open a pull request.

Please ensure your code follows consistent style and includes tests where applicable.

> **NOTE:** If you are adding new code then back it by respective test case/s and make sure it doesn't break the existing ones.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact

Maintainer: Swarnendu Roy Chowdhury
GitHub: [SwarnenduRC](https://github.com/SwarnenduRC)

---

*Efficient C++ logging made simple. Feel free to open issues for bugs or feature requests.*
