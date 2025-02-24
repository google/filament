# GLFW

[![Build status](https://github.com/glfw/glfw/actions/workflows/build.yml/badge.svg)](https://github.com/glfw/glfw/actions)
[![Build status](https://ci.appveyor.com/api/projects/status/0kf0ct9831i5l6sp/branch/master?svg=true)](https://ci.appveyor.com/project/elmindreda/glfw)

## Introduction

GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan
application development.  It provides a simple, platform-independent API for
creating windows, contexts and surfaces, reading input, handling events, etc.

GLFW natively supports Windows, macOS and Linux and other Unix-like systems.  On
Linux both Wayland and X11 are supported.

GLFW is licensed under the [zlib/libpng
license](https://www.glfw.org/license.html).

You can [download](https://www.glfw.org/download.html) the latest stable release
as source or Windows binaries.  Each release starting with 3.0 also has
a corresponding [annotated tag](https://github.com/glfw/glfw/releases) with
source and binary archives.

The [documentation](https://www.glfw.org/docs/latest/) is available online and is
included in all source and binary archives.  See the [release
notes](https://www.glfw.org/docs/latest/news.html) for new features, caveats and
deprecations in the latest release.  For more details see the [version
history](https://www.glfw.org/changelog.html).

The `master` branch is the stable integration branch and _should_ always compile
and run on all supported platforms, although details of newly added features may
change until they have been included in a release.  New features and many bug
fixes live in [other branches](https://github.com/glfw/glfw/branches/all) until
they are stable enough to merge.

If you are new to GLFW, you may find the
[tutorial](https://www.glfw.org/docs/latest/quick.html) for GLFW 3 useful.  If
you have used GLFW 2 in the past, there is a [transition
guide](https://www.glfw.org/docs/latest/moving.html) for moving to the GLFW
3 API.

GLFW exists because of the contributions of [many people](CONTRIBUTORS.md)
around the world, whether by reporting bugs, providing community support, adding
features, reviewing or testing code, debugging, proofreading docs, suggesting
features or fixing bugs.


## Compiling GLFW

GLFW is written primarily in C99, with parts of macOS support being written in
Objective-C.  GLFW itself requires only the headers and libraries for your OS
and window system.  It does not need any additional headers for context creation
APIs (WGL, GLX, EGL, NSGL, OSMesa) or rendering APIs (OpenGL, OpenGL ES, Vulkan)
to enable support for them.

GLFW supports compilation on Windows with Visual C++ 2013 and later, MinGW and
MinGW-w64, on macOS with Clang and on Linux and other Unix-like systems with GCC
and Clang.  It will likely compile in other environments as well, but this is
not regularly tested.

There are [pre-compiled binaries](https://www.glfw.org/download.html) available
for all supported compilers on Windows and macOS.

See the [compilation guide](https://www.glfw.org/docs/latest/compile.html) for
more information about how to compile GLFW yourself.


## Using GLFW

See the [documentation](https://www.glfw.org/docs/latest/) for tutorials, guides
and the API reference.


## Contributing to GLFW

See the [contribution
guide](https://github.com/glfw/glfw/blob/master/docs/CONTRIBUTING.md) for
more information.


## System requirements

GLFW supports Windows XP and later and macOS 10.11 and later.  Linux and other
Unix-like systems running the X Window System are supported even without
a desktop environment or modern extensions, although some features require
a running window or clipboard manager.  The OSMesa backend requires Mesa 6.3.

See the [compatibility guide](https://www.glfw.org/docs/latest/compat.html)
in the documentation for more information.


## Dependencies

GLFW itself needs only CMake 3.4 or later and the headers and libraries for your
OS and window system.

The examples and test programs depend on a number of tiny libraries.  These are
located in the `deps/` directory.

 - [getopt\_port](https://github.com/kimgr/getopt_port/) for examples
   with command-line options
 - [TinyCThread](https://github.com/tinycthread/tinycthread) for threaded
   examples
 - [glad2](https://github.com/Dav1dde/glad) for loading OpenGL and Vulkan
   functions
 - [linmath.h](https://github.com/datenwolf/linmath.h) for linear algebra in
   examples
 - [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) for test and example UI
 - [stb\_image\_write](https://github.com/nothings/stb) for writing images to disk

The documentation is generated with [Doxygen](https://doxygen.org/) if CMake can
find that tool.


## Reporting bugs

Bugs are reported to our [issue tracker](https://github.com/glfw/glfw/issues).
Please check the [contribution
guide](https://github.com/glfw/glfw/blob/master/docs/CONTRIBUTING.md) for
information on what to include when reporting a bug.


## Changelog since 3.4

 - Added `GLFW_UNLIMITED_MOUSE_BUTTONS` input mode that allows mouse buttons beyond
   the limit of the mouse button tokens to be reported (#2423)
 - [Cocoa] Added `QuartzCore` framework as link-time dependency
 - [Cocoa] Removed support for OS X 10.10 Yosemite and earlier (#2506)
 - [Wayland] Bugfix: The fractional scaling related objects were not destroyed
 - [Wayland] Bugfix: `glfwInit` would segfault on compositor with no seat (#2517)
 - [Wayland] Bugfix: A drag entering a non-GLFW surface could cause a segfault
 - [Null] Added Vulkan 'window' surface creation via `VK_EXT_headless_surface`
 - [Null] Added EGL context creation on Mesa via `EGL_MESA_platform_surfaceless`
 - [EGL] Allowed native access on Wayland with `GLFW_CONTEXT_CREATION_API` set to
   `GLFW_NATIVE_CONTEXT_API` (#2518)


## Contact

On [glfw.org](https://www.glfw.org/) you can find the latest version of GLFW, as
well as news, documentation and other information about the project.

If you have questions related to the use of GLFW, we have a
[forum](https://discourse.glfw.org/).

If you have a bug to report, a patch to submit or a feature you'd like to
request, please file it in the
[issue tracker](https://github.com/glfw/glfw/issues) on GitHub.

Finally, if you're interested in helping out with the development of GLFW or
porting it to your favorite platform, join us on the forum or GitHub.

