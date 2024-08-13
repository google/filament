# GLFW binaries for 64-bit Windows

This archive contains documentation, headers, pre-compiled static libraries,
import libraries and DLLs for GLFW 3.4.

Binaries for the following compilers are included

 - Visual C++ 2022 (built with 17.9.0)
 - Visual C++ 2019 (built with 16.11.34)
 - Visual C++ 2017 (built with 15.9.60)
 - Visual C++ 2015 (built with 14.0.25431.01)
 - Visual C++ 2013 (built with 12.0.40629.00)
 - MinGW-w64 (built with 13.2.0-win32-dwarf-msvcrt)


## Binaries for Visual C++

All binaries for Visual C++ 2017 and earlier are compatible with Windows XP, but
this is not supported by Visual C++ 2019.  This support has been deprecated by
Microsoft and GLFW will also drop support for Windows XP in a future release.

### GLFW as a DLL

To use GLFW as a DLL, link against the `glfw3dll.lib` file for your
environment.  This will add a load time dependency on `glfw3.dll`.  The
remaining files in the same directory are not needed.

This DLL is built in release mode for the Multithreaded DLL runtime library.

There is also a GLFW DLL and import library pair in the `lib-static-ucrt`
directory.  These are built with Visual C++ 2019 and the static Multithreaded
runtime library.

### GLFW as a static library

To use GLFW as a static library, link against `glfw3.lib` if your application
is using the Multithreaded DLL runtime library, or `glfw3_mt.lib` if it is
using the static Multithreaded runtime library.  The remaining files in the same
directory are not needed.

The static libraries are built in release mode and do not contain debug
information but can still be linked with the debug versions of the runtime
library.


## Binaries for MinGW-w64

### GLFW as a DLL

To use GLFW as a DLL, link against the `libglfw3dll.a` file for your
environment.  This will add a load time dependency on `glfw3.dll`.  The
remaining files in the same directory are not needed.

The DLLs are built in release mode.

The DLLs depend on the `msvcrt.dll` C runtime library.  There is also a GLFW
DLL and import library in the `lib-static-ucrt` directory that is built with
Visual C++ 2019 and statically linked against the UCRT.

All DLLs in this archive provide the same ABI and can be used as drop-in
replacements for one another, as long as the C runtime library they depend on is
available.

### GLFW as a static library

To use GLFW as a static library, link against the `libglfw3.a` file for your
environment.  The other files in the same directory are not needed.

The library is built in release mode and do not contain debug information.

