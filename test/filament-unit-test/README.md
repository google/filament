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

The binaries run concurrently, up to one process per CPU by default, and the slowest ones are split into several processes with Google Test's built-in sharding (the `shards=N` prefix in `test_list.txt`). Each process's output is captured to a `test.log` file next to its XML output and printed once every process has finished. Pass `-j` to change how many processes run at once, for example `-j 1` to run them one at a time:

```bash
./test/filament-unit-test/test.sh -j 1
```

Each process is killed if it runs for more than 600 seconds, so that a hung test is reported as a failure with its log rather than stalling the run. Pass `-t` to change the limit. The limit is only enforced where the GNU `timeout` command is installed, which excludes stock macOS:

```bash
./test/filament-unit-test/test.sh -t 1200
```

By default, the script respects the `--gtest_filter` arguments defined in `test_list.txt` (which disable some known-failing or slow tests). If you wish to run the full, unfiltered suite of tests for those binaries, pass the `-f` flag:

```bash
./test/filament-unit-test/test.sh -f
```

## Tests that render

Some of the tests in `test_filament`, such as `RenderingTest`, render with the platform's default backend: Vulkan on Linux and Metal on macOS. On a Linux machine without a GPU, Vulkan needs Mesa's software driver, lavapipe, which is what the CI uses. On Debian and Ubuntu, install it with:

```bash
sudo apt-get install mesa-vulkan-drivers
```

In a build with sanitizers (`./build.sh -b`), LeakSanitizer reports what some of Mesa's Vulkan drivers still reference from their globals as leaked, once the Vulkan loader unloads them. `test.sh` sets `VK_LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING=1` to keep them loaded; set it too when you run such a build of `test_filament` directly.
