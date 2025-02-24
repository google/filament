
# Loader Tests

This directory contains a test suite for the Vulkan loader.
These tests are not exhaustive &mdash; they are expected to be supplemented with other tests, such as CTS.


## Test specific CMake Configuration

| Option                         | Platform | Default | Description                                              |
| ------------------------------ | -------- | ------- | -------------------------------------------------------- |
| BUILD_TESTS                    | All      | `OFF`   | Controls whether or not the loader tests are built.      |
| ENABLE_LIVE_VERIFICATION_TESTS | All      | `OFF`   | Enables building of tests meant to run with live drivers |

## Running Tests

For most purposes `ctest` is the desired method of running tests.
This is because when a test fails, `ctest` will automatically printout the failing test case.

Tests are organized into various executables:
 * `test_regression` - Contains most tests.
 * `test_threading` - Tests which need multiple threads to execute.
   * This allows targeted testing which uses tools like ThreadSanitizer

The loader test framework is designed to be easy to use, as simple as just running a single executable. To achieve that requires extensive build script
automation is required. More details are in the tests/framework/README.md.
The consequence of this automation: Do not relocate the build folder of the project without cleaning the CMakeCache. Most components are found by absolute
path and thus require the contents of the folder to not be relocated.

## Writing Tests

The `test_environment.h/cpp` are the primary tool used when creating new tests. Either use the existing child classes of FrameworkEnvironment or create a new one
to suite the tests need. Refer to the `tests/framework/README.md` for more details.

IMPORTANT NOTES:
 * NEVER FORGET TO DESTROY A VKINSTANCE THAT WAS SUCCESSFULLY CREATED.
   * The loader loads dynamic libraries in `vkCreateInstance` and unloads them in `vkDestroyInstance`. If these dynamic libraries aren't unloaded, they leak state
   into the next test that runs, causing spurious failures or even crashes.
   * Use InstWrapper helper as well as DeviceWrapper to automate this concern away.

## Using a specific loader with the tests

The environment variable `VK_LOADER_TEST_LOADER_PATH` can be used to specify which vulkan-loader binary should be used.
This is useful when writing tests to exercise a bug fix.
Simply build the loader without the fix, stash it in a known location.
Write the fix and the test that should exercise the bug and it passes.
Then run the test again but with this env-var set to the older loader without the fix and show that the test now fails.

Basic usage example:
```c
VK_LOADER_TEST_LOADER_PATH="/path/to/older/loader/build" ctest --output-on-failure
```
