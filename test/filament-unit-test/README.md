# Filament Unit Tests

This folder contains the core C++ unit tests for Filament.

## Building the tests

Before running the unit tests, you must compile the test binaries. The easiest way to do this is to build the desktop debug target:

```bash
./build.sh debug
```

Alternatively, if you want to build them using release:
```bash
./build.sh release
```

## Running the tests

Once the project is compiled, you can execute the unit tests by running:

```bash
./test/filament-unit-test/test.sh
```

This script will verify that the test binaries exist, execute each one listed in `test_list.txt`, and generate Google Test XML output in `out/test-results/`.

By default, the script respects the `--gtest_filter` arguments defined in `test_list.txt` (which disable some known-failing or slow tests). If you wish to run the full, unfiltered suite of tests for those binaries, pass the `-f` flag:

```bash
./test/filament-unit-test/test.sh -f
```
