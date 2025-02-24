Using AddressSanitizer in Subzero
=================================

AddressSanitizer is a powerful compile-time tool used to detect and report
illegal memory accesses. For a full description of the tool, see the original
`paper
<https://www.usenix.org/system/files/conference/atc12/atc12-final39.pdf>`_.
AddressSanitizer is only supported on native builds of .pexe files and cannot be
used in production.

In Subzero, AddressSanitizer depends on being able to find and instrument calls
to various functions such as malloc() and free(), and as such the .pexe file
being translated must not have had those symbols stripped or inlined. Subzero
will not complain if it is told to translate a .pexe file with its symbols
stripped, but it will not be able to find calls to malloc(), calloc(), free(),
etc., so AddressSanitizer will not work correctly in the final executable.

Furthermore, pnacl-clang automatically inlines some calls to calloc(),
even with inlining turned off, so we provide wrapper scripts,
sz-clang.py and sz-clang++.py, that normally just pass their arguments
through to pnacl-clang or pnacl-clang++, but add instrumentation to
replace calls to calloc() at the source level if they are passed
-fsanitize-address.

These are the steps to compile hello.c to an instrumented object file::

    sz-clang.py -fsanitize-address -o hello.nonfinal.pexe hello.c
    pnacl-finalize --no-strip-syms -o hello.pexe hello.nonfinal.pexe
    pnacl-sz -fsanitize-address -filetype=obj -o hello.o hello.pexe

The resulting object file must be linked with the Subzero-specific
AddressSanitizer runtime to work correctly. A .pexe file can be compiled with
AddressSanitizer and properly linked into a final executable using
subzero/pydir/szbuild.py with the --fsanitize-address flag, i.e.::

    pydir/szbuild.py --fsanitize-address hello.pexe

Handling Wide Loads
===================

Since AddressSanitizer is implemented only in Subzero, the target .pexe may
contain widened loads that would cause false positives. To avoid reporting such
loads as errors, we treat any word-aligned, four byte load as a potentially
widened load and only check the first byte of the loaded word against shadow
memory.

Building SPEC2000 Benchmark Suite
=================================

Most of the SPEC2000 benchmarks can be built with Subzero and AddressSanitizer,
however due to the nature of our solution for LLVM's aggressive inlining of
calloc, 300.twolf and 252.eon will not build. AddressSanitizer correctly finds
bugs in 197.parser and 253.perlbmk. 176.gcc crashes for unknown reasons. Among
the benchmarks that do run to completion, the average slowdown introduced is
4.6x.

To build the benchmarks with AddressSanitizer, some small changes to the
Makefile are needed. They can be found `here
<https://codereview.chromium.org/2266553002/>`_.

Once the Makefile has been patched, build and run with these commands::

  cd native_client/tests/spec2k
  ./run_all.sh BuildBenchmarks 0 SetupPnaclX8632Opt <benchmarks>
  ../../toolchain_build/src/subzero/pydir/szbuild_spec2k.py -v -O2 \
      --fsanitize-address <benchmarks>
  ./run_all.sh RunTimedBenchmarks SetupGccX8632Opt train <benchmarks>
