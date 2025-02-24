# Generating and viewing Tint code-coverage

Requirements:

* Host running Linux or macOS
* Clang toolchain on the `PATH` environment variable

## Building Tint with coverage generation enabled

Follow the steps [to build Tint with CMake](../building.md), but include the additional `-DDAWN_EMIT_COVERAGE=1` CMake flag.

## Generate coverage information

Use the `<tint>/tools/tint-generate-coverage` script to run the tint executable or unit tests and generate the coverage information.

The script takes the executable to invoke as the first command line argument, followed by additional arguments to pass to the executable.

For example, to see the code coverage for all unit tests, run:
`<tint>/tools/tint-generate-coverage <build>/tint_unittests --gtest_brief`

The script will emit two files at the root of the tint directory:

* `coverage.summary` - A text file giving a coverage summary for all Tint source files.
* `lcov.info` - A binary coverage file that can be consumed with the [VSCode Coverage Gutters](https://marketplace.visualstudio.com/items?itemName=ryanluker.vscode-coverage-gutters) extension.
