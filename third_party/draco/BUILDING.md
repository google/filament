_**Contents**_

  * [CMake Basics](#cmake-basics)
  * [Mac OS X](#mac-os-x)
  * [Windows](#windows)
  * [CMake Build Configuration](#cmake-build-configuration)
    * [Debugging and Optimization](#debugging-and-optimization)
    * [Googletest Integration](#googletest-integration)
    * [Javascript Encoder/Decoder](#javascript-encoderdecoder)
  * [Android Studio Project Integration](#android-studio-project-integration)
  * [Native Android Builds](#native-android-builds)
  * [vcpkg](#vcpkg)

Building
========
For all platforms, you must first generate the project/make files and then
compile the examples.

CMake Basics
------------

To generate project/make files for the default toolchain on your system, run
`cmake` from a directory where you would like to generate build files, and pass
it the path to your Draco repository.

E.g. Starting from Draco root.

~~~~~ bash
$ mkdir build_dir && cd build_dir
$ cmake ../
~~~~~

On Windows, the above command will produce Visual Studio project files for the
newest Visual Studio detected on the system. On Mac OS X and Linux systems,
the above command will produce a `makefile`.

To control what types of projects are generated, add the `-G` parameter to the
`cmake` command. This argument must be followed by the name of a generator.
Running `cmake` with the `--help` argument will list the available
generators for your system.

Mac OS X
---------

On Mac OS X, run the following command to generate Xcode projects:

~~~~~ bash
$ cmake ../ -G Xcode
~~~~~

Windows
-------

On a Windows box you would run the following command to generate Visual Studio
2017 projects:

~~~~~ bash
C:\Users\nobody> cmake ../ -G "Visual Studio 15 2017"
~~~~~

To generate 64-bit Windows Visual Studio 2017 projects:

~~~~~ bash
C:\Users\nobody> cmake ../ -G "Visual Studio 15 2017 Win64"
~~~~~


CMake Build Configuration
-------------------------

Debugging and Optimization
--------------------------

Unlike Visual Studio and Xcode projects, the build configuration for make
builds is controlled when you run `cmake`. The following examples demonstrate
various build configurations.

Omitting the build type produces makefiles that use release build flags
by default:

~~~~~ bash
$ cmake ../
~~~~~

A makefile using release (optimized) flags is produced like this:

~~~~~ bash
$ cmake ../ -DCMAKE_BUILD_TYPE=release
~~~~~

A release build with debug info can be produced as well:

~~~~~ bash
$ cmake ../ -DCMAKE_BUILD_TYPE=relwithdebinfo
~~~~~

And your standard debug build will be produced using:

~~~~~ bash
$ cmake ../ -DCMAKE_BUILD_TYPE=debug
~~~~~

To enable the use of sanitizers when the compiler in use supports them, set the
sanitizer type when running CMake:

~~~~~ bash
$ cmake ../ -DSANITIZE=address
~~~~~

Googletest Integration
----------------------

Draco includes testing support built using Googletest. To enable Googletest unit
test support the ENABLE_TESTS cmake variable must be turned on at cmake
generation time:

~~~~~ bash
$ cmake ../ -DENABLE_TESTS=ON
~~~~~

When cmake is used as shown in the above example the Draco cmake file assumes
that the Googletest source directory is a sibling of the Draco repository. To
change the location to something else use the GTEST_SOURCE_DIR cmake variable:

~~~~~ bash
$ cmake ../ -DENABLE_TESTS=ON -DGTEST_SOURCE_DIR=path/to/googletest
~~~~~

To run the tests just execute `draco_tests` from your toolchain's build output
directory.

WebAssembly Decoder
-------------------

The WebAssembly decoder can be built using the existing cmake build file by
passing the path the Emscripten's cmake toolchain file at cmake generation time
in the CMAKE_TOOLCHAIN_FILE variable and enabling the WASM build option.
In addition, the EMSCRIPTEN environment variable must be set to the local path
of the parent directory of the Emscripten tools directory.

~~~~~ bash
# Make the path to emscripten available to cmake.
$ export EMSCRIPTEN=/path/to/emscripten/tools/parent

# Emscripten.cmake can be found within your Emscripten installation directory,
# it should be the subdir: cmake/Modules/Platform/Emscripten.cmake
$ cmake ../ -DCMAKE_TOOLCHAIN_FILE=/path/to/Emscripten.cmake -DENABLE_WASM=ON

# Build the WebAssembly decoder.
$ make

# Run the Javascript wrapper through Closure.
$ java -jar closure.jar --compilation_level SIMPLE --js draco_decoder.js --js_output_file draco_wasm_wrapper.js

~~~~~

WebAssembly Mesh Only Decoder
-----------------------------

~~~~~ bash

# cmake command line for mesh only WebAssembly decoder.
$ cmake ../ -DCMAKE_TOOLCHAIN_FILE=/path/to/Emscripten.cmake -DENABLE_WASM=ON -DENABLE_POINT_CLOUD_COMPRESSION=OFF

~~~~~

WebAssembly Point Cloud Only Decoder
-----------------------------

~~~~~ bash

# cmake command line for point cloud only WebAssembly decoder.
$ cmake ../ -DCMAKE_TOOLCHAIN_FILE=/path/to/Emscripten.cmake -DENABLE_WASM=ON -DENABLE_MESH_COMPRESSION=OFF

~~~~~

Javascript Encoder/Decoder
------------------

The javascript encoder and decoder can be built using the existing cmake build
file by passing the path the Emscripten's cmake toolchain file at cmake
generation time in the CMAKE_TOOLCHAIN_FILE variable.
In addition, the EMSCRIPTEN environment variable must be set to the local path
of the parent directory of the Emscripten tools directory.

*Note* The WebAssembly decoder should be favored over the JavaScript decoder.

~~~~~ bash
# Make the path to emscripten available to cmake.
$ export EMSCRIPTEN=/path/to/emscripten/tools/parent

# Emscripten.cmake can be found within your Emscripten installation directory,
# it should be the subdir: cmake/Modules/Platform/Emscripten.cmake
$ cmake ../ -DCMAKE_TOOLCHAIN_FILE=/path/to/Emscripten.cmake

# Build the Javascript encoder and decoder.
$ make
~~~~~

Native Android Builds
---------------------

It's sometimes useful to build Draco command line tools and run them directly on
Android devices via adb.

~~~~~ bash
# All targets require CMAKE_ANDROID_NDK. It must be set in the environment.
$ export CMAKE_ANDROID_NDK=path/to/ndk

# arm
$ cmake ../ -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/armv7-android-ndk-libcpp.cmake
$ make

# arm64
$ cmake ../ -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/arm64-android-ndk-libcpp.cmake
$ make

# x86
$ cmake ../ -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/x86-android-ndk-libcpp.cmake
$ make

# x86_64
$ cmake ../ -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/x86_64-android-ndk-libcpp.cmake
$ make
~~~~~

After building the tools they can be moved to an android device via the use of
`adb push`, and then run within an `adb shell` instance.


Android Studio Project Integration
----------------------------------

Tested on Android Studio 3.5.3.


Draco - Static Library
----------------------

To include Draco in an existing or new Android Studio project, reference it
from the `cmake` file of an existing native project that has a minimum SDK
version of 18 or higher. The project must support C++11.
To add Draco to your project:

  1. Create a new "Native C++" project.

  2. Add the following somewhere within the `CMakeLists.txt` for your project
     before the `add_library()` for your project's native-lib:

     ~~~~~ cmake
     # Note "/path/to/draco" must be changed to the path where you have cloned
     # the Draco sources.

     add_subdirectory(/path/to/draco
                      ${CMAKE_BINARY_DIR}/draco_build)
     include_directories("${CMAKE_BINARY_DIR}" /path/to/draco)
     ~~~~~

  3. Add the library target "draco" to the `target_link_libraries()` call for
     your project's native-lib. The `target_link_libraries()` call for an
     empty activity native project looks like this after the addition of
     Draco:

     ~~~~~ cmake
     target_link_libraries( # Specifies the target library.
                            native-lib

                            # Tells cmake this build depends on libdraco.
                            draco

                            # Links the target library to the log library
                            # included in the NDK.
                            ${log-lib} )

vcpkg
---------------------
You can download and install Draco using the [vcpkg](https://github.com/Microsoft/vcpkg/) dependency manager:

    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    ./vcpkg integrate install
    vcpkg install draco

The Draco port in vcpkg is kept up to date by Microsoft team members and community contributors. If the version is out of date, please [create an issue or pull request](https://github.com/Microsoft/vcpkg) on the vcpkg repository.
