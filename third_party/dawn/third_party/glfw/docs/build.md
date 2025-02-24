# Building applications {#build_guide}

[TOC]

This is about compiling and linking applications that use GLFW.  For information on
how to write such applications, start with the
[introductory tutorial](@ref quick_guide).  For information on how to compile
the GLFW library itself, see @ref compile_guide.

This is not a tutorial on compilation or linking.  It assumes basic
understanding of how to compile and link a C program as well as how to use the
specific compiler of your chosen development environment.  The compilation
and linking process should be explained in your C programming material and in
the documentation for your development environment.


## Including the GLFW header file {#build_include}

You should include the GLFW header in the source files where you use OpenGL or
GLFW.

```c
#include <GLFW/glfw3.h>
```

This header defines all the constants and declares all the types and function
prototypes of the GLFW API.  By default, it also includes the OpenGL header from
your development environment.  See [option macros](@ref build_macros) below for
how to select OpenGL ES headers and more.

The GLFW header also defines any platform-specific macros needed by your OpenGL
header, so that it can be included without needing any window system headers.

It does this only when needed, so if window system headers are included, the
GLFW header does not try to redefine those symbols.  The reverse is not true,
i.e. `windows.h` cannot cope if any Win32 symbols have already been defined.

In other words:

 - Use the GLFW header to include OpenGL or OpenGL ES headers portably
 - Do not include window system headers unless you will use those APIs directly
 - If you do need such headers, include them before the GLFW header

If you are using an OpenGL extension loading library such as [glad][], the
extension loader header should be included before the GLFW one.  GLFW attempts
to detect any OpenGL or OpenGL ES header or extension loader header included
before it and will then disable the inclusion of the default OpenGL header.
Most extension loaders also define macros that disable similar headers below it.

[glad]: https://github.com/Dav1dde/glad

```c
#include <glad/gl.h>
#include <GLFW/glfw3.h>
```

Both of these mechanisms depend on the extension loader header defining a known
macro.  If yours doesn't or you don't know which one your users will pick, the
@ref GLFW_INCLUDE_NONE macro will explicitly prevent the GLFW header from
including the OpenGL header.  This will also allow you to include the two
headers in any order.

```c
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
```


### GLFW header option macros {#build_macros}

These macros may be defined before the inclusion of the GLFW header and affect
its behavior.

@anchor GLFW_DLL
__GLFW_DLL__ is required on Windows when using the GLFW DLL, to tell the
compiler that the GLFW functions are defined in a DLL.

The following macros control which OpenGL or OpenGL ES API header is included.
Only one of these may be defined at a time.

@note GLFW does not provide any of the API headers mentioned below.  They are
provided by your development environment or your OpenGL, OpenGL ES or Vulkan
SDK, and most of them can be downloaded from the [Khronos Registry][registry].

[registry]: https://www.khronos.org/registry/

@anchor GLFW_INCLUDE_GLCOREARB
__GLFW_INCLUDE_GLCOREARB__ makes the GLFW header include the modern
`GL/glcorearb.h` header (`OpenGL/gl3.h` on macOS) instead of the regular OpenGL
header.

@anchor GLFW_INCLUDE_ES1
__GLFW_INCLUDE_ES1__ makes the GLFW header include the OpenGL ES 1.x `GLES/gl.h`
header instead of the regular OpenGL header.

@anchor GLFW_INCLUDE_ES2
__GLFW_INCLUDE_ES2__ makes the GLFW header include the OpenGL ES 2.0
`GLES2/gl2.h` header instead of the regular OpenGL header.

@anchor GLFW_INCLUDE_ES3
__GLFW_INCLUDE_ES3__ makes the GLFW header include the OpenGL ES 3.0
`GLES3/gl3.h` header instead of the regular OpenGL header.

@anchor GLFW_INCLUDE_ES31
__GLFW_INCLUDE_ES31__ makes the GLFW header include the OpenGL ES 3.1
`GLES3/gl31.h` header instead of the regular OpenGL header.

@anchor GLFW_INCLUDE_ES32
__GLFW_INCLUDE_ES32__ makes the GLFW header include the OpenGL ES 3.2
`GLES3/gl32.h` header instead of the regular OpenGL header.

@anchor GLFW_INCLUDE_NONE
__GLFW_INCLUDE_NONE__ makes the GLFW header not include any OpenGL or OpenGL ES
API header.  This is useful in combination with an extension loading library.

If none of the above inclusion macros are defined, the standard OpenGL `GL/gl.h`
header (`OpenGL/gl.h` on macOS) is included, unless GLFW detects the inclusion
guards of any OpenGL, OpenGL ES or extension loader header it knows about.

The following macros control the inclusion of additional API headers.  Any
number of these may be defined simultaneously, and/or together with one of the
above macros.

@anchor GLFW_INCLUDE_VULKAN
__GLFW_INCLUDE_VULKAN__ makes the GLFW header include the Vulkan
`vulkan/vulkan.h` header in addition to any selected OpenGL or OpenGL ES header.

@anchor GLFW_INCLUDE_GLEXT
__GLFW_INCLUDE_GLEXT__ makes the GLFW header include the appropriate extension
header for the OpenGL or OpenGL ES header selected above after and in addition
to that header.

@anchor GLFW_INCLUDE_GLU
__GLFW_INCLUDE_GLU__ makes the header include the GLU header in addition to the
header selected above.  This should only be used with the standard OpenGL header
and only for compatibility with legacy code.  GLU has been deprecated and should
not be used in new code.

@note None of these macros may be defined during the compilation of GLFW itself.
If your build includes GLFW and you define any these in your build files, make
sure they are not applied to the GLFW sources.


## Link with the right libraries {#build_link}

GLFW is essentially a wrapper of various platform-specific APIs and therefore
needs to link against many different system libraries.  If you are using GLFW as
a shared library / dynamic library / DLL then it takes care of these links.
However, if you are using GLFW as a static library then your executable will
need to link against these libraries.

On Windows and macOS, the list of system libraries is static and can be
hard-coded into your build environment.  See the section for your development
environment below.  On Linux and other Unix-like operating systems, the list
varies but can be retrieved in various ways as described below.

A good general introduction to linking is [Beginner's Guide to
Linkers][linker_guide] by David Drysdale.

[linker_guide]: https://www.lurklurk.org/linkers/linkers.html


### With Visual C++ and GLFW binaries {#build_link_win32}

If you are using a downloaded [binary
archive](https://www.glfw.org/download.html), first make sure you have the
archive matching the architecture you are building for (32-bit or 64-bit), or
you will get link errors.  Also make sure you are using the binaries for your
version of Visual C++ or you may get other link errors.

There are two version of the static GLFW library in the binary archive, because
it needs to use the same base run-time library variant as the rest of your
executable.

One is named `glfw3.lib` and is for projects with the _Runtime Library_ project
option set to _Multi-threaded DLL_ or _Multi-threaded Debug DLL_.  The other is
named `glfw3_mt.lib` and is for projects with _Runtime Library_ set to
_Multi-threaded_ or _Multi-threaded Debug_.  To use the static GLFW library you
will need to add `path/to/glfw3.lib` or `path/to/glfw3_mt.lib` to the
_Additional Dependencies_ project option.

If you compiled a GLFW static library yourself then there will only be one,
named `glfw3.lib`, and you have to make sure the run-time library variant
matches.

The DLL version of the GLFW library is named `glfw3.dll`, but you will be
linking against the `glfw3dll.lib` link library.  To use the DLL you will need
to add `path/to/glfw3dll.lib` to the _Additional Dependencies_ project option.
All of its dependencies are already listed there by default, but when building
with the DLL version of GLFW, you also need to define the @ref GLFW_DLL.  This
can be done either in the _Preprocessor Definitions_ project option or by
defining it in your source code before including the GLFW header.

```c
#define GLFW_DLL
#include <GLFW/glfw3.h>
```

All link-time dependencies for GLFW are already listed in the _Additional
Dependencies_ option by default.


### With MinGW-w64 and GLFW binaries {#build_link_mingw}

This is intended for building a program from the command-line or by writing
a makefile, on Windows with [MinGW-w64][] and GLFW binaries.  These can be from
a downloaded and extracted [binary archive](https://www.glfw.org/download.html)
or by compiling GLFW yourself.  The paths below assume a binary archive is used.

If you are using a downloaded binary archive, first make sure you have the
archive matching the architecture you are building for (32-bit or 64-bit) or you
will get link errors.

Note that the order of source files and libraries matter for GCC.  Dependencies
must be listed after the files that depend on them.  Any source files that
depend on GLFW must be listed before the GLFW library.  GLFW in turn depends on
`gdi32` and must be listed before it.

[MinGW-w64]: https://www.mingw-w64.org/

If you are using the static version of the GLFW library, which is named
`libglfw3.a`, do:

```sh
gcc -o myprog myprog.c -I path/to/glfw/include path/to/glfw/lib-mingw-w64/libglfw3.a -lgdi32
```

If you are using the DLL version of the GLFW library, which is named
`glfw3.dll`, you will need to use the `libglfw3dll.a` link library.

```sh
gcc -o myprog myprog.c -I path/to/glfw/include path/to/glfw/lib-mingw-w64/libglfw3dll.a -lgdi32
```

The resulting executable will need to find `glfw3.dll` to run, typically by
keeping both files in the same directory.

When you are building with the DLL version of GLFW, you will also need to define
the @ref GLFW_DLL macro.  This can be done in your source files, as long as it
done before including the GLFW header:

```c
#define GLFW_DLL
#include <GLFW/glfw3.h>
```

It can also be done on the command-line:

```sh
gcc -o myprog myprog.c -D GLFW_DLL -I path/to/glfw/include path/to/glfw/lib-mingw-w64/libglfw3dll.a -lgdi32
```


### With CMake and GLFW source {#build_link_cmake_source}

This section is about using CMake to compile and link GLFW along with your
application.  If you want to use an installed binary instead, see @ref
build_link_cmake_package.

With a few changes to your `CMakeLists.txt` you can have the GLFW source tree
built along with your application.

Add the root directory of the GLFW source tree to your project.  This will add
the `glfw` target to your project.

```cmake
add_subdirectory(path/to/glfw)
```

Once GLFW has been added, link your application against the `glfw` target.
This adds the GLFW library and its link-time dependencies as it is currently
configured, the include directory for the GLFW header and, when applicable, the
@ref GLFW_DLL macro.

```cmake
target_link_libraries(myapp glfw)
```

Note that the `glfw` target does not depend on OpenGL, as GLFW loads any OpenGL,
OpenGL ES or Vulkan libraries it needs at runtime.  If your application calls
OpenGL directly, instead of using a modern
[extension loader library](@ref context_glext_auto), use the OpenGL CMake
package.

```cmake
find_package(OpenGL REQUIRED)
```

If OpenGL is found, the `OpenGL::GL` target is added to your project, containing
library and include directory paths.  Link against this like any other library.

```cmake
target_link_libraries(myapp OpenGL::GL)
```

For a minimal example of a program and GLFW sources built with CMake, see the
[GLFW CMake Starter][cmake_starter] on GitHub.

[cmake_starter]: https://github.com/juliettef/GLFW-CMake-starter


### With CMake and installed GLFW binaries {#build_link_cmake_package}

This section is about using CMake to link GLFW after it has been built and
installed.  If you want to build it along with your application instead, see
@ref build_link_cmake_source.

With a few changes to your `CMakeLists.txt` you can locate the package and
target files generated when GLFW is installed.

```cmake
find_package(glfw3 3.5 REQUIRED)
```

Once GLFW has been added to the project, link against it with the `glfw` target.
This adds the GLFW library and its link-time dependencies, the include directory
for the GLFW header and, when applicable, the @ref GLFW_DLL macro.

```cmake
target_link_libraries(myapp glfw)
```

Note that the `glfw` target does not depend on OpenGL, as GLFW loads any OpenGL,
OpenGL ES or Vulkan libraries it needs at runtime.  If your application calls
OpenGL directly, instead of using a modern
[extension loader library](@ref context_glext_auto), use the OpenGL CMake
package.

```cmake
find_package(OpenGL REQUIRED)
```

If OpenGL is found, the `OpenGL::GL` target is added to your project, containing
library and include directory paths.  Link against this like any other library.

```cmake
target_link_libraries(myapp OpenGL::GL)
```


### With pkg-config and GLFW binaries on Unix {#build_link_pkgconfig}

This is intended for building a program from the command-line or by writing
a makefile, on macOS or any Unix-like system like Linux, FreeBSD and Cygwin.

GLFW supports [pkg-config][], and the `glfw3.pc` pkg-config file is generated
when the GLFW library is built and is installed along with it.  A pkg-config
file describes all necessary compile-time and link-time flags and dependencies
needed to use a library.  When they are updated or if they differ between
systems, you will get the correct ones automatically.

[pkg-config]: https://www.freedesktop.org/wiki/Software/pkg-config/

A typical compile and link command-line when using the static version of the
GLFW library may look like this:

```sh
cc $(pkg-config --cflags glfw3) -o myprog myprog.c $(pkg-config --static --libs glfw3)
```

If you are using the shared version of the GLFW library, omit the `--static`
flag.

```sh
cc $(pkg-config --cflags glfw3) -o myprog myprog.c $(pkg-config --libs glfw3)
```

You can also use the `glfw3.pc` file without installing it first, by using the
`PKG_CONFIG_PATH` environment variable.

```sh
env PKG_CONFIG_PATH=path/to/glfw/src cc $(pkg-config --cflags glfw3) -o myprog myprog.c $(pkg-config --libs glfw3)
```

The dependencies do not include OpenGL, as GLFW loads any OpenGL, OpenGL ES or
Vulkan libraries it needs at runtime.  If your application calls OpenGL
directly, instead of using a modern
[extension loader library](@ref context_glext_auto), you should add the `gl`
pkg-config package.

```sh
cc $(pkg-config --cflags glfw3 gl) -o myprog myprog.c $(pkg-config --libs glfw3 gl)
```


### With Xcode on macOS {#build_link_xcode}

If you are using the dynamic library version of GLFW, add it to the project
dependencies.

If you are using the static library version of GLFW, add it and the Cocoa,
OpenGL, IOKit and QuartzCore frameworks to the project as dependencies.  They
can all be found in `/System/Library/Frameworks`.


### With command-line or makefile on macOS {#build_link_osx}

It is recommended that you use [pkg-config](@ref build_link_pkgconfig) when
using installed GLFW binaries from the command line on macOS.  That way you will
get any new dependencies added automatically.  If you still wish to build
manually, you need to add the required frameworks and libraries to your
command-line yourself using the `-l` and `-framework` switches.

If you are using the dynamic GLFW library, which is named `libglfw.3.dylib`, do:

```sh
cc -o myprog myprog.c -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework QuartzCore
```

If you are using the static library, named `libglfw3.a`, substitute `-lglfw3`
for `-lglfw`.

Note that you do not add the `.framework` extension to a framework when linking
against it from the command-line.

@note Your machine may have `libGL.*.dylib` style OpenGL library, but that is
for the X Window System and will not work with the macOS native version of GLFW.

