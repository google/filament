# Generating Backend Code Coverage

Code coverage analysis helps visualize which parts of the backend are exercised by backend tests.
This guide outlines the process for generating an HTML coverage report for Filament's backend on
macOS.

## 1\. Prerequisites: Install Clang and LLVM tools

You'll need a recent version of **Clang** and its corresponding **LLVM tools** for code coverage.
You can install these using Homebrew or MacPorts.

### Using Homebrew

Install the `llvm` package:

```bash
brew install llvm
```

This typically installs the tools in a location like `/usr/local/opt/llvm/bin`. You may need to add
this to your `PATH` environment variable.

### Using MacPorts

Install a specific version of Clang (e.g., version 18):

```bash
sudo port install clang-18
```

MacPorts often adds version suffixes to the tool names (e.g., `llvm-cov-mp-18`).

### Required Tools

Ensure you can locate the following tools from your installation:

  * `clang` and `clang++` (The C/C++ compilers)
  * `llvm-profdata` (For merging coverage data)
  * `llvm-cov` (For generating reports)

The rest of this guide assumes your tools are in your `PATH`. If not, you'll need to use the full
path to each executable.

## 2\. Build Filament with Coverage Enabled

Compile the `backend_test_mac` target with coverage instrumentation. This is done by setting the
`CC` and `CXX` environment variables to point to your Clang compiler and using the `-V` flag in the
build script.

```bash
CC=clang CXX=clang++ ./build.sh -V -p desktop debug backend_test_mac
```

*If your Clang executables aren't in your `PATH` or have version suffixes, provide the full name or
path (e.g., `CC=/opt/local/bin/clang CXX=/opt/local/bin/clang++`).*

## 3\. Run the Backend Tests

Running the test suite will generate the raw coverage data needed for the report.

1.  Navigate to the build output directory:

    ```bash
    cd out/cmake-debug/filament/backend
    ```

2.  Run the tests for a specific backend (e.g., Metal):

    ```bash
    ./backend_test_mac --api metal
    ```

This command creates a `default.profraw` file in the current directory, which contains the raw
execution profile data.

## 4\. Generate the Coverage Report

Finally, process the raw data and generate an HTML report.

1.  **Merge the raw profile data** into a single file using `llvm-profdata`.

    ```bash
    llvm-profdata merge -sparse default.profraw -o filament.profdata
    ```

    *Remember to use the version-specific tool name if required (e.g., `llvm-profdata-mp-18`).*

2.  **Generate the HTML report** using `llvm-cov`. This command creates a report for the entire
    `backend_test_mac` executable.

    ```bash
    llvm-cov show ./backend_test_mac \
      -instr-profile=filament.profdata \
      -format=html \
      -show-line-counts-or-regions > coverage.html
    ```

    *To view coverage for a **specific source file**, add its path at the end of the command:*

    ```bash
    llvm-cov show ./backend_test_mac \
      -instr-profile=filament.profdata \
      -format=html \
      -show-line-counts-or-regions \
      -- ../../../../filament/backend/src/metal/MetalDriver.mm > coverage.html
    ```

3.  **Open the report** in your browser:

    ```bash
    open coverage.html
    ```

In the report, code paths that were not executed during the test run will be highlighted in red.
