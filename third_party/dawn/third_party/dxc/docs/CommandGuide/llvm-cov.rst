llvm-cov - emit coverage information
====================================

SYNOPSIS
--------

:program:`llvm-cov` *command* [*args...*]

DESCRIPTION
-----------

The :program:`llvm-cov` tool shows code coverage information for
programs that are instrumented to emit profile data. It can be used to
work with ``gcov``\-style coverage or with ``clang``\'s instrumentation
based profiling.

If the program is invoked with a base name of ``gcov``, it will behave as if
the :program:`llvm-cov gcov` command were called. Otherwise, a command should
be provided.

COMMANDS
--------

* :ref:`gcov <llvm-cov-gcov>`
* :ref:`show <llvm-cov-show>`
* :ref:`report <llvm-cov-report>`

.. program:: llvm-cov gcov

.. _llvm-cov-gcov:

GCOV COMMAND
------------

SYNOPSIS
^^^^^^^^

:program:`llvm-cov gcov` [*options*] *SOURCEFILE*

DESCRIPTION
^^^^^^^^^^^

The :program:`llvm-cov gcov` tool reads code coverage data files and displays
the coverage information for a specified source file. It is compatible with the
``gcov`` tool from version 4.2 of ``GCC`` and may also be compatible with some
later versions of ``gcov``.

To use :program:`llvm-cov gcov`, you must first build an instrumented version
of your application that collects coverage data as it runs. Compile with the
``-fprofile-arcs`` and ``-ftest-coverage`` options to add the
instrumentation. (Alternatively, you can use the ``--coverage`` option, which
includes both of those other options.) You should compile with debugging
information (``-g``) and without optimization (``-O0``); otherwise, the
coverage data cannot be accurately mapped back to the source code.

At the time you compile the instrumented code, a ``.gcno`` data file will be
generated for each object file. These ``.gcno`` files contain half of the
coverage data. The other half of the data comes from ``.gcda`` files that are
generated when you run the instrumented program, with a separate ``.gcda``
file for each object file. Each time you run the program, the execution counts
are summed into any existing ``.gcda`` files, so be sure to remove any old
files if you do not want their contents to be included.

By default, the ``.gcda`` files are written into the same directory as the
object files, but you can override that by setting the ``GCOV_PREFIX`` and
``GCOV_PREFIX_STRIP`` environment variables. The ``GCOV_PREFIX_STRIP``
variable specifies a number of directory components to be removed from the
start of the absolute path to the object file directory. After stripping those
directories, the prefix from the ``GCOV_PREFIX`` variable is added. These
environment variables allow you to run the instrumented program on a machine
where the original object file directories are not accessible, but you will
then need to copy the ``.gcda`` files back to the object file directories
where :program:`llvm-cov gcov` expects to find them.

Once you have generated the coverage data files, run :program:`llvm-cov gcov`
for each main source file where you want to examine the coverage results. This
should be run from the same directory where you previously ran the
compiler. The results for the specified source file are written to a file named
by appending a ``.gcov`` suffix. A separate output file is also created for
each file included by the main source file, also with a ``.gcov`` suffix added.

The basic content of an ``.gcov`` output file is a copy of the source file with
an execution count and line number prepended to every line. The execution
count is shown as ``-`` if a line does not contain any executable code. If
a line contains code but that code was never executed, the count is displayed
as ``#####``.

OPTIONS
^^^^^^^

.. option:: -a, --all-blocks

 Display all basic blocks. If there are multiple blocks for a single line of
 source code, this option causes llvm-cov to show the count for each block
 instead of just one count for the entire line.

.. option:: -b, --branch-probabilities

 Display conditional branch probabilities and a summary of branch information.

.. option:: -c, --branch-counts

 Display branch counts instead of probabilities (requires -b).

.. option:: -f, --function-summaries

 Show a summary of coverage for each function instead of just one summary for
 an entire source file.

.. option:: --help

 Display available options (--help-hidden for more).

.. option:: -l, --long-file-names

 For coverage output of files included from the main source file, add the
 main file name followed by ``##`` as a prefix to the output file names. This
 can be combined with the --preserve-paths option to use complete paths for
 both the main file and the included file.

.. option:: -n, --no-output

 Do not output any ``.gcov`` files. Summary information is still
 displayed.

.. option:: -o=<DIR|FILE>, --object-directory=<DIR>, --object-file=<FILE>

 Find objects in DIR or based on FILE's path. If you specify a particular
 object file, the coverage data files are expected to have the same base name
 with ``.gcno`` and ``.gcda`` extensions. If you specify a directory, the
 files are expected in that directory with the same base name as the source
 file.

.. option:: -p, --preserve-paths

 Preserve path components when naming the coverage output files. In addition
 to the source file name, include the directories from the path to that
 file. The directories are separate by ``#`` characters, with ``.`` directories
 removed and ``..`` directories replaced by ``^`` characters. When used with
 the --long-file-names option, this applies to both the main file name and the
 included file name.

.. option:: -u, --unconditional-branches

 Include unconditional branches in the output for the --branch-probabilities
 option.

.. option:: -version

 Display the version of llvm-cov.

EXIT STATUS
^^^^^^^^^^^

:program:`llvm-cov gcov` returns 1 if it cannot read input files.  Otherwise,
it exits with zero.


.. program:: llvm-cov show

.. _llvm-cov-show:

SHOW COMMAND
------------

SYNOPSIS
^^^^^^^^

:program:`llvm-cov show` [*options*] -instr-profile *PROFILE* *BIN* [*SOURCES*]

DESCRIPTION
^^^^^^^^^^^

The :program:`llvm-cov show` command shows line by line coverage of a binary
*BIN* using the profile data *PROFILE*. It can optionally be filtered to only
show the coverage for the files listed in *SOURCES*.

To use :program:`llvm-cov show`, you need a program that is compiled with
instrumentation to emit profile and coverage data. To build such a program with
``clang`` use the ``-fprofile-instr-generate`` and ``-fcoverage-mapping``
flags. If linking with the ``clang`` driver, pass ``-fprofile-instr-generate``
to the link stage to make sure the necessary runtime libraries are linked in.

The coverage information is stored in the built executable or library itself,
and this is what you should pass to :program:`llvm-cov show` as the *BIN*
argument. The profile data is generated by running this instrumented program
normally. When the program exits it will write out a raw profile file,
typically called ``default.profraw``, which can be converted to a format that
is suitable for the *PROFILE* argument using the :program:`llvm-profdata merge`
tool.

OPTIONS
^^^^^^^

.. option:: -show-line-counts

 Show the execution counts for each line. This is enabled by default, unless
 another ``-show`` option is used.

.. option:: -show-expansions

 Expand inclusions, such as preprocessor macros or textual inclusions, inline
 in the display of the source file.

.. option:: -show-instantiations

 For source regions that are instantiated multiple times, such as templates in
 ``C++``, show each instantiation separately as well as the combined summary.

.. option:: -show-regions

 Show the execution counts for each region by displaying a caret that points to
 the character where the region starts.

.. option:: -show-line-counts-or-regions

 Show the execution counts for each line if there is only one region on the
 line, but show the individual regions if there are multiple on the line.

.. option:: -use-color[=VALUE]

 Enable or disable color output. By default this is autodetected.

.. option:: -arch=<name>

 If the covered binary is a universal binary, select the architecture to use.
 It is an error to specify an architecture that is not included in the
 universal binary or to use an architecture that does not match a
 non-universal binary.

.. option:: -name=<NAME>

 Show code coverage only for functions with the given name.

.. option:: -name-regex=<PATTERN>

 Show code coverage only for functions that match the given regular expression.

.. option:: -line-coverage-gt=<N>

 Show code coverage only for functions with line coverage greater than the
 given threshold.

.. option:: -line-coverage-lt=<N>

 Show code coverage only for functions with line coverage less than the given
 threshold.

.. option:: -region-coverage-gt=<N>

 Show code coverage only for functions with region coverage greater than the
 given threshold.

.. option:: -region-coverage-lt=<N>

 Show code coverage only for functions with region coverage less than the given
 threshold.

.. program:: llvm-cov report

.. _llvm-cov-report:

REPORT COMMAND
--------------

SYNOPSIS
^^^^^^^^

:program:`llvm-cov report` [*options*] -instr-profile *PROFILE* *BIN* [*SOURCES*]

DESCRIPTION
^^^^^^^^^^^

The :program:`llvm-cov report` command displays a summary of the coverage of a
binary *BIN* using the profile data *PROFILE*. It can optionally be filtered to
only show the coverage for the files listed in *SOURCES*.

If no source files are provided, a summary line is printed for each file in the
coverage data. If any files are provided, summaries are shown for each function
in the listed files instead.

For information on compiling programs for coverage and generating profile data,
see :ref:`llvm-cov-show`.

OPTIONS
^^^^^^^

.. option:: -use-color[=VALUE]

 Enable or disable color output. By default this is autodetected.

.. option:: -arch=<name>

 If the covered binary is a universal binary, select the architecture to use.
 It is an error to specify an architecture that is not included in the
 universal binary or to use an architecture that does not match a
 non-universal binary.
