# cpp_json

## Introduction

This repository is used to learn C++ by implementing a JSON parser. The project is based on the tutorial from [miloyip/json-tutorial](https://github.com/miloyip/json-tutorial).

## Learning Purpose

The primary purpose of this project is to provide a hands-on learning experience for understanding the implementation of a JSON parser in C++. By working through this project, you will gain insights into various aspects of C++ programming, including memory management, data structures, and algorithm design.

## Project Source

This project is derived from the JSON tutorial by Milo Yip, which can be found at [miloyip/json-tutorial](https://github.com/miloyip/json-tutorial). The tutorial provides a step-by-step guide to implementing a JSON parser, and this project follows the same approach.

## Features

- JSON parsing
- JSON stringification
- Handling of various JSON data types (null, boolean, number, string, array, object)
- Memory management for JSON values
- Equality comparison of JSON values

## Usage

To use this project, you can clone the repository and build it using CMake. The project includes a test suite to verify the correctness of the JSON parser implementation.

```sh
git clone https://github.com/Foolish-Han/cpp_json.git
cd cpp_json
mkdir build
cd build
cmake ..
make
./leptjson_test
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
