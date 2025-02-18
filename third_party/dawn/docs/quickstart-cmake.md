# Quickstart With CMake

This document is designed to allow you to build Dawn with CMake and link to it from other CMake projects. If you're planning to work on Dawn specifically, please use `gclient` and `depot_tools`. It ensures
that you always get the right dependencies and compilers.

## Prerequisites

- A compatible platform (e.g. Windows, macOS, Linux, etc.). Most platforms are fully supported.
- A compatible C++ compiler supporting at least C++17. Most major compilers are supported.
- Git for interacting with the Dawn source code repository.
- CMake for building your project and Dawn. Dawn supports CMake 3.13+.

## Getting the Dawn Code

```shell
# Change to the directory where you want to create the code repository
$ git clone https://dawn.googlesource.com/dawn
Cloning into 'dawn'...
```

Git will create the repository within a directory named `dawn`. Navigate into this directory and configure the project.

## Build and install Dawn with CMake
```shell
$ cd dawn
$ cmake -S . -B out/Release -DDAWN_FETCH_DEPENDENCIES=ON -DDAWN_ENABLE_INSTALL=ON -DCMAKE_BUILD_TYPE=Release
...
-- Configuring done (34.9s)
-- Generating done (0.8s)
-- Build files have been written to: ${PWD}/out/Release
```

- `DAWN_FETCH_DEPENDENCIES=ON` uses a Python script `fetch_dawn_dependencies.py` as a less tedious alternative to setting up depot_tools and fetching dependencies via `gclient`.
- `DAWN_ENABLE_INSTALL=ON` configures the project to install binaries, headers and cmake configuration files in a directory of your choice. This allows other projects to discover Dawn in CMake and link with it.

Now you can build Dawn:

```shell
$ cmake --build out/Release
...
[100%] Linking CXX static library libdawn_utils.a
[100%] Built target dawn_utils
```

Once you have built the Dawn library, install it with `cmake`

```shell
$ cmake --install out/Release --prefix install/Release
```

This installs Dawn from the `out/Release` build tree into `install/Release`.

## Linking your code to the Dawn installation

### Create test code

```shell
$ mkdir TestDawn
$ cd TestDawn
```

Now, create a `hello_webgpu.cpp` C++ file within the `TestDawn` directory.

```cpp
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>

#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
  wgpu::InstanceDescriptor instanceDescriptor{};
  instanceDescriptor.capabilities.timedWaitAnyEnable = true;
  wgpu::Instance instance = wgpu::CreateInstance(&instanceDescriptor);
  if (instance == nullptr) {
    std::cerr << "Instance creation failed!\n";
    return EXIT_FAILURE;
  }
  // Synchronously request the adapter.
  wgpu::RequestAdapterOptions options = {};
  wgpu::Adapter adapter;
  wgpu::RequestAdapterCallbackInfo callbackInfo = {};
  callbackInfo.nextInChain = nullptr;
  callbackInfo.mode = wgpu::CallbackMode::WaitAnyOnly;
  callbackInfo.callback = [](WGPURequestAdapterStatus status,
                             WGPUAdapter adapter, const char *message,
                             void *userdata) {
    if (status != WGPURequestAdapterStatus_Success) {
      std::cerr << "Failed to get an adapter:" << message;
      return;
    }
    *static_cast<wgpu::Adapter *>(userdata) = wgpu::Adapter::Acquire(adapter);
  };
  callbackInfo.userdata = &adapter;
  instance.WaitAny(instance.RequestAdapter(&options, callbackInfo), UINT64_MAX);
  if (adapter == nullptr) {
    std::cerr << "RequestAdapter failed!\n";
    return EXIT_FAILURE;
  }

  wgpu::DawnAdapterPropertiesPowerPreference power_props{};

  wgpu::AdapterInfo info{};
  info.nextInChain = &power_props;

  adapter.GetInfo(&info);
  std::cout << "VendorID: " << std::hex << info.vendorID << std::dec << "\n";
  std::cout << "Vendor: " << info.vendor << "\n";
  std::cout << "Architecture: " << info.architecture << "\n";
  std::cout << "DeviceID: " << std::hex << info.deviceID << std::dec << "\n";
  std::cout << "Name: " << info.device << "\n";
  std::cout << "Driver description: " << info.description << "\n";
  return EXIT_SUCCESS;
}
```

### Create CMakeLists.txt file

Now, create a `CMakeLists.txt` file within the `TestDawn` directory like the following:

```cmake
cmake_minimum_required(VERSION 3.13)

project(hello_webgpu)

find_package(Dawn REQUIRED)
add_executable(hello_webgpu hello_webgpu.cpp)

# Declare dependency on the dawn::webgpu_dawn library
target_link_libraries(hello_webgpu dawn::webgpu_dawn)
```

For more information on how to create CMakeLists.txt files, consult the [CMake Tutorial](https://cmake.org/getting-started/).

### Build test project

Configure the CMake build from a fresh binary directory. This configuration is called an “out of source” build and is the preferred method for CMake projects.

```shell
$ export CMAKE_PREFIX_PATH=/path/to/dawn/install/Release
$ cmake -S . -B out/Release -DCMAKE_BUILD_TYPE=Release
...
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: ${PWD}/out/Release
```

Now build our test application:

```shell
$ cmake --build out/Release
[ 50%] Building CXX object CMakeFiles/hello_webgpu.dir/hello_webgpu.cpp.o
[100%] Linking CXX executable hello_webgpu
[100%] Built target hello_webgpu
```

Now run your binary:

```shell
$ ./out/Release/hello_webgpu
VendorID: 8086
Vendor: intel
Architecture: gen-12lp
DeviceID: 9a60
Name: Intel(R) UHD Graphics (TGL GT1)
Driver description: Intel open-source Mesa driver: Mesa 23.2.1-1ubuntu3.1~22.04.2
```

Note: You may move the contents of `install/Release` to any other directory and `cmake` will still be able to discover dawn as long as `CMAKE_PREFIX_PATH` points to the new install tree.
