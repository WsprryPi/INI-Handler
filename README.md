# INI File Parser

## Overview

This project is a simple and efficient **INI file parser** written in C++. It provides functionality to load, modify, and save INI-style configuration files while preserving comments and formatting.

## Features

- Meyers' singleton - access the class with the alias `iniFile`
- Load and save INI files while maintaining formatting and comments.
- Retrieve values as **string, boolean, integer, and double**.
- Supports **default values** when retrieving data.
- Provides **error handling** for missing keys, invalid formats, and out-of-range conversions.
- Includes a test target for verifying functionality.
- Static analysis with `cppcheck` is supported.

## Dependencies

- **C++20 or later**
- `g++` (or another C++ compiler with C++20 support)
- `cppcheck` (for linting, optional)

## Installation

1. Clone the repository:

   ```sh
   git clone https://github.com/your-repo/ini-parser.git
   cd ini-parser
   ```

2. Compile the project:

   ```sh
   make
   ```

3. Run tests:

   ```sh
   make test
   ```

## Usage

### Loading an INI File

```cpp
#include "ini_file.hpp"

iniFile.set_filename("config.ini");
std::string value = config.get_string_value("Settings", "Theme");
int number = config.get_int_value("Settings", "MaxUsers", 100);
```

### Setting and Saving Values

```cpp
config.set_string_value("Settings", "Theme", "Dark");
config.set_int_value("Settings", "MaxUsers", 200);
config.commit_changes();
```

## Makefile Targets

| Target      | Description |
|------------|-------------|
| `make`     | Builds the library (excludes test program) |
| `make test`| Compiles and runs the test program |
| `make clean` | Removes compiled files |
| `make lint` | Runs static analysis using `cppcheck` |
| `make help` | Displays available targets |

## License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE.md) for details.

## Contributing

Contributions are welcome! Please submit a pull request with your improvements or bug fixes.

## Contact

For questions or issues, please create an issue in the repository.
