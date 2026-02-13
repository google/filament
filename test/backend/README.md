# Backend Tests

This directory (`/test/backend`) contains scripts to run Filament's backend tests using a software
rasterizer (Mesa). This is useful for running tests in a continuous integration environment where a
GPU might not be available.

The `test.sh` script will:

1.  Build Mesa if needed.
2.  Build the backend tests.
3.  Run the tests using the OpenGL and Vulkan backends with Mesa.

## Flags

The `test.sh` script accepts the following flags:

-   `--gtest_filter`: Filters the tests that are run. This is passed directly to the underlying
    gtest executable.
-   `--backend`: Specifies which backend to test. Can be `opengl`, `vulkan`, or a comma-separated
    list (e.g., `opengl,vulkan`). Defaults to both.
