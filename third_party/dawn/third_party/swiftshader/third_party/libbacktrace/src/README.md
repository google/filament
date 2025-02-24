# libbacktrace
A C library that may be linked into a C/C++ program to produce symbolic backtraces

Initially written by Ian Lance Taylor <iant@golang.org>.

This is version 1.0.
It is likely that this will always be version 1.0.

The libbacktrace library may be linked into a program or library and
used to produce symbolic backtraces.
Sample uses would be to print a detailed backtrace when an error
occurs or to gather detailed profiling information.

The libbacktrace library is provided under a BSD license.
See the source files for the exact license text.

The public functions are declared and documented in the header file
backtrace.h, which should be #include'd by a user of the library.

Building libbacktrace will generate a file backtrace-supported.h,
which a user of the library may use to determine whether backtraces
will work.
See the source file backtrace-supported.h.in for the macros that it
defines.

As of January 2018, libbacktrace only supports ELF, PE/COFF, and XCOFF
executables with DWARF debugging information.
The library is written to make it straightforward to add support for
other object file and debugging formats.

The library relies on the C++ unwind API defined at
https://itanium-cxx-abi.github.io/cxx-abi/abi-eh.html
This API is provided by GCC.
