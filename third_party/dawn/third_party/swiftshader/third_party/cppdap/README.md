# cppdap

## About

`cppdap` is a C++11 library (["SDK"](https://microsoft.github.io/debug-adapter-protocol/implementors/sdks/)) implementation of the [Debug Adapter Protocol](https://microsoft.github.io/debug-adapter-protocol/), providing an API for implementing a DAP client or server.

`cppdap` provides C++ type-safe structures for the full [DAP specification](https://microsoft.github.io/debug-adapter-protocol/specification), and provides a simple way to add custom protocol messages.

## Fetching dependencies

`cppdap` provides CMake build files to build the library, unit tests and examples.

`cppdap` depends on the [`nlohmann/json` library](https://github.com/nlohmann/json), and the unit tests depend on the [`googletest` library](https://github.com/google/googletest). Both are referenced as a git submodules.

Before building, fetch the git submodules with:

```bash
cd <path-to-cppdap>
git submodule update --init
```

## Building

### Linux and macOS

Next, generate the build files:

```bash
cd <path-to-cppdap>
mkdir build
cd build
cmake ..
```

You may wish to suffix the `cmake ..` line with any of the following flags:

* `-DCPPDAP_BUILD_TESTS=1` - Builds the `cppdap` unit tests
* `-DCPPDAP_BUILD_EXAMPLES=1` - Builds the `cppdap` examples
* `-DCPPDAP_INSTALL_VSCODE_EXAMPLES=1` - Installs the  `cppdap` examples as Visual Studio Code extensions
* `-DCPPDAP_WARNINGS_AS_ERRORS=1` - Treats all compiler warnings as errors.

Finally, build the project:

`make`

### Windows

`cppdap` can be built using [Visual Studio 2019's CMake integration](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019).


### Using `cppdap` in your CMake project

You can build and link `cppdap` using `add_subdirectory()` in your project's `CMakeLists.txt` file:
```cmake
set(CPPDAP_DIR <path-to-cppdap>) # example <path-to-cppdap>: "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cppdap"
add_subdirectory(${CPPDAP_DIR})
```

This will define the `cppdap` library target, which you can pass to `target_link_libraries()`:

```cmake
target_link_libraries(<target> cppdap) # replace <target> with the name of your project's target
```

You may also wish to specify your own paths to the third party libraries used by `cppdap`.
You can do this by setting any of the following variables before the call to `add_subdirectory()`:

```cmake
set(CPPDAP_THIRD_PARTY_DIR <third-party-root-directory>) # defaults to ${CPPDAP_DIR}/third_party
set(CPPDAP_JSON_DIR        <path-to-nlohmann-json>)      # defaults to ${CPPDAP_THIRD_PARTY_DIR}/json
set(CPPDAP_GOOGLETEST_DIR  <path-to-googletest>)         # defaults to ${CPPDAP_THIRD_PARTY_DIR}/googletest
add_subdirectory(${CPPDAP_DIR})
```

---

Note: This is not an officially supported Google product
