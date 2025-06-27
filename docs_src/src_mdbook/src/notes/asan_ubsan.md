# Running with ASAN/UBSAN

## Enabling

When building though build.sh, pass the `-b` flag. This sets the cmake variable
`FILAMENT_ENABLE_ASAN_UBSAN=ON` which eventually passes `"-fsanitize=address -fsanitize=undefined"`
to all compile and link operations.

If building through CMake directly, or an IDE like CLion that doesn't use build.sh, instead pass
`-DFILAMENT_ENABLE_ASAN_UBSAN=ON` to cmake in order to get the same result.

## Getting memory leak detection on Mac

Memory leak detection isn't enabled by default on MacOS. There are two issues to address, first is
using a version of clang that supports memory leak detection and second is enabling it at runtime.

The version of clang distributed by Apple (with a version like "Apple clang version 16.0.0") doesn't
currently support leak detection at all. Instead you will need to get or build a different LLVM,
such as the one distributed through homebrew and get CMake to use that instead.

Then during runtime you'll need to have the environment variable `ASAN_OPTIONS` include the option
`detect_leaks=1`. Multiple `ASAN_OPTIONS` values are concatenated with `:`.

## Getting memory leak output in CLion

### Setting variables

Under `Settings | Build, Execution, Deployment | Dynamic Analysis Tools | Sanitizers` there is an
ASAN Settings field that overrides whatever other `ASAN_OPTIONS` you might set elsewhere, so you
must use that instead of setting it through your Run/Debug Configuration.

To pass `-DFILAMENT_ENABLE_ASAN_UBSAN=ON` to CMake you'll want to create a new CMake Profile and
pass it as a CMake argument.

### Avoiding losing output

CMake will consume ASAN output and display it through a separate "Sanitizers" tab. Unfortunately
certain leak detection errors that interrupt the executable seem to not show up in this tab, but are
still removed from the user-visible console output. If this is happening and you need to see the
unfiltered console output you'll need to go to `Settings | Build, Execution, Deployment | Dynamic
Analysis Tools | Sanitizers` and uncheck "Use visual representation for Sanitizer's output".
