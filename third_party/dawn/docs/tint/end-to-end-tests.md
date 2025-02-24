# Tint end-to-end tests

This repo contains a large number of end-to-end tests at `<dawn>/test/tint`.

## Test files

Test input files have either the `.wgsl`, `.spv` or `.spvasm` file extension.

Each test input file is tested against each of the Tint backends. There are `<number-of-input-files>` &times; `<number-of-tint-backends>` tests that are performed on an unfiltered end-to-end test run.

Each backend test can have an **expectation file**. This expectation file sits next to the input file, with a `<input-file>.expected.<format>` extension. For example the test `test/foo.wgsl` would have the HLSL expectation file `test/foo.wgsl.expected.hlsl`.

An expectation file contains the expected output of Tint, when passed the input file for the given backend.

If the first line of the expectation file starts `SKIP`, then the test will be skipped instead of failing the end-to-end test run. It is good practice to include after the `SKIP` a reason for why the test is being skipped, along with any additional details, such as compiler error messages.

## Running

To run the end-to-end tests use the `./tools/run tests` script, passing the path to the tint executable with the `--tint` (defaulting to `./out/active/tint`):

    ./tools/run tests

You can pass a list of globs, directories or file paths as extra arguments to specify what tests you
want to run.

You can pass `--help` to see the full list of command line flags.\
The most commonly used flags are:

| flag                 | description |
|----------------------|-------------|
|`--format`            | Filters the tests to the particular backend.<br>Example: `--format hlsl` will just test the HLSL backend.
|`--generate-expected` | Generate expectation files for the tests that previously had no expectation file, or were marked as `SKIP` but now pass.
|`--generate-skip`     | Generate `SKIP` expectation files for tests that are not currently passing.

## Authoring guidelines

Each test should be as small as possible, and focused on the particular feature being tested.

Use sub-directories whenever possible to group similar tests, and try to keep the pattern of directories as consistent as possible between different tests. This helps filter tests with glob patterns.
