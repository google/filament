# Compiling GLFW {#compile_guide}

[TOC]

This is about compiling the GLFW library itself.  For information on how to
build applications that use GLFW, see @ref build_guide.

GLFW uses some C99 features and does not support Visual Studio 2012 and earlier.


## Using CMake {#compile_cmake}

GLFW behaves like most other libraries that use CMake so this guide mostly
describes the standard configure, generate and compile sequence.  If you are already
familiar with this from other projects, you may want to focus on the @ref
compile_deps and @ref compile_options sections for GLFW-specific information.

GLFW uses [CMake](https://cmake.org/) to generate project files or makefiles
for your chosen development environment.  To compile GLFW, first generate these
files with CMake and then use them to compile the GLFW library. 

If you are on Windows and macOS you can [download
CMake](https://cmake.org/download/) from their site.

If you are on a Unix-like system such as Linux, FreeBSD or Cygwin or have
a package system like Fink, MacPorts or Homebrew, you can install its CMake
package.

CMake is a complex tool and this guide will only show a few of the possible ways
to set up and compile GLFW.  The CMake project has their own much more detailed
[CMake user guide][cmake-guide] that includes everything in this guide not
specific to GLFW.  It may be a useful companion to this one.

[cmake-guide]: https://cmake.org/cmake/help/latest/guide/user-interaction/


### Installing dependencies {#compile_deps}

The C/C++ development environments in Visual Studio, Xcode and MinGW-w64 come
with all necessary dependencies for compiling GLFW, but on Unix-like systems
like Linux and FreeBSD you will need a few extra packages.


#### Dependencies for Wayland and X11 {#compile_deps_wayland}

By default, both the Wayland and X11 backends are enabled on Linux and other Unix-like
systems (except macOS).  To disable one or both of these, set the @ref GLFW_BUILD_WAYLAND
or @ref GLFW_BUILD_X11 CMake options in the next step when generating build files.

To compile GLFW for both Wayland and X11, you need to have the X11, Wayland and xkbcommon
development packages installed.  On some systems a few other packages are also required.
None of the development packages above are needed to build or run programs that use an
already compiled GLFW library.

On Debian and derivatives like Ubuntu and Linux Mint you will need the `libwayland-dev`
and `libxkbcommon-dev` packages to compile for Wayland and the `xorg-dev` meta-package to
compile for X11.  These will pull in all other dependencies.

```sh
sudo apt install libwayland-dev libxkbcommon-dev xorg-dev
```

On Fedora and derivatives like Red Hat you will need the `wayland-devel` and
`libxkbcommon-devel` packages to compile for Wayland and the `libXcursor-devel`,
`libXi-devel`, `libXinerama-devel` and `libXrandr-devel` packages to compile for X11.
These will pull in all other dependencies.

```sh
sudo dnf install wayland-devel libxkbcommon-devel libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel
```

On FreeBSD you will need the `wayland`, `libxkbcommon` and `evdev-proto` packages to
compile for Wayland.  The X11 headers are installed along the end-user X11 packages, so if
you have an X server running you should have the headers as well.  If not, install the
`xorgproto` package to compile for X11.

```sh
pkg install wayland libxkbcommon evdev-proto xorgproto
```

On Cygwin Wayland is not supported but you will need the `libXcursor-devel`,
`libXi-devel`, `libXinerama-devel`, `libXrandr-devel` and `libXrender-devel` packages to
compile for X11.  These can be found in the Libs section of the GUI installer and will
pull in all other dependencies.

Once you have the required dependencies, move on to @ref compile_generate.


### Generating build files with CMake {#compile_generate}

Once you have all necessary dependencies it is time to generate the project
files or makefiles for your development environment.  CMake needs two paths for
this:

 - the path to the root directory of the GLFW source tree (not its `src`
   subdirectory)
 - the path to the directory where the generated build files and compiled
   binaries will be placed

If these are the same, it is called an in-tree build, otherwise it is called an
out-of-tree build.

Out-of-tree builds are recommended as they avoid cluttering up the source tree.
They also allow you to have several build directories for different
configurations all using the same source tree.

A common pattern when building a single configuration is to have a build
directory named `build` in the root of the source tree.


#### Generating with the CMake GUI {#compile_generate_gui}

Start the CMake GUI and set the paths to the source and build directories
described above.  Then press _Configure_ and _Generate_.

If you wish change any CMake variables in the list, press _Configure_ and then
_Generate_ to have the new values take effect.  The variable list will be
populated after the first configure step.

By default, GLFW will use Wayland and X11 on Linux and other Unix-like systems other than
macOS.  To disable support for one or both of these, set the @ref GLFW_BUILD_WAYLAND
and/or @ref GLFW_BUILD_X11 option in the GLFW section of the variable list, then apply the
new value as described above.

Once you have generated the project files or makefiles for your chosen
development environment, move on to @ref compile_compile.


#### Generating with command-line CMake {#compile_generate_cli}

To make a build directory, pass the source and build directories to the `cmake`
command.  These can be relative or absolute paths.  The build directory is
created if it doesn't already exist.

```sh
cmake -S path/to/glfw -B path/to/build
```

It is common to name the build directory `build` and place it in the root of the
source tree when only planning to build a single configuration.

```sh
cd path/to/glfw
cmake -S . -B build
```

Without other flags these will generate Visual Studio project files on Windows
and makefiles on other platforms.  You can choose other targets using the `-G`
flag.

```sh
cmake -S path/to/glfw -B path/to/build -G Xcode
```

By default, GLFW will use Wayland and X11 on Linux and other Unix-like systems other than
macOS.  To disable support for one or both of these, set the @ref GLFW_BUILD_WAYLAND
and/or @ref GLFW_BUILD_X11 CMake option.

```sh
cmake -S path/to/glfw -B path/to/build -D GLFW_BUILD_X11=0
```

Once you have generated the project files or makefiles for your chosen
development environment, move on to @ref compile_compile.


### Compiling the library {#compile_compile}

You should now have all required dependencies and the project files or makefiles
necessary to compile GLFW.  Go ahead and compile the actual GLFW library with
these files as you would with any other project.

With Visual Studio open `GLFW.sln` and use the Build menu.  With Xcode open
`GLFW.xcodeproj` and use the Project menu.

With Linux, macOS and other forms of Unix, run `make`.

```sh
cd path/to/build
make
```

With MinGW-w64, it is `mingw32-make`.

```sh
cd path/to/build
mingw32-make
```

Any CMake build directory can also be built with the `cmake` command and the
`--build` flag.

```sh
cmake --build path/to/build
```

This will run the platform specific build tool the directory was generated for.

Once the GLFW library is compiled you are ready to build your application,
linking it to the GLFW library.  See @ref build_guide for more information.


## CMake options {#compile_options}

The CMake files for GLFW provide a number of options, although not all are
available on all supported platforms.  Some of these are de facto standards
among projects using CMake and so have no `GLFW_` prefix.

If you are using the GUI version of CMake, these are listed and can be changed
from there.  If you are using the command-line version of CMake you can use the
`ccmake` ncurses GUI to set options.  Some package systems like Ubuntu and other
distributions based on Debian GNU/Linux have this tool in a separate
`cmake-curses-gui` package.

Finally, if you don't want to use any GUI, you can set options from the `cmake`
command-line with the `-D` flag.

```sh
cmake -S path/to/glfw -B path/to/build -D BUILD_SHARED_LIBS=ON
```


### Shared CMake options {#compile_options_shared}

@anchor BUILD_SHARED_LIBS
__BUILD_SHARED_LIBS__ determines whether GLFW is built as a static library or as
a DLL / shared library / dynamic library.  This is disabled by default,
producing a static GLFW library.  This variable has no `GLFW_` prefix because it
is defined by CMake.  If you want to change the library only for GLFW when it is
part of a larger project, see @ref GLFW_LIBRARY_TYPE.

@anchor GLFW_LIBRARY_TYPE
__GLFW_LIBRARY_TYPE__ allows you to override @ref BUILD_SHARED_LIBS only for
GLFW, without affecting other libraries in a larger project.  When set, the
value of this option must be a valid CMake library type.  Set it to `STATIC` to
build GLFW as a static library, `SHARED` to build it as a shared library
/ dynamic library / DLL, or `OBJECT` to make GLFW a CMake object library.

@anchor GLFW_BUILD_EXAMPLES
__GLFW_BUILD_EXAMPLES__ determines whether the GLFW examples are built
along with the library.  This is enabled by default unless GLFW is being built
as a subproject of a larger CMake project.

@anchor GLFW_BUILD_TESTS
__GLFW_BUILD_TESTS__ determines whether the GLFW test programs are
built along with the library.  This is enabled by default unless GLFW is being
built as a subproject of a larger CMake project.

@anchor GLFW_BUILD_DOCS
__GLFW_BUILD_DOCS__ determines whether the GLFW documentation is built along
with the library.  This is enabled by default if
[Doxygen](https://www.doxygen.nl/) is found by CMake during configuration.


### Win32 specific CMake options {#compile_options_win32}

@anchor GLFW_BUILD_WIN32
__GLFW_BUILD_WIN32__ determines whether to include support for Win32 when compiling the
library.  This option is only available when compiling for Windows.  This is enabled by
default.

@anchor USE_MSVC_RUNTIME_LIBRARY_DLL
__USE_MSVC_RUNTIME_LIBRARY_DLL__ determines whether to use the DLL version or the
static library version of the Visual C++ runtime library.  When enabled, the
DLL version of the Visual C++ library is used.  This is enabled by default.

It is recommended to set the standard CMake variable [CMAKE_MSVC_RUNTIME_LIBRARY][]
instead of this GLFW-specific option.

[CMAKE_MSVC_RUNTIME_LIBRARY]: https://cmake.org/cmake/help/latest/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html

@anchor GLFW_USE_HYBRID_HPG
__GLFW_USE_HYBRID_HPG__ determines whether to export the `NvOptimusEnablement` and
`AmdPowerXpressRequestHighPerformance` symbols, which force the use of the
high-performance GPU on Nvidia Optimus and AMD PowerXpress systems.  These symbols
need to be exported by the EXE to be detected by the driver, so the override
will not work if GLFW is built as a DLL.  This is disabled by default, letting
the operating system and driver decide.


### macOS specific CMake options {#compile_options_macos}

@anchor GLFW_BUILD_COCOA
__GLFW_BUILD_COCOA__ determines whether to include support for Cocoa when compiling the
library.  This option is only available when compiling for macOS.  This is enabled by
default.


### Unix-like system specific CMake options {#compile_options_unix}

@anchor GLFW_BUILD_WAYLAND
__GLFW_BUILD_WAYLAND__ determines whether to include support for Wayland when compiling
the library.  This option is only available when compiling for Linux and other Unix-like
systems other than macOS.  This is enabled by default.

@anchor GLFW_BUILD_X11
__GLFW_BUILD_X11__ determines whether to include support for X11 when compiling the
library.  This option is only available when compiling for Linux and other Unix-like
systems other than macOS.  This is enabled by default.


## Cross-compilation with CMake and MinGW-w64 {#compile_mingw_cross}

Both Cygwin and many Linux distributions have MinGW-w64 packages.  For example,
Cygwin has the `mingw64-i686-gcc` and `mingw64-x86_64-gcc` packages for 32- and
64-bit version of MinGW-w64, while Debian GNU/Linux and derivatives like Ubuntu
have the `mingw-w64` package for both.

GLFW has CMake toolchain files in the `CMake` subdirectory that set up
cross-compilation of Windows binaries.  To use these files you set the
`CMAKE_TOOLCHAIN_FILE` CMake variable with the `-D` flag add an option when
configuring and generating the build files.

```sh
cmake -S path/to/glfw -B path/to/build -D CMAKE_TOOLCHAIN_FILE=path/to/file
```

The exact toolchain file to use depends on the prefix used by the MinGW-w64
binaries on your system.  You can usually see this in the /usr directory.  For
example, both the Ubuntu and Cygwin MinGW-w64 packages have
`/usr/x86_64-w64-mingw32` for the 64-bit compilers, so the correct invocation
would be:

```sh
cmake -S path/to/glfw -B path/to/build -D CMAKE_TOOLCHAIN_FILE=CMake/x86_64-w64-mingw32.cmake
```

The path to the toolchain file is relative to the path to the GLFW source tree
passed to the `-S` flag, not to the current directory.

For more details see the [CMake toolchain guide][cmake-toolchains].

[cmake-toolchains]: https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html


## Compiling GLFW manually {#compile_manual}

If you wish to compile GLFW without its CMake build environment then you will have to do
at least some platform-detection yourself.  There are preprocessor macros for
enabling support for the platforms (window systems) available.  There are also optional,
platform-specific macros for various features.

When building, GLFW will expect the necessary configuration macros to be defined
on the command-line.  The GLFW CMake files set these as private compile
definitions on the GLFW target but if you compile the GLFW sources manually you
will need to define them yourself.

The window system is used to create windows, handle input, monitors, gamma ramps and
clipboard.  The options are:

 - @b _GLFW_COCOA to use the Cocoa frameworks
 - @b _GLFW_WIN32 to use the Win32 API
 - @b _GLFW_WAYLAND to use the Wayland protocol
 - @b _GLFW_X11 to use the X Window System

The @b _GLFW_WAYLAND and @b _GLFW_X11 macros may be combined and produces a library that
attempts to detect the appropriate platform at initialization.

If you are building GLFW as a shared library / dynamic library / DLL then you
must also define @b _GLFW_BUILD_DLL.  Otherwise, you must not define it.

If you are using a custom name for the Vulkan, EGL, GLX, OSMesa, OpenGL, GLESv1
or GLESv2 library, you can override the default names by defining those you need
of @b _GLFW_VULKAN_LIBRARY, @b _GLFW_EGL_LIBRARY, @b _GLFW_GLX_LIBRARY, @b
_GLFW_OSMESA_LIBRARY, @b _GLFW_OPENGL_LIBRARY, @b _GLFW_GLESV1_LIBRARY and @b
_GLFW_GLESV2_LIBRARY.  Otherwise, GLFW will use the built-in default names.

@note None of the @ref build_macros may be defined during the compilation of
GLFW.  If you define any of these in your build files, make sure they are not
applied to the GLFW sources.

