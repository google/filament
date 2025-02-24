# Loader Testing Framework

The loader testing framework is a mocking environment which fakes many of the global systems that vulkan requires to run. This allows the writing of tests against a known driver, layer, and system configuration.

The framework consists of:
* Test ICD
* Test Layer
* Shim
* Manifest Writer
* Utility
* Test Environment
* Tests


## Running

By default the Vulkan-Loader repo doesn't enable testing.

To turn on building of the tests, set `BUILD_TESTS=ON` in the CMake configuration.

Use the CMake configuration `UPDATE_DEPS=ON` to automatically get all required test dependencies.
Or Ensure that `googletest` is in the `external` directory.
And on Windows only, ensure that the `Detours` library is in the `external` directory.

Linux & macOS only: The CMake Configuration `LOADER_ENABLE_ADDRESS_SANITIZER` can be used to
enable Address Sanitizer.

Run the test executables as normal

The executables available are:
* `test_regression`

Alternatively, in the build directory run `ctest` to start the test framework.

Use the `ctest` command line parameter `--output-on-failure` to printout logs in failing tests

Note: The test framework was not designed to allow multiple tests to be run in parallel due to the extensive use of files and folders on the system.


## Components

### Test ICD and Layer
The Test ICD and Test Layer have much of their configuration available at runtime to allow maximal test setup flexibility.
However exported functions in the icd or layer library binaries are an integral part of the configuration but are baked into the binary, thus not runtime configurable.
To account for that there are multiple binaries of the Test ICD and Test Layer, each being a distinct combination of exported functions.
The `test_icd.cpp` and `test_layer.cpp` files use macro defines to allow the build system to specify which functions should be exported.
```c
#if defined(TEST_ICD_EXPORT_NEW_FUNCTION)
FRAMEWORK_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkNewFunction() { ... }
#endif
```
The `CMakeLists.txt` is responsible for creating the various binaries for the Test ICD and Test Layer with the corresponding macros defined to alter which functions are exported.
Since the test framework needs to know where the binaries are located in the build tree, the `framework_config.h` file contains the absolute paths to all of the libraries that CMake has defined.
This allows C++ tests to include the header and know exactly what `.dll`/`.so` file to use.
Since the paths aren't known until build time, CMake processes the `framework_config.h.in` file which contains the names of the libraries that CMake recognizes and replaces the names with the path on the system to the library.

To add a new configuration for a ICD or layer, the changes needed are as follows:
* Add the relevant functions & macro guards in `test_icd.cpp` or `test_layer.cpp`
* In the `framework/icd/CMakeLists.txt` or `framework/layer/CMakeLists.txt`, add a new SHARED library using the icd/layer source files.
  * We are going to use `new_function` as the name of this new configuration for clarity in this documentation.
```cmake
add_library(test_icd_export_new_function SHARED ${TEST_ICD_SOURCES})
```
* Use `target_compile_definitions()` cmake command on the new library to add the specified exports.
```cmake
target_compile_definitions(test_icd_export_new_function PRIVATE TEST_ICD_EXPORT_NEW_FUNCTION=1)
```
* Create a new `.def` file with the same name as the library and place it in the `icd\export_definitions` or `layer\export_definitions` folder.
  * This file is needed in 32 bit windows to prevent name mangling in `.dll`.
* Add all of the exported functions in this file.
  * Include all exported functions, not just the newly added functions.
```
LIBRARY test_icd_export_new_function
EXPORTS
    <exported_function_0>
    <exported_function_1>
    ...
```
* Go back to the `CMakeLists.txt` and add this def file to the library by using `target_sources`, that way the compiler knows which .def file to use for each library
```cmake
if (WIN32)
    target_sources(test_icd_export_new_function PRIVATE export_definitions/test_icd_new_function.def)
endif()
```
* To make the library accessible to tests, add the name of the library to `framework/framework_config.h.in`
```c
#define TEST_ICD_PATH_NEW_FUNCTION "$<TARGET_FILE:test_icd_export_new_function>"
```

### Shim
Because the loader makes many calls to various OS functionality, the framework intercepts certain calls and makes a few of its own calls to OS functionality to allow proper isolation of the loader from the system it is running on.
The shim is a SHARED library on windows and apple and is a STATIC library on linux. This is due to the nature of the dynamic linker and how overriding functions operates on different operating systems.
#### Linux

On linux the dynamic linker will use functions defined in the program binary in loaded `.so`'s if the name matches, allowing easy interception of system calls.
##### Overridden functions
* opendir
* access
* fopen

#### MacOS
Redirects the following functions: opendir, access fopen.
The dynamic linker on MacOS requires a bit of tweaking to make it use functions defined in the program binary to override system functions in `.so`'s.
##### Overridden functions
* opendir
* access
* fopen

#### Windows
Windows requires a significantly larger number of functions to be intercepted to isolate it sufficiently enough for testing.
To facilitate that an external library `Detours` is used.
It is an open source Microsoft library and supports all of the necessary functionality needed for loader testing.
Note that the loader calls more system functions than are directly overridden with Detours.
This is due to some functions being used to query other functions, which the shim library intercepts and returns its own versions of system functions instead, such as `CreateDXGIFactory1`.

##### Overridden functions
* GetSidSubAuthority
* EnumAdapters2
* QueryAdapterInfo
* CM_Get_Device_ID_List_SizeW
* CM_Get_Device_ID_ListW
* CM_Locate_DevNodeW
* CM_Get_DevNode_Status
* CM_Get_Device_IDW
* CM_Get_Child
* CM_Get_DevNode_Registry_PropertyW
* CM_Get_Sibling
* GetDesc1
* CreateDXGIFactory1

### Utility

There are many utilities that the test framework and tests have access to. These include:
* Including common C and C++ headers
* `FRAMEWORK_EXPORT` - macro used for exporting shared library funtions
* Environment Variable Wrapper: `EnvVarWrapper` for creating, setting, getting, and removing environment variables in a RAII manner
* Windows API error handling helpers
* filesystem abstractions:
  * `create_folder`/`delete_folder`
  * `FolderManager`
    * Creates a new folder with the given name at construction time.
    * Allows writing manifests and files (eg, icd or layer binaries)
    * Automatically destroys the folder and all contained files at destruction
* LibraryWrapper - load and unload `.dll`/`.so`'s automatically
* DispatchableHandle - helper class for managing the creation and freeing of dispatchable handles
* VulkanFunctions - Loads the vulkan-loader and queries all used functions from it
  * If a test needs to use vulkan functions, they must be loaded here
* Vulkan Create Info Helpers - provide a nice interface for setting up a vulkan creat info struct.
  * InstanceCreateInfo
  * DeviceCreateInfo
  * DeviceQueueCreateInfo
* Comparison operators for various vulkan structs

### Test Environment

The `test_environment.h/.cpp` contains classes which organize all the disparate parts of the framework into an easy to use entity that allows setting up and configuring the environment tests run in.

The core components are:
* InstWrapper - helper to construct and then destroy a vulkan instance during tests
* DeviceWrapper - helper to construct and then destroy a vulkan device during tests
* PlatformShimWrapper - opens and configures the platform specific shim library
  * Sets up the overrides
  * Resets state (clearing out previous test state)
  * Sets the `VK_LOADER_DEBUG` env var to `all`.
* TestICD|LayerHandle - corresponds to a single ICD or Layer on the system
  * Loads the Test ICD/Layer library
  * Allows easily 'resetting' the Test ICD/Layer
* TestICD|LayerDetails - holder of data used for constructing ICD and Layers
  * Contains the name and api version of the icd or layer binary
  * LayerDetails also contains the manifest,
* FrameworkEnvironment
  * Owns the platform shim
  * Creates folders for the various manifest search locations, including
    * icd manifest
    * explicit layer manifests
    * implicit layer manifests
    * null - necessary empty folder to make the loader find nothing when we want it to
  * Sets these folders up as the redirection locations with the platform shim
  * Allows adding ICD's and Layers
    * Writes the json manifest file to the correct folder

The `FrameworkEnvironment` class is used to easily create 'environments'.

The `add_XXX()` member functions of `FrameworkEnvironment` make it easy to add drivers and layers to the environment a test runs in.

The `get_test_icd()` and `get_test_layer()` functions allow querying references to the underlying
drivers and layers that are in the environment, allowing quick modification of their behavior.

The `reset_test_icd()` and `reset_test_layer()` are similar to the above functions but additionally
reset the layer or driver to its initial state.
Use this if you need to reset a driver during a test.
These functions are called on the drivers and layers when the framework is being create in each test.
