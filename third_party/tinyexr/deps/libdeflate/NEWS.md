# libdeflate release notes

## Version 1.25

* Fixed a build error with gcc 16.

* The libdeflate CMake package is now relocatable.

## Version 1.24

* The CMake-based build system now supports Apple Framework builds.

* libdeflate now builds for Windows ARM64EC.

* Made another small optimization to the x86 and ARM CRC32 code.

* Fixed a compiler warning on certain platforms (issue #416).

## Version 1.23

* Fixed bug introduced in 1.20 where incorrect checksums could be calculated if
  libdeflate was compiled with clang at -O0 and run on a CPU supporting AVX512.

* Fixed bug introduced in 1.20 where incorrect checksums could be calculated in
  rare cases on macOS computers that support AVX512 and are running an older
  version of macOS that contains a bug that corrupts AVX512 registers.  This
  could occur only if code outside libdeflate enabled AVX512 in the thread.

* Fixed build error when using -mno-evex512 with clang 18+ or gcc 14+.

* Increased the minimum CMake version to 3.10.

* Further optimized the x86 CRC code.

## Version 1.22

* The CMake-based build system now implements a workaround for gcc being paired
  with a too-old binutils version.  This can prevent build errors.

## Version 1.21

* Fixed build error on x86 with gcc 8.1 and gcc 8.2.

* Fixed build error on x86 when gcc 11 is paired with a binutils version that
  doesn't support AVX-VNNI, e.g. as it is on RHEL 9.

* Fixed build error on arm64 with gcc 6.

* Fixed build error on arm64 with gcc 13.1 and later with some -mcpu options.

* Enabled detection of dotprod support in Windows ARM64 builds.

## Version 1.20

* Improved CRC-32 performance on recent x86 CPUs by adding
  VPCLMULQDQ-accelerated implementations using 256-bit and 512-bit vectors.

* Improved Adler-32 performance on recent x86 CPUs by adding
  VNNI-accelerated implementations using 256-bit and 512-bit vectors.

* Improved CRC-32 and Adler-32 performance on short inputs.

* Optimized the portable implementation of Adler-32.

* Added some basic optimizations for RISC-V.

* Dropped support for gcc versions older than v4.9 (released in 2014)
  and clang versions older than v3.9 (released in 2016).

* Dropped support for CRC-32 acceleration on 32-bit ARM using the ARMv8 pmull or
  crc32 instructions.  This code only worked on CPUs that also have a 64-bit
  mode, and it was already disabled on many compiler versions due to compiler
  limitations.  CRC-32 acceleration remains fully supported on 64-bit ARM.

## Version 1.19

* Added new functions `libdeflate_alloc_compressor_ex()` and
  `libdeflate_alloc_decompressor_ex()`.  These functions allow specifying a
  custom memory allocator on a per-compressor basis.

* libdeflate now always generates Huffman codes with at least 2 codewords.  This
  fixes a compatibility issue where Windows Explorer's ZIP unpacker could not
  decompress DEFLATE streams created by libdeflate.  libdeflate's behavior was
  allowed by the DEFLATE RFC, but not all software was okay with it.  In rare
  cases, compression ratios can be slightly reduced by this change.

* Disabled the use of some compiler intrinsics on MSVC versions where they don't
  work correctly.

* libdeflate can now compress up to the exact size of the output buffer.

* Slightly improved compression performance at levels 1-9.

* Improved the compression ratio of very short inputs.

## Version 1.18

* Fixed a bug where the build type didn't default to "Release" when using
  CMake 3.10 or earlier.

* Fixed a bug where some optimized code wasn't used when building with
  Clang 15 or later (x86), or with Clang 16 or later (aarch64).

* Fixed build errors with some architecture and compiler combos:
  * aarch64 with Clang 16
  * armv6kz or armv7e-m with gcc
  * armhf with gcc (on Debian only)

## Version 1.17

(Apologies for another release so soon after v1.16, but the bug fix listed below
needed to go out.)

* Fixed a bug introduced in v1.16 where compression at levels 10-12 would
  sometimes produce an output larger than the size that was returned by the
  corresponding `libdeflate_*_compress_bound()` function.

* Converted the fuzzing scripts to use LLVM's libFuzzer and added them to the
  GitHub Actions workflow.  (This would have detected the above bug.)

* Further improved the support for direct compilation without using the official
  build system.  The top-level source directory no longer needs to be added to
  the include path, and building the programs no longer requires that
  `_FILE_OFFSET_BITS` and `_POSIX_C_SOURCE` be defined on the command line.

## Version 1.16

* Improved the compression ratio at levels 10-12 slightly, mainly levels 11-12.
  Some inputs (such as certain PNG files) see much improved compression ratios.
  As a trade-off, compressing at levels 11-12 is now about 5-20% slower.

* For consistency with zlib, the decompressor now returns an error on some
  invalid inputs that were accepted before.

* Fixed a build error on arm64 with gcc with certain target CPUs.  (Fixes v1.12)

* Fixed a build error on arm32 with gcc 10.1-10.3 and 11.1-11.2.  (Fixes v1.15)

* Fixed a build error on arm32 with gcc in soft float mode.  (Fixes v1.15)

* Fixed a build error in programs/gzip.c with uClibc.  (Fixes v1.15)

* Fixed the install target on Windows.  (Fixes v1.15)

## Version 1.15

* libdeflate now uses CMake instead of a plain Makefile.

* Improved MSVC support.  Enabled most architecture-specific code with MSVC,
  fixed building with clang in MSVC compatibility mode, and other improvements.

* When libdeflate is built with MinGW, the static library and import library are
  now named using the MinGW convention (`*.a` and `*.dll.a`) instead of the
  Visual Studio convention.  This affects the official Windows binaries.

## Version 1.14

Significantly improved decompression performance on all platforms.  Examples
include (measuring DEFLATE only):

| Platform                           | Speedup over v1.13 |
|------------------------------------|--------------------|
| x86_64 (Intel Comet Lake), gcc     | 1.287x             |
| x86_64 (Intel Comet Lake), clang   | 1.437x             |
| x86_64 (Intel Ice Lake), gcc       | 1.332x             |
| x86_64 (Intel Ice Lake), clang     | 1.296x             |
| x86_64 (Intel Sandy Bridge), gcc   | 1.162x             |
| x86_64 (Intel Sandy Bridge), clang | 1.092x             |
| x86_64 (AMD Zen 2), gcc            | 1.263x             |
| x86_64 (AMD Zen 2), clang          | 1.259x             |
| i386 (Intel Comet Lake), gcc       | 1.570x             |
| i386 (Intel Comet Lake), clang     | 1.344x             |
| arm64 (Apple M1), clang            | 1.306x             |
| arm64 (Cortex-A76), clang          | 1.355x             |
| arm64 (Cortex-A55), clang          | 1.190x             |
| arm32 (Cortex-A76), clang          | 1.665x             |
| arm32 (Cortex-A55), clang          | 1.283x             |

Thanks to Dougall Johnson (https://dougallj.wordpress.com/) for ideas for many
of the improvements.

## Version 1.13

* Changed the 32-bit Windows build of the library to use the default calling
  convention (cdecl) instead of stdcall, reverting a change from libdeflate 1.4.

* Fixed a couple macOS compatibility issues with the gzip program.

## Version 1.12

This release focuses on improving the performance of the CRC-32 and Adler-32
checksum algorithms on x86 and ARM (both 32-bit and 64-bit).

* Build updates:

  * Fixed building libdeflate on Apple platforms.

  * For Visual Studio builds, Visual Studio 2015 or later is now required.

* CRC-32 algorithm updates:

  * Improved CRC-32 performance on short inputs on x86 and ARM.

  * Improved CRC-32 performance on Apple Silicon Macs by using a 12-way pmull
    implementation.   Performance on large inputs on M1 is now about 67 GB/s,
    compared to 8 GB/s before, or 31 GB/s with the Apple-provided zlib.

  * Improved CRC-32 performance on some other ARM CPUs by reworking the code so
    that multiple crc32 instructions can be issued in parallel.

  * Improved CRC-32 performance on some x86 CPUs by increasing the stride length
    of the pclmul implementation.

* Adler-32 algorithm updates:

  * Improved Adler-32 performance on some x86 CPUs by optimizing the AVX-2
    implementation.  E.g., performance on Zen 1 improved from 19 to 30 GB/s, and
    on Ice Lake from 35 to 41 GB/s (if the AVX-512 implementation is excluded).

  * Removed the AVX-512 implementation of Adler-32 to avoid CPU frequency
    downclocking, and because the AVX-2 implementation was made faster.

  * Improved Adler-32 performance on some ARM CPUs by optimizing the NEON
    implementation.  E.g., Apple M1 improved from about 36 to 52 GB/s.

## Version 1.11

* Library updates:

  * Improved compression performance slightly.

  * Detect arm64 CPU features on Apple platforms, which should improve
    performance in some areas such as CRC-32 computation.

* Program updates:

  * The included `gzip` and `gunzip` programs now support the `-q` option.

  * The included `gunzip` program now passes through non-gzip data when both
    the `-f` and `-c` options are used.

* Build updates:

  * Avoided a build error on arm32 with certain gcc versions, by disabling
    building `crc32_arm()` as dynamically-dispatched code when needed.

  * Support building with the LLVM toolchain on Windows.

  * Disabled the use of the "stdcall" ABI in static library builds on Windows.

  * Use the correct `install_name` in macOS builds.

  * Support Haiku builds.

## Version 1.10

* Added an additional check to the decompressor to make it quickly detect
  certain bad inputs and not try to generate an unbounded amount of output.

  Note: this was only a problem when decompressing with an unknown output size,
  which isn't the recommended use case of libdeflate.  However,
  `libdeflate-gunzip` has to do this, and it would run out of memory as it would
  keep trying to allocate a larger output buffer.

* Fixed a build error on Solaris.

* Cleaned up a few things in the compression code.

## Version 1.9

* Made many improvements to the compression algorithms, and rebalanced the
  compression levels:

  * Heuristics were implemented which significantly improve the compression
    ratio on data where short matches aren't useful, such as DNA sequencing
    data.  This applies to all compression levels, but primarily to levels 1-9.

  * Level 1 was made much faster, though it often compresses slightly worse than
    before (but still better than zlib).

  * Levels 8-9 were also made faster, though they often compress slightly worse
    than before (but still better than zlib).  On some data, levels 8-9 are much
    faster and compress much better than before; this change addressed an issue
    where levels 8-9 did poorly on certain files.  The algorithm used by levels
    8-9 is now more similar to that of levels 6-7 than to that of levels 10-12.

  * Levels 2-3, 7, and 10-12 were strengthened slightly.

  * Levels 4-6 were also strengthened slightly, but some of this improvement was
    traded off to speed them up slightly as well.

  * Levels 1-9 had their per-compressor memory usage greatly reduced.

  As always, compression ratios will vary depending on the input data, and
  compression speeds will vary depending on the input data and target platform.

* `make install` will now install a pkg-config file for libdeflate.

* The Makefile now supports the `DISABLE_SHARED` parameter to disable building
  the shared library.

* Improved the Android build support in the Makefile.

## Version 1.8

* Added `-t` (test) option to `libdeflate-gunzip`.

* Unaligned access optimizations are now enabled on WebAssembly builds.

* Fixed a build error when building with the Intel C Compiler (ICC).

* Fixed a build error when building with uClibc.

* libdeflate's CI system has switched from Travis CI to GitHub Actions.

* Made some improvements to test scripts.

## Version 1.7

* Added support for compression level 0, "no compression".

* Added an ARM CRC32 instruction accelerated implementation of CRC32.

* Added support for linking the programs to the shared library version of
  libdeflate rather than to the static library version.

* Made the compression level affect the minimum input size at which compression
  is attempted.

* Fixed undefined behavior in x86 Adler32 implementation.  (No miscompilations
  were observed in practice.)

* Fixed undefined behavior in x86 CPU feature code.  (No miscompilations were
  observed in practice.)

* Fixed installing shared lib symlink on macOS.

* Documented third-party bindings.

* Made a lot of improvements to the testing scripts and the CI configuration
  file.

* Lots of other small improvements and cleanups.

## Version 1.6

* Prevented gcc 10 from miscompiling libdeflate (workaround for
  https://gcc.gnu.org/bugzilla/show_bug.cgi?id=94994).

* Removed workaround for gcc 5 and earlier producing slow code on ARM32.  If
  this affects you, please upgrade your compiler.

* New API function: `libdeflate_zlib_decompress_ex()`.  It provides the actual
  size of the stream that was decompressed, like the gzip and DEFLATE
  equivalents.

* `libdeflate_zlib_decompress()` now accepts trailing bytes after the end of the
  stream, like the gzip and DEFLATE equivalents.

* Added support for custom memory allocators.  (New API function:
  `libdeflate_set_memory_allocator()`)

* Added support for building the library in freestanding mode.

* Building libdeflate no longer requires `CPPFLAGS=-Icommon`.

## Version 1.5

* Fixed up stdcall support on 32-bit Windows: the functions are now exported
  using both suffixed and non-suffixed names, and fixed `libdeflate.h` to be
  MSVC-compatible again.

## Version 1.4

* The 32-bit Windows build of libdeflate now uses the "stdcall" calling
  convention instead of "cdecl".  If you're calling `libdeflate.dll` directly
  from C or C++, you'll need to recompile your code.  If you're calling it from
  another language, or calling it indirectly using `LoadLibrary()`, you'll need
  to update your code to use the stdcall calling convention.

* The Makefile now supports building libdeflate as a shared
  library (`.dylib`) on macOS.

* Fixed a bug where support for certain optimizations and optional features
  (file access hints and more precise timestamps) was incorrectly omitted when
  libdeflate was compiled with `-Werror`.

* Added `make check` target to the Makefile.

* Added CI configuration files.

## Version 1.3

* `make install` now supports customizing the directories into which binaries,
  headers, and libraries are installed.

* `make install` now installs into `/usr/local` by default.  To change it, use
  e.g. `make install PREFIX=/usr`.

* `make install` now works on more platforms.

* The Makefile now supports overriding the optimization flags.

* The compression functions now correctly handle an output data buffer >= 4 GiB
  in size, and `gzip` and `gunzip` now correctly handle multi-gigabyte files (if
  enough memory is available).

## Version 1.2

* Slight improvements to decompression speed.

* Added an AVX-512BW implementation of Adler-32.

* The Makefile now supports a user-specified installation `PREFIX`.

* Fixed build error with some Visual Studio versions.

## Version 1.1

* Fixed crash in CRC-32 code when the prebuilt libdeflate for 32-bit Windows was
  called by a program built with Visual Studio.

* Improved the worst-case decompression speed of malicious data.

* Fixed build error when compiling for an ARM processor without hardware
  floating point support.

* Improved performance on the PowerPC64 architecture.

* Added soname to `libdeflate.so`, to make packaging easier.

* Added `make install` target to the Makefile.

* The Makefile now supports user-specified `CPPFLAGS`.

* The Windows binary releases now include the import library for
  `libdeflate.dll`.  `libdeflate.lib` is now the import library, and
  `libdeflatestatic.lib` is the static library.

## Version 1.0

* Added support for multi-member gzip files.

* Moved architecture-specific code into subdirectories.  If you aren't using the
  provided Makefile to build libdeflate, you now need to compile `lib/*.c` and
  `lib/*/*.c` instead of just `lib/*.c`.

* Added an ARM PMULL implementation of CRC-32, which speeds up gzip compression
  and decompression on 32-bit and 64-bit ARM processors that have the
  Cryptography Extensions.

* Improved detection of CPU features, resulting in accelerated functions being
  used in more cases.  This includes:

  * Detect CPU features on 32-bit x86, not just 64-bit as was done previously.

  * Detect CPU features on ARM, both 32 and 64-bit.  (Limited to Linux only
    currently.)

## Version 0.8

* Build fixes for certain platforms and compilers.

* libdeflate now produces the same output on all CPU architectures.

* Improved documentation for building libdeflate on Windows.

## Version 0.7

* Fixed a very rare bug that caused data to be compressed incorrectly.  The bug
  affected compression levels 7 and below since libdeflate v0.2.  Although there
  have been no user reports of the bug, and I believe it would have been highly
  unlikely to encounter on realistic data, it could occur on data specially
  crafted to reproduce it.

* Fixed a compilation error when building with clang 3.7.

## Version 0.6

* Various improvements to the gzip program's behavior.

* Faster CRC-32 on AVX-capable processors.

* Other minor changes.

## Version 0.5

* The CRC-32 checksum algorithm has been optimized with carryless multiplication
  instructions for `x86_64` (PCLMUL).  This speeds up gzip compression and
  decompression.

* Build fixes for certain platforms and compilers.

* Added more test programs and scripts.

* libdeflate is now entirely MIT-licensed.

## Version 0.4

* The Adler-32 checksum algorithm has been optimized with vector instructions
  for `x86_64` (SSE2 and AVX2) and ARM (NEON).  This speeds up zlib compression
  and decompression.

* To avoid naming collisions, functions and definitions in libdeflate's API have
  been renamed to be prefixed with `libdeflate_` or `LIBDEFLATE_`.  Programs
  using the old API will need to be updated.

* Various bug fixes and other improvements.

## Version 0.3

* Some bug fixes and other minor changes.

## Version 0.2

* Implemented a new block splitting algorithm which typically improves the
  compression ratio slightly at all compression levels.

* The compressor now outputs each block using the cheapest type (dynamic
  Huffman, static Huffman, or uncompressed).

* The gzip program has received an overhaul and now behaves more like the
  standard version.

* Build system updates, including: some build options were changed and some
  build options were removed, and the default 'make' target now includes the
  gzip program as well as the library.

## Version 0.1

* Initial official release.
