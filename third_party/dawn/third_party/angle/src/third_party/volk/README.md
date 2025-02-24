# 🐺 volk [![Build Status](https://travis-ci.org/zeux/volk.svg?branch=master)](https://travis-ci.org/zeux/volk)

## ANGLE Integration

Note that the entirety of the volk README.md is included below. This is an additional section with information
on volk's integration into ANGLE. The volk source files are copied directly from the volk GitHub repo.
To update volk in ANGLE, copy the latest volk.h/c files into ANGLE "src/third_party/volk" dir and push update.
If any changes are made to volk source files locally within ANGLE, please also make corresponding PRs so that
the changes land in the upstream source volk GitHub repo.
Make sure to change tabs to spaces, which clang-format later fixes, after updating the files.
Also make sure to update the `Revision:` field in `README.chromium`.

## Purpose

volk is a meta-loader for Vulkan. It allows you to dynamically load entrypoints required to use Vulkan
without linking to vulkan-1.dll or statically linking Vulkan loader. Additionally, volk simplifies the use of Vulkan extensions by automatically loading all associated entrypoints. Finally, volk enables loading
Vulkan entrypoints directly from the driver which can increase performance by skipping loader dispatch overhead.

volk is written in C89 and supports Windows, Linux, Android and macOS (via MoltenVK).

## Building

There are multiple ways to use volk in your project:

1. You can just add `volk.c` to your build system. Note that the usual preprocessor defines that enable Vulkan's platform-specific functions (VK_USE_PLATFORM_WIN32_KHR, VK_USE_PLATFORM_XLIB_KHR, VK_USE_PLATFORM_MACOS_MVK, etc) must be passed as desired to the compiler when building `volk.c`.
2. You can use volk in header-only fashion. Include `volk.h` whereever you want to use Vulkan functions. In exactly one source file, define `VOLK_IMPLEMENTATION` before including `volk.h`. Do not build `volk.c` at all in this case. This method of integrating volk makes it possible to set the platform defines mentioned above with arbitrary (preprocessor) logic in your code.
3. You can use provided CMake files, with the usage detailed below.

## Basic usage

To use volk, you have to include `volk.h` instead of `vulkan/vulkan.h`; this is necessary to use function definitions from volk.
If some files in your application include `vulkan/vulkan.h` and don't include `volk.h`, this can result in symbol conflicts; consider defining `VK_NO_PROTOTYPES` when compiling code that uses Vulkan to make sure this doesn't happen.

To initialize volk, call this function first:

```c++
VkResult volkInitialize();
```

This will attempt to load Vulkan loader from the system; if this function returns `VK_SUCCESS` you can proceed to create Vulkan instance.
If this function fails, this means Vulkan loader isn't installed on your system.

After creating the Vulkan instance using Vulkan API, call this function:

```c++
void volkLoadInstance(VkInstance instance);
```

This function will load all required Vulkan entrypoints, including all extensions; you can use Vulkan from here on as usual.

## Optimizing device calls

If you use volk as described in the previous section, all device-related function calls, such as `vkCmdDraw`, will go through Vulkan loader dispatch code.
This allows you to transparently support multiple VkDevice objects in the same application, but comes at a price of dispatch overhead which can be as high as 7% depending on the driver and application.

To avoid this, you have one of two options:

1. For applications that use just one VkDevice object, load device-related Vulkan entrypoints directly from the driver with this function:

```c++
void volkLoadDevice(VkDevice device);
```

2. For applications that use multiple VkDevice objects, load device-related Vulkan entrypoints into a table:

```c++
void volkLoadDeviceTable(struct VolkDeviceTable* table, VkDevice device);
```

The second option requires you to change the application code to store one `VolkDeviceTable` per `VkDevice` and call functions from this table instead.

Device entrypoints are loaded using `vkGetDeviceProcAddr`; when no layers are present, this commonly results in most function pointers pointing directly at the driver functions, minimizing the call overhead. When layers are loaded, the entrypoints will point at the implementations in the first applicable layer, so this is compatible with any layers including validation layers.

## CMake support

If your project uses CMake, volk provides you with targets corresponding to the different use cases:

1. Target `volk` is a static library. Any platform defines can be passed to the compiler by setting `VOLK_STATIC_DEFINES`. Example:
```cmake
if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif()
   ...
endif()
add_subdirectory(volk)
target_link_library(my_application PRIVATE volk)
```
2. Target `volk_headers` is an interface target for the header-only style. Example:
```cmake
add_subdirectory(volk)
target_link_library(my_application PRIVATE volk_headers)
```
and in the code:
```c
/* ...any logic setting VK_USE_PLATFORM_WIN32_KHR and friends... */
#define VOLK_IMPLEMENTATION
#include "volk.h"
```

The above example use `add_subdirectory` to include volk into CMake's build tree. This is a good choice if you copy the volk files into your project tree or as a git submodule.

Volk also supports installation and config-file packages. Installation is disabled by default (so as to not pollute user projects with install rules), and can be disabled by passing `-DVOLK_INSTALL=ON` to CMake. Once installed, do something like `find_package(volk CONFIG REQUIRED)` in your project's CMakeLists.txt. The imported volk targets are called `volk::volk` and `volk::volk_headers`.

## License

This library is available to anybody free of charge, under the terms of MIT License (see LICENSE.md).
