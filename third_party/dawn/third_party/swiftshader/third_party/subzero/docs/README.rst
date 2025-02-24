Subzero - Fast code generator for PNaCl bitcode
===============================================

Design
------

See the accompanying DESIGN.rst file for a more detailed technical overview of
Subzero.

Building
--------

Subzero is set up to be built within the Native Client tree.  Follow the
`Developing PNaCl
<https://sites.google.com/a/chromium.org/dev/nativeclient/pnacl/developing-pnacl>`_
instructions, in particular the section on building PNaCl sources.  This will
prepare the necessary external headers and libraries that Subzero needs.
Checking out the Native Client project also gets the pre-built clang and LLVM
tools in ``native_client/../third_party/llvm-build/Release+Asserts/bin`` which
are used for building Subzero.

The Subzero source is in ``native_client/toolchain_build/src/subzero``.  From
within that directory, ``git checkout master && git pull`` to get the latest
version of Subzero source code.

The Makefile is designed to be used as part of the higher level LLVM build
system.  To build manually, use the ``Makefile.standalone``.  There are several
build configurations from the command line::

    make -f Makefile.standalone
    make -f Makefile.standalone DEBUG=1
    make -f Makefile.standalone NOASSERT=1
    make -f Makefile.standalone DEBUG=1 NOASSERT=1
    make -f Makefile.standalone MINIMAL=1
    make -f Makefile.standalone ASAN=1
    make -f Makefile.standalone TSAN=1

``DEBUG=1`` builds without optimizations and is good when running the translator
inside a debugger.  ``NOASSERT=1`` disables assertions and is the preferred
configuration for performance testing the translator.  ``MINIMAL=1`` attempts to
minimize the size of the translator by compiling out everything unnecessary.
``ASAN=1`` enables AddressSanitizer, and ``TSAN=1`` enables ThreadSanitizer.

The result of the ``make`` command is the target ``pnacl-sz`` in the current
directory.

Building within LLVM trunk
--------------------------

Subzero can also be built from within a standard LLVM trunk checkout.  Here is
an example of how it can be checked out and built::

    mkdir llvm-git
    cd llvm-git
    git clone http://llvm.org/git/llvm.git
    cd llvm/projects/
    git clone https://chromium.googlesource.com/native_client/pnacl-subzero
    cd ../..
    mkdir build
    cd build
    cmake -G Ninja ../llvm/
    ninja
    ./bin/pnacl-sz -version

This creates a default build of ``pnacl-sz``; currently any options such as
``DEBUG=1`` or ``MINIMAL=1`` have to be added manually.

``pnacl-sz``
------------

The ``pnacl-sz`` program parses a pexe or an LLVM bitcode file and translates it
into ICE (Subzero's intermediate representation).  It then invokes the ICE
translate method to lower it to target-specific machine code, optionally dumping
the intermediate representation at various stages of the translation.

The program can be run as follows::

    ../pnacl-sz ./path/to/<file>.pexe
    ../pnacl-sz ./tests_lit/pnacl-sz_tests/<file>.ll

At this time, ``pnacl-sz`` accepts a number of arguments, including the
following:

    ``-help`` -- Show available arguments and possible values.  (Note: this
    unfortunately also pulls in some LLVM-specific options that are reported but
    that Subzero doesn't use.)

    ``-notranslate`` -- Suppress the ICE translation phase, which is useful if
    ICE is missing some support.

    ``-target=<TARGET>`` -- Set the target architecture.  The default is x8632.
    Future targets include x8664, arm32, and arm64.

    ``-filetype=obj|asm|iasm`` -- Select the output file type.  ``obj`` is a
    native ELF file, ``asm`` is a textual assembly file, and ``iasm`` is a
    low-level textual assembly file demonstrating the integrated assembler.

    ``-O<LEVEL>`` -- Set the optimization level.  Valid levels are ``2``, ``1``,
    ``0``, ``-1``, and ``m1``.  Levels ``-1`` and ``m1`` are synonyms, and
    represent the minimum optimization and worst code quality, but fastest code
    generation.

    ``-verbose=<list>`` -- Set verbosity flags.  This argument allows a
    comma-separated list of values.  The default is ``none``, and the value
    ``inst,pred`` will roughly match the .ll bitcode file.  Of particular use
    are ``all``, ``most``, and ``none``.

    ``-o <FILE>`` -- Set the assembly output file name.  Default is stdout.

    ``-log <FILE>`` -- Set the file name for diagnostic output (whose level is
    controlled by ``-verbose``).  Default is stdout.

    ``-timing`` -- Dump some pass timing information after translating the input
    file.

Running the test suite
----------------------

Subzero uses the LLVM ``lit`` testing tool for part of its test suite, which
lives in ``tests_lit``. To execute the test suite, first build Subzero, and then
run::

    make -f Makefile.standalone check-lit

There is also a suite of cross tests in the ``crosstest`` directory.  A cross
test takes a test bitcode file implementing some unit tests, and translates it
twice, once with Subzero and once with LLVM's known-good ``llc`` translator.
The Subzero-translated symbols are specially mangled to avoid multiple
definition errors from the linker.  Both translated versions are linked together
with a driver program that calls each version of each unit test with a variety
of interesting inputs and compares the results for equality.  The cross tests
are currently invoked by running::

    make -f Makefile.standalone check-xtest

Similar, there is a suite of unit tests::

    make -f Makefile.standalone check-unit

A convenient way to run the lit, cross, and unit tests is::

    make -f Makefile.standalone check

Assembling ``pnacl-sz`` output as needed
----------------------------------------

``pnacl-sz`` can now produce a native ELF binary using ``-filetype=obj``.

``pnacl-sz`` can also produce textual assembly code in a structure suitable for
input to ``llvm-mc``, using ``-filetype=asm`` or ``-filetype=iasm``.  An object
file can then be produced using the command::

    llvm-mc -triple=i686 -filetype=obj -o=MyObj.o

Building a translated binary
----------------------------

There is a helper script, ``pydir/szbuild.py``, that translates a finalized pexe
into a fully linked executable.  Run it with ``-help`` for extensive
documentation.

By default, ``szbuild.py`` builds an executable using only Subzero translation,
but it can also be used to produce hybrid Subzero/``llc`` binaries (``llc`` is
the name of the LLVM translator) for bisection-based debugging.  In bisection
debugging mode, the pexe is translated using both Subzero and ``llc``, and the
resulting object files are combined into a single executable using symbol
weakening and other linker tricks to control which Subzero symbols and which
``llc`` symbols take precedence.  This is controlled by the ``-include`` and
``-exclude`` arguments.  These can be used to rapidly find a single function
that Subzero translates incorrectly leading to incorrect output.

There is another helper script, ``pydir/szbuild_spec2k.py``, that runs
``szbuild.py`` on one or more components of the Spec2K suite.  This assumes that
Spec2K is set up in the usual place in the Native Client tree, and the finalized
pexe files have been built.  (Note: for working with Spec2K and other pexes,
it's helpful to finalize the pexe using ``--no-strip-syms``, to preserve the
original function and global variable names.)

Status
------

Subzero currently fully supports the x86-32 architecture, for both native and
Native Client sandboxing modes.  The x86-64 architecture is also supported in
native mode only, and only for the x32 flavor due to the fact that pointers and
32-bit integers are indistinguishable in PNaCl bitcode.  Sandboxing support for
x86-64 is in progress.  ARM and MIPS support is in progress.  Two optimization
levels, ``-Om1`` and ``-O2``, are implemented.

The ``-Om1`` configuration is designed to be the simplest and fastest possible,
with a minimal set of passes and transformations.

* Simple Phi lowering before target lowering, by generating temporaries and
  adding assignments to the end of predecessor blocks.

* Simple register allocation limited to pre-colored or infinite-weight
  Variables.

The ``-O2`` configuration is designed to use all optimizations available and
produce the best code.

* Address mode inference to leverage the complex x86 addressing modes.

* Compare/branch fusing based on liveness/last-use analysis.

* Global, linear-scan register allocation.

* Advanced phi lowering after target lowering and global register allocation,
  via edge splitting, topological sorting of the parallel moves, and final local
  register allocation.

* Stack slot coalescing to reduce frame size.

* Branch optimization to reduce the number of branches to the following block.
