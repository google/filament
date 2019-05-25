Building Open Image Denoise from Source
=======================================

The latest Open Image Denoise sources are always available at the
[Open Image Denoise GitHub repository](http://github.com/OpenImageDenoise/oidn).
The default `master` branch should always point to the latest tested bugfix
release.

Prerequisites
-------------

Open Image Denoise currently supports 64-bit Linux, Windows, and macOS
operating systems. In addition, before you can build Open Image Denoise
you need the following prerequisites:

-   You can clone the latest Open Image Denoise sources via:

        git clone --recursive https://github.com/OpenImageDenoise/oidn.git

-   To build Open Image Denoise you need [CMake](http://www.cmake.org) 3.1 or
    later, a C++11 compiler (we recommend using Clang, but also support GCC,
    Microsoft Visual Studio 2015 or later, and
    [Intel® C++ Compiler](https://software.intel.com/en-us/c-compilers) 17.0 or
    later), and Python 2.7 or later.
-   Additionally you require a copy of [Intel® Threading Building
    Blocks](https://www.threadingbuildingblocks.org/) (TBB) 2017 or later.

Depending on your Linux distribution you can install these dependencies
using `yum` or `apt-get`. Some of these packages might already be installed or
might have slightly different names.

Type the following to install the dependencies using `yum`:

    sudo yum install cmake
    sudo yum install tbb-devel

Type the following to install the dependencies using `apt-get`:

    sudo apt-get install cmake-curses-gui
    sudo apt-get install libtbb-dev

Under macOS these dependencies can be installed using
[MacPorts](http://www.macports.org/):

    sudo port install cmake tbb

Under Windows please directly use the appropriate installers or packages for
[CMake](https://cmake.org/download/),
[Python](https://www.python.org/downloads/),
and [TBB](https://github.com/01org/tbb/releases).


Compiling Open Image Denoise on Linux/macOS
-------------------------------------------

Assuming the above prerequisites are all fulfilled, building Open Image Denoise
through CMake is easy:

-   Create a build directory, and go into it

        mkdir oidn/build
        cd oidn/build

    (We do recommend having separate build directories for different
    configurations such as release, debug, etc.).

-   The compiler CMake will use by default will be whatever the `CC` and
    `CXX` environment variables point to. Should you want to specify a
    different compiler, run cmake manually while specifying the desired
    compiler. The default compiler on most Linux machines is `gcc`, but
    it can be pointed to `clang` instead by executing the following:

        cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang ..

    CMake will now use Clang instead of GCC. If you are OK with using
    the default compiler on your system, then simply skip this step.
    Note that the compiler variables cannot be changed after the first
    `cmake` or `ccmake` run.

-   Open the CMake configuration dialog

        ccmake ..

-   Make sure to properly set the build mode and enable the components you
    need, etc.; then type 'c'onfigure and 'g'enerate. When back on the
    command prompt, build it using

        make

-   You should now have `libOpenImageDenoise.so` as well as a set of example
    applications.


Compiling Open Image Denoise on Windows
---------------------------------------

On Windows using the CMake GUI (`cmake-gui.exe`) is the most convenient way to
configure Open Image Denoise and to create the Visual Studio solution files:

-   Browse to the Open Image Denoise sources and specify a build directory (if
    it does not exist yet CMake will create it).

-   Click "Configure" and select as generator the Visual Studio version you
    have (Open Image Denoise needs Visual Studio 14 2015 or newer), for Win64
    (32-bit builds are not supported), e.g., "Visual Studio 15 2017 Win64".

-   If the configuration fails because some dependencies could not be found
    then follow the instructions given in the error message, e.g., set the
    variable `TBB_ROOT` to the folder where TBB was installed.

-   Optionally change the default build options, and then click "Generate" to
    create the solution and project files in the build directory.

-   Open the generated `OpenImageDenoise.sln` in Visual Studio, select the
    build configuration and compile the project.


Alternatively, Open Image Denoise can also be built without any GUI, entirely on the
console. In the Visual Studio command prompt type:

    cd path\to\oidn
    mkdir build
    cd build
    cmake -G "Visual Studio 15 2017 Win64" [-D VARIABLE=value] ..
    cmake --build . --config Release

Use `-D` to set variables for CMake, e.g., the path to TBB with "`-D
TBB_ROOT=\path\to\tbb`".


CMake Configuration
-------------------

The default CMake configuration in the configuration dialog should be appropriate
for most usages. The following list describes the options that can be configured
in CMake:

- `CMAKE_BUILD_TYPE`: Can be used to switch between Debug mode
  (Debug), Release mode (Release) (default), and Release mode with
  enabled assertions and debug symbols (RelWithDebInfo).

- `OIDN_STATIC_LIB`: Builds Open Image Denoise as a static library (OFF by
  default). CMake 3.13.0 or later is required to enable this option. When using
  the statically compiled Open Image Denoise library, you either have to use
  the generated CMake configuration files (recommended), or you have to
  manually define `OIDN_STATIC_LIB` before including the library headers in your
  application.

- `TBB_ROOT`: The path to the TBB installation (autodetected by default).
