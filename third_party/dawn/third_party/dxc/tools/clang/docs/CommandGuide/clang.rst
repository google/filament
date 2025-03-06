clang - the Clang C, C++, and Objective-C compiler
==================================================

NOTE: this document applies to the original Clang project, not the DirectX
Compiler. It's made available for informational purposes only. The primary
replacement for the clang program is dxc.

SYNOPSIS
--------

:program:`clang` [*options*] *filename ...*

DESCRIPTION
-----------

:program:`clang` is a C, C++, and Objective-C compiler which encompasses
preprocessing, parsing, optimization, code generation, assembly, and linking.
Depending on which high-level mode setting is passed, Clang will stop before
doing a full link.  While Clang is highly integrated, it is important to
understand the stages of compilation, to understand how to invoke it.  These
stages are:

Driver
    The clang executable is actually a small driver which controls the overall
    execution of other tools such as the compiler, assembler and linker.
    Typically you do not need to interact with the driver, but you
    transparently use it to run the other tools.

Preprocessing
    This stage handles tokenization of the input source file, macro expansion,
    #include expansion and handling of other preprocessor directives.  The
    output of this stage is typically called a ".i" (for C), ".ii" (for C++),
    ".mi" (for Objective-C), or ".mii" (for Objective-C++) file.

Parsing and Semantic Analysis
    This stage parses the input file, translating preprocessor tokens into a
    parse tree.  Once in the form of a parse tree, it applies semantic
    analysis to compute types for expressions as well and determine whether
    the code is well formed. This stage is responsible for generating most of
    the compiler warnings as well as parse errors. The output of this stage is
    an "Abstract Syntax Tree" (AST).

Code Generation and Optimization
    This stage translates an AST into low-level intermediate code (known as
    "LLVM IR") and ultimately to machine code.  This phase is responsible for
    optimizing the generated code and handling target-specific code generation.
    The output of this stage is typically called a ".s" file or "assembly" file.

    Clang also supports the use of an integrated assembler, in which the code
    generator produces object files directly. This avoids the overhead of
    generating the ".s" file and of calling the target assembler.

Assembler
    This stage runs the target assembler to translate the output of the
    compiler into a target object file. The output of this stage is typically
    called a ".o" file or "object" file.

Linker
    This stage runs the target linker to merge multiple object files into an
    executable or dynamic library. The output of this stage is typically called
    an "a.out", ".dylib" or ".so" file.

:program:`Clang Static Analyzer`

The Clang Static Analyzer is a tool that scans source code to try to find bugs
through code analysis.  This tool uses many parts of Clang and is built into
the same driver.  Please see <http://clang-analyzer.llvm.org> for more details
on how to use the static analyzer.

OPTIONS
-------

Stage Selection Options
~~~~~~~~~~~~~~~~~~~~~~~

.. option:: -E

 Run the preprocessor stage.

.. option:: -fsyntax-only

 Run the preprocessor, parser and type checking stages.

.. option:: -S

 Run the previous stages as well as LLVM generation and optimization stages
 and target-specific code generation, producing an assembly file.

.. option:: -c

 Run all of the above, plus the assembler, generating a target ".o" object file.

.. option:: no stage selection option

 If no stage selection option is specified, all stages above are run, and the
 linker is run to combine the results into an executable or shared library.

Language Selection and Mode Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. option:: -x <language>

 Treat subsequent input files as having type language.

.. option:: -std=<language>

 Specify the language standard to compile for.

.. option:: -stdlib=<library>

 Specify the C++ standard library to use; supported options are libstdc++ and
 libc++.

.. option:: -ansi

 Same as -std=c89.

.. option:: -ObjC, -ObjC++

 Treat source input files as Objective-C and Object-C++ inputs respectively.

.. option:: -trigraphs

 Enable trigraphs.

.. option:: -ffreestanding

 Indicate that the file should be compiled for a freestanding, not a hosted,
 environment.

.. option:: -fno-builtin

 Disable special handling and optimizations of builtin functions like
 :c:func:`strlen` and :c:func:`malloc`.

.. option:: -fmath-errno

 Indicate that math functions should be treated as updating :c:data:`errno`.

.. option:: -fpascal-strings

 Enable support for Pascal-style strings with "\\pfoo".

.. option:: -fms-extensions

 Enable support for Microsoft extensions.

.. option:: -fmsc-version=

 Set _MSC_VER. Defaults to 1300 on Windows. Not set otherwise.

.. option:: -fborland-extensions

 Enable support for Borland extensions.

.. option:: -fwritable-strings

 Make all string literals default to writable.  This disables uniquing of
 strings and other optimizations.

.. option:: -flax-vector-conversions

 Allow loose type checking rules for implicit vector conversions.

.. option:: -fblocks

 Enable the "Blocks" language feature.

.. option:: -fobjc-gc-only

 Indicate that Objective-C code should be compiled in GC-only mode, which only
 works when Objective-C Garbage Collection is enabled.

.. option:: -fobjc-gc

 Indicate that Objective-C code should be compiled in hybrid-GC mode, which
 works with both GC and non-GC mode.

.. option:: -fobjc-abi-version=version

 Select the Objective-C ABI version to use. Available versions are 1 (legacy
 "fragile" ABI), 2 (non-fragile ABI 1), and 3 (non-fragile ABI 2).

.. option:: -fobjc-nonfragile-abi-version=<version>

 Select the Objective-C non-fragile ABI version to use by default. This will
 only be used as the Objective-C ABI when the non-fragile ABI is enabled
 (either via :option:`-fobjc-nonfragile-abi`, or because it is the platform
 default).

.. option:: -fobjc-nonfragile-abi

 Enable use of the Objective-C non-fragile ABI. On platforms for which this is
 the default ABI, it can be disabled with :option:`-fno-objc-nonfragile-abi`.

Target Selection Options
~~~~~~~~~~~~~~~~~~~~~~~~

Clang fully supports cross compilation as an inherent part of its design.
Depending on how your version of Clang is configured, it may have support for a
number of cross compilers, or may only support a native target.

.. option:: -arch <architecture>

  Specify the architecture to build for.

.. option:: -mmacosx-version-min=<version>

  When building for Mac OS X, specify the minimum version supported by your
  application.

.. option:: -miphoneos-version-min

  When building for iPhone OS, specify the minimum version supported by your
  application.

.. option:: -march=<cpu>

  Specify that Clang should generate code for a specific processor family
  member and later.  For example, if you specify -march=i486, the compiler is
  allowed to generate instructions that are valid on i486 and later processors,
  but which may not exist on earlier ones.


Code Generation Options
~~~~~~~~~~~~~~~~~~~~~~~

.. option:: -O0, -O1, -O2, -O3, -Ofast, -Os, -Oz, -O, -O4

  Specify which optimization level to use:

    :option:`-O0` Means "no optimization": this level compiles the fastest and
    generates the most debuggable code.

    :option:`-O1` Somewhere between :option:`-O0` and :option:`-O2`.

    :option:`-O2` Moderate level of optimization which enables most
    optimizations.

    :option:`-O3` Like :option:`-O2`, except that it enables optimizations that
    take longer to perform or that may generate larger code (in an attempt to
    make the program run faster).

    :option:`-Ofast` Enables all the optimizations from :option:`-O3` along
    with other aggressive optimizations that may violate strict compliance with
    language standards.

    :option:`-Os` Like :option:`-O2` with extra optimizations to reduce code
    size.

    :option:`-Oz` Like :option:`-Os` (and thus :option:`-O2`), but reduces code
    size further.

    :option:`-O` Equivalent to :option:`-O2`.

    :option:`-O4` and higher

      Currently equivalent to :option:`-O3`

.. option:: -g

  Generate debug information.  Note that Clang debug information works best at -O0.

.. option:: -fstandalone-debug -fno-standalone-debug

  Clang supports a number of optimizations to reduce the size of debug
  information in the binary. They work based on the assumption that the
  debug type information can be spread out over multiple compilation units.
  For instance, Clang will not emit type definitions for types that are not
  needed by a module and could be replaced with a forward declaration.
  Further, Clang will only emit type info for a dynamic C++ class in the
  module that contains the vtable for the class.

  The :option:`-fstandalone-debug` option turns off these optimizations.
  This is useful when working with 3rd-party libraries that don't come with
  debug information.  This is the default on Darwin.  Note that Clang will
  never emit type information for types that are not referenced at all by the
  program.

.. option:: -fexceptions

  Enable generation of unwind information. This allows exceptions to be thrown
  through Clang compiled stack frames.  This is on by default in x86-64.

.. option:: -ftrapv

  Generate code to catch integer overflow errors.  Signed integer overflow is
  undefined in C. With this flag, extra code is generated to detect this and
  abort when it happens.

.. option:: -fvisibility

  This flag sets the default visibility level.

.. option:: -fcommon

  This flag specifies that variables without initializers get common linkage.
  It can be disabled with :option:`-fno-common`.

.. option:: -ftls-model=<model>

  Set the default thread-local storage (TLS) model to use for thread-local
  variables. Valid values are: "global-dynamic", "local-dynamic",
  "initial-exec" and "local-exec". The default is "global-dynamic". The default
  model can be overridden with the tls_model attribute. The compiler will try
  to choose a more efficient model if possible.

.. option:: -flto, -emit-llvm

  Generate output files in LLVM formats, suitable for link time optimization.
  When used with :option:`-S` this generates LLVM intermediate language
  assembly files, otherwise this generates LLVM bitcode format object files
  (which may be passed to the linker depending on the stage selection options).

Driver Options
~~~~~~~~~~~~~~

.. option:: -###

  Print (but do not run) the commands to run for this compilation.

.. option:: --help

  Display available options.

.. option:: -Qunused-arguments

  Do not emit any warnings for unused driver arguments.

.. option:: -Wa,<args>

  Pass the comma separated arguments in args to the assembler.

.. option:: -Wl,<args>

  Pass the comma separated arguments in args to the linker.

.. option:: -Wp,<args>

  Pass the comma separated arguments in args to the preprocessor.

.. option:: -Xanalyzer <arg>

  Pass arg to the static analyzer.

.. option:: -Xassembler <arg>

  Pass arg to the assembler.

.. option:: -Xlinker <arg>

  Pass arg to the linker.

.. option:: -Xpreprocessor <arg>

  Pass arg to the preprocessor.

.. option:: -o <file>

  Write output to file.

.. option:: -print-file-name=<file>

  Print the full library path of file.

.. option:: -print-libgcc-file-name

  Print the library path for "libgcc.a".

.. option:: -print-prog-name=<name>

  Print the full program path of name.

.. option:: -print-search-dirs

  Print the paths used for finding libraries and programs.

.. option:: -save-temps

  Save intermediate compilation results.

.. option:: -integrated-as, -no-integrated-as

  Used to enable and disable, respectively, the use of the integrated
  assembler. Whether the integrated assembler is on by default is target
  dependent.

.. option:: -time

  Time individual commands.

.. option:: -ftime-report

  Print timing summary of each stage of compilation.

.. option:: -v

  Show commands to run and use verbose output.


Diagnostics Options
~~~~~~~~~~~~~~~~~~~

.. option:: -fshow-column, -fshow-source-location, -fcaret-diagnostics, -fdiagnostics-fixit-info, -fdiagnostics-parseable-fixits, -fdiagnostics-print-source-range-info, -fprint-source-range-info, -fdiagnostics-show-option, -fmessage-length

  These options control how Clang prints out information about diagnostics
  (errors and warnings). Please see the Clang User's Manual for more information.

Preprocessor Options
~~~~~~~~~~~~~~~~~~~~

.. option:: -D<macroname>=<value>

  Adds an implicit #define into the predefines buffer which is read before the
  source file is preprocessed.

.. option:: -U<macroname>

  Adds an implicit #undef into the predefines buffer which is read before the
  source file is preprocessed.

.. option:: -include <filename>

  Adds an implicit #include into the predefines buffer which is read before the
  source file is preprocessed.

.. option:: -I<directory>

  Add the specified directory to the search path for include files.

.. option:: -F<directory>

  Add the specified directory to the search path for framework include files.

.. option:: -nostdinc

  Do not search the standard system directories or compiler builtin directories
  for include files.

.. option:: -nostdlibinc

  Do not search the standard system directories for include files, but do
  search compiler builtin include directories.

.. option:: -nobuiltininc

  Do not search clang's builtin directory for include files.


ENVIRONMENT
-----------

.. envvar:: TMPDIR, TEMP, TMP

  These environment variables are checked, in order, for the location to write
  temporary files used during the compilation process.

.. envvar:: CPATH

  If this environment variable is present, it is treated as a delimited list of
  paths to be added to the default system include path list. The delimiter is
  the platform dependent delimiter, as used in the PATH environment variable.

  Empty components in the environment variable are ignored.

.. envvar:: C_INCLUDE_PATH, OBJC_INCLUDE_PATH, CPLUS_INCLUDE_PATH, OBJCPLUS_INCLUDE_PATH

  These environment variables specify additional paths, as for :envvar:`CPATH`, which are
  only used when processing the appropriate language.

.. envvar:: MACOSX_DEPLOYMENT_TARGET

  If :option:`-mmacosx-version-min` is unspecified, the default deployment
  target is read from this environment variable. This option only affects
  Darwin targets.

BUGS
----

To report bugs, please visit <http://llvm.org/bugs/>.  Most bug reports should
include preprocessed source files (use the :option:`-E` option) and the full
output of the compiler, along with information to reproduce.

SEE ALSO
--------

:manpage:`as(1)`, :manpage:`ld(1)`

