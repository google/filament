========================================================
LibFuzzer -- a library for coverage-guided fuzz testing.
========================================================
.. contents::
   :local:
   :depth: 4

Introduction
============

This library is intended primarily for in-process coverage-guided fuzz testing
(fuzzing) of other libraries. The typical workflow looks like this:

* Build the Fuzzer library as a static archive (or just a set of .o files).
  Note that the Fuzzer contains the main() function.
  Preferably do *not* use sanitizers while building the Fuzzer.
* Build the library you are going to test with
  `-fsanitize-coverage={bb,edge}[,indirect-calls,8bit-counters]`
  and one of the sanitizers. We recommend to build the library in several
  different modes (e.g. asan, msan, lsan, ubsan, etc) and even using different
  optimizations options (e.g. -O0, -O1, -O2) to diversify testing.
* Build a test driver using the same options as the library.
  The test driver is a C/C++ file containing interesting calls to the library
  inside a single function  ``extern "C" void LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size);``
* Link the Fuzzer, the library and the driver together into an executable
  using the same sanitizer options as for the library.
* Collect the initial corpus of inputs for the
  fuzzer (a directory with test inputs, one file per input).
  The better your inputs are the faster you will find something interesting.
  Also try to keep your inputs small, otherwise the Fuzzer will run too slow.
  By default, the Fuzzer limits the size of every input to 64 bytes
  (use ``-max_len=N`` to override).
* Run the fuzzer with the test corpus. As new interesting test cases are
  discovered they will be added to the corpus. If a bug is discovered by
  the sanitizer (asan, etc) it will be reported as usual and the reproducer
  will be written to disk.
  Each Fuzzer process is single-threaded (unless the library starts its own
  threads). You can run the Fuzzer on the same corpus in multiple processes
  in parallel.


The Fuzzer is similar in concept to AFL_,
but uses in-process Fuzzing, which is more fragile, more restrictive, but
potentially much faster as it has no overhead for process start-up.
It uses LLVM's SanitizerCoverage_ instrumentation to get in-process
coverage-feedback

The code resides in the LLVM repository, requires the fresh Clang compiler to build
and is used to fuzz various parts of LLVM,
but the Fuzzer itself does not (and should not) depend on any
part of LLVM and can be used for other projects w/o requiring the rest of LLVM.

Flags
=====
The most important flags are::

  seed                               	0	Random seed. If 0, seed is generated.
  runs                               	-1	Number of individual test runs (-1 for infinite runs).
  max_len                            	64	Maximum length of the test input.
  cross_over                         	1	If 1, cross over inputs.
  mutate_depth                       	5	Apply this number of consecutive mutations to each input.
  timeout                            	1200	Timeout in seconds (if positive). If one unit runs more than this number of seconds the process will abort.
  help                               	0	Print help.
  save_minimized_corpus              	0	If 1, the minimized corpus is saved into the first input directory
  jobs                               	0	Number of jobs to run. If jobs >= 1 we spawn this number of jobs in separate worker processes with stdout/stderr redirected to fuzz-JOB.log.
  workers                            	0	Number of simultaneous worker processes to run the jobs. If zero, "min(jobs,NumberOfCpuCores()/2)" is used.
  tokens                             	0	Use the file with tokens (one token per line) to fuzz a token based input language.
  apply_tokens                       	0	Read the given input file, substitute bytes  with tokens and write the result to stdout.
  sync_command                       	0	Execute an external command "<sync_command> <test_corpus>" to synchronize the test corpus.
  sync_timeout                       	600	Minimum timeout between syncs.

For the full list of flags run the fuzzer binary with ``-help=1``.

Usage examples
==============

Toy example
-----------

A simple function that does something interesting if it receives the input "HI!"::

  cat << EOF >> test_fuzzer.cc
  extern "C" void LLVMFuzzerTestOneInput(const unsigned char *data, unsigned long size) {
    if (size > 0 && data[0] == 'H')
      if (size > 1 && data[1] == 'I')
         if (size > 2 && data[2] == '!')
         __builtin_trap();
  }
  EOF
  # Get lib/Fuzzer. Assuming that you already have fresh clang in PATH.
  svn co http://llvm.org/svn/llvm-project/llvm/trunk/lib/Fuzzer
  # Build lib/Fuzzer files.
  clang -c -g -O2 -std=c++11 Fuzzer/*.cpp -IFuzzer
  # Build test_fuzzer.cc with asan and link against lib/Fuzzer.
  clang++ -fsanitize=address -fsanitize-coverage=edge test_fuzzer.cc Fuzzer*.o
  # Run the fuzzer with no corpus.
  ./a.out

You should get ``Illegal instruction (core dumped)`` pretty quickly.

PCRE2
-----

Here we show how to use lib/Fuzzer on something real, yet simple: pcre2_::

  COV_FLAGS=" -fsanitize-coverage=edge,indirect-calls,8bit-counters"
  # Get PCRE2
  svn co svn://vcs.exim.org/pcre2/code/trunk pcre
  # Get lib/Fuzzer. Assuming that you already have fresh clang in PATH.
  svn co http://llvm.org/svn/llvm-project/llvm/trunk/lib/Fuzzer
  # Build PCRE2 with AddressSanitizer and coverage.
  (cd pcre; ./autogen.sh; CC="clang -fsanitize=address $COV_FLAGS" ./configure --prefix=`pwd`/../inst && make -j && make install)
  # Build lib/Fuzzer files.
  clang -c -g -O2 -std=c++11 Fuzzer/*.cpp -IFuzzer
  # Build the actual function that does something interesting with PCRE2.
  cat << EOF > pcre_fuzzer.cc
  #include <string.h>
  #include "pcre2posix.h"
  extern "C" void LLVMFuzzerTestOneInput(const unsigned char *data, size_t size) {
    if (size < 1) return;
    char *str = new char[size+1];
    memcpy(str, data, size);
    str[size] = 0;
    regex_t preg;
    if (0 == regcomp(&preg, str, 0)) {
      regexec(&preg, str, 0, 0, 0);
      regfree(&preg);
    }
    delete [] str;
  }
  EOF
  clang++ -g -fsanitize=address $COV_FLAGS -c -std=c++11  -I inst/include/ pcre_fuzzer.cc
  # Link.
  clang++ -g -fsanitize=address -Wl,--whole-archive inst/lib/*.a -Wl,-no-whole-archive Fuzzer*.o pcre_fuzzer.o -o pcre_fuzzer

This will give you a binary of the fuzzer, called ``pcre_fuzzer``.
Now, create a directory that will hold the test corpus::

  mkdir -p CORPUS

For simple input languages like regular expressions this is all you need.
For more complicated inputs populate the directory with some input samples.
Now run the fuzzer with the corpus dir as the only parameter::

  ./pcre_fuzzer ./CORPUS

You will see output like this::

  Seed: 1876794929
  #0      READ   cov 0 bits 0 units 1 exec/s 0
  #1      pulse  cov 3 bits 0 units 1 exec/s 0
  #1      INITED cov 3 bits 0 units 1 exec/s 0
  #2      pulse  cov 208 bits 0 units 1 exec/s 0
  #2      NEW    cov 208 bits 0 units 2 exec/s 0 L: 64
  #3      NEW    cov 217 bits 0 units 3 exec/s 0 L: 63
  #4      pulse  cov 217 bits 0 units 3 exec/s 0

* The ``Seed:`` line shows you the current random seed (you can change it with ``-seed=N`` flag).
* The ``READ``  line shows you how many input files were read (since you passed an empty dir there were inputs, but one dummy input was synthesised).
* The ``INITED`` line shows you that how many inputs will be fuzzed.
* The ``NEW`` lines appear with the fuzzer finds a new interesting input, which is saved to the CORPUS dir. If multiple corpus dirs are given, the first one is used.
* The ``pulse`` lines appear periodically to show the current status.

Now, interrupt the fuzzer and run it again the same way. You will see::

  Seed: 1879995378
  #0      READ   cov 0 bits 0 units 564 exec/s 0
  #1      pulse  cov 502 bits 0 units 564 exec/s 0
  ...
  #512    pulse  cov 2933 bits 0 units 564 exec/s 512
  #564    INITED cov 2991 bits 0 units 344 exec/s 564
  #1024   pulse  cov 2991 bits 0 units 344 exec/s 1024
  #1455   NEW    cov 2995 bits 0 units 345 exec/s 1455 L: 49

This time you were running the fuzzer with a non-empty input corpus (564 items).
As the first step, the fuzzer minimized the set to produce 344 interesting items (the ``INITED`` line)

It is quite convenient to store test corpuses in git.
As an example, here is a git repository with test inputs for the above PCRE2 fuzzer::

  git clone https://github.com/kcc/fuzzing-with-sanitizers.git
  ./pcre_fuzzer ./fuzzing-with-sanitizers/pcre2/C1/

You may run ``N`` independent fuzzer jobs in parallel on ``M`` CPUs::

  N=100; M=4; ./pcre_fuzzer ./CORPUS -jobs=$N -workers=$M

By default (``-reload=1``) the fuzzer processes will periodically scan the CORPUS directory
and reload any new tests. This way the test inputs found by one process will be picked up
by all others.

If ``-workers=$M`` is not supplied, ``min($N,NumberOfCpuCore/2)`` will be used.

Heartbleed
----------
Remember Heartbleed_?
As it was recently `shown <https://blog.hboeck.de/archives/868-How-Heartbleed-couldve-been-found.html>`_,
fuzzing with AddressSanitizer can find Heartbleed. Indeed, here are the step-by-step instructions
to find Heartbleed with LibFuzzer::

  wget https://www.openssl.org/source/openssl-1.0.1f.tar.gz
  tar xf openssl-1.0.1f.tar.gz
  COV_FLAGS="-fsanitize-coverage=edge,indirect-calls" # -fsanitize-coverage=8bit-counters
  (cd openssl-1.0.1f/ && ./config &&
    make -j 32 CC="clang -g -fsanitize=address $COV_FLAGS")
  # Get and build LibFuzzer
  svn co http://llvm.org/svn/llvm-project/llvm/trunk/lib/Fuzzer
  clang -c -g -O2 -std=c++11 Fuzzer/*.cpp -IFuzzer
  # Get examples of key/pem files.
  git clone   https://github.com/hannob/selftls
  cp selftls/server* . -v
  cat << EOF > handshake-fuzz.cc
  #include <openssl/ssl.h>
  #include <openssl/err.h>
  #include <assert.h>
  SSL_CTX *sctx;
  int Init() {
    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();
    assert (sctx = SSL_CTX_new(TLSv1_method()));
    assert (SSL_CTX_use_certificate_file(sctx, "server.pem", SSL_FILETYPE_PEM));
    assert (SSL_CTX_use_PrivateKey_file(sctx, "server.key", SSL_FILETYPE_PEM));
    return 0;
  }
  extern "C" void LLVMFuzzerTestOneInput(unsigned char *Data, size_t Size) {
    static int unused = Init();
    SSL *server = SSL_new(sctx);
    BIO *sinbio = BIO_new(BIO_s_mem());
    BIO *soutbio = BIO_new(BIO_s_mem());
    SSL_set_bio(server, sinbio, soutbio);
    SSL_set_accept_state(server);
    BIO_write(sinbio, Data, Size);
    SSL_do_handshake(server);
    SSL_free(server);
  }
  EOF
  # Build the fuzzer. 
  clang++ -g handshake-fuzz.cc  -fsanitize=address \
    openssl-1.0.1f/libssl.a openssl-1.0.1f/libcrypto.a Fuzzer*.o
  # Run 20 independent fuzzer jobs.
  ./a.out  -jobs=20 -workers=20

Voila::

  #1048576        pulse  cov 3424 bits 0 units 9 exec/s 24385
  =================================================================
  ==17488==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x629000004748 at pc 0x00000048c979 bp 0x7fffe3e864f0 sp 0x7fffe3e85ca8
  READ of size 60731 at 0x629000004748 thread T0
      #0 0x48c978 in __asan_memcpy
      #1 0x4db504 in tls1_process_heartbeat openssl-1.0.1f/ssl/t1_lib.c:2586:3
      #2 0x580be3 in ssl3_read_bytes openssl-1.0.1f/ssl/s3_pkt.c:1092:4

Advanced features
=================

Tokens
------

By default, the fuzzer is not aware of complexities of the input language
and when fuzzing e.g. a C++ parser it will mostly stress the lexer.
It is very hard for the fuzzer to come up with something like ``reinterpret_cast<int>``
from a test corpus that doesn't have it.
See a detailed discussion of this topic at
http://lcamtuf.blogspot.com/2015/01/afl-fuzz-making-up-grammar-with.html.

lib/Fuzzer implements a simple technique that allows to fuzz input languages with
long tokens. All you need is to prepare a text file containing up to 253 tokens, one token per line,
and pass it to the fuzzer as ``-tokens=TOKENS_FILE.txt``.
Three implicit tokens are added: ``" "``, ``"\t"``, and ``"\n"``.
The fuzzer itself will still be mutating a string of bytes
but before passing this input to the target library it will replace every byte ``b`` with the ``b``-th token.
If there are less than ``b`` tokens, a space will be added instead.

AFL compatibility
-----------------
LibFuzzer can be used in parallel with AFL_ on the same test corpus.
Both fuzzers expect the test corpus to reside in a directory, one file per input.
You can run both fuzzers on the same corpus in parallel::

  ./afl-fuzz -i testcase_dir -o findings_dir /path/to/program -r @@
  ./llvm-fuzz testcase_dir findings_dir  # Will write new tests to testcase_dir

Periodically restart both fuzzers so that they can use each other's findings.

How good is my fuzzer?
----------------------

Once you implement your target function ``LLVMFuzzerTestOneInput`` and fuzz it to death,
you will want to know whether the function or the corpus can be improved further.
One easy to use metric is, of course, code coverage.
You can get the coverage for your corpus like this::

  ASAN_OPTIONS=coverage_pcs=1 ./fuzzer CORPUS_DIR -runs=0

This will run all the tests in the CORPUS_DIR but will not generate any new tests
and dump covered PCs to disk before exiting.
Then you can subtract the set of covered PCs from the set of all instrumented PCs in the binary,
see SanitizerCoverage_ for details.

User-supplied mutators
----------------------

LibFuzzer allows to use custom (user-supplied) mutators,
see FuzzerInterface.h_

Fuzzing components of LLVM
==========================

clang-format-fuzzer
-------------------
The inputs are random pieces of C++-like text.

Build (make sure to use fresh clang as the host compiler)::

    cmake -GNinja  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DLLVM_USE_SANITIZER=Address -DLLVM_USE_SANITIZE_COVERAGE=YES -DCMAKE_BUILD_TYPE=Release /path/to/llvm
    ninja clang-format-fuzzer
    mkdir CORPUS_DIR
    ./bin/clang-format-fuzzer CORPUS_DIR

Optionally build other kinds of binaries (asan+Debug, msan, ubsan, etc).

TODO: commit the pre-fuzzed corpus to svn (?).

Tracking bug: https://llvm.org/bugs/show_bug.cgi?id=23052

clang-fuzzer
------------

The default behavior is very similar to ``clang-format-fuzzer``.
Clang can also be fuzzed with Tokens_ using ``-tokens=$LLVM/lib/Fuzzer/cxx_fuzzer_tokens.txt`` option.

Tracking bug: https://llvm.org/bugs/show_bug.cgi?id=23057

Buildbot
--------

We have a buildbot that runs the above fuzzers for LLVM components
24/7/365 at http://lab.llvm.org:8011/builders/sanitizer-x86_64-linux-fuzzer .

Pre-fuzzed test inputs in git
-----------------------------

The buildbot occumulates large test corpuses over time.
The corpuses are stored in git on github and can be used like this::

  git clone https://github.com/kcc/fuzzing-with-sanitizers.git
  bin/clang-format-fuzzer fuzzing-with-sanitizers/llvm/clang-format/C1
  bin/clang-fuzzer        fuzzing-with-sanitizers/llvm/clang/C1/
  bin/clang-fuzzer        fuzzing-with-sanitizers/llvm/clang/TOK1  -tokens=$LLVM/llvm/lib/Fuzzer/cxx_fuzzer_tokens.txt


FAQ
=========================

Q. Why Fuzzer does not use any of the LLVM support?
---------------------------------------------------

There are two reasons.

First, we want this library to be used outside of the LLVM w/o users having to
build the rest of LLVM. This may sound unconvincing for many LLVM folks,
but in practice the need for building the whole LLVM frightens many potential
users -- and we want more users to use this code.

Second, there is a subtle technical reason not to rely on the rest of LLVM, or
any other large body of code (maybe not even STL). When coverage instrumentation
is enabled, it will also instrument the LLVM support code which will blow up the
coverage set of the process (since the fuzzer is in-process). In other words, by
using more external dependencies we will slow down the fuzzer while the main
reason for it to exist is extreme speed.

Q. What about Windows then? The Fuzzer contains code that does not build on Windows.
------------------------------------------------------------------------------------

The sanitizer coverage support does not work on Windows either as of 01/2015.
Once it's there, we'll need to re-implement OS-specific parts (I/O, signals).

Q. When this Fuzzer is not a good solution for a problem?
---------------------------------------------------------

* If the test inputs are validated by the target library and the validator
  asserts/crashes on invalid inputs, the in-process fuzzer is not applicable
  (we could use fork() w/o exec, but it comes with extra overhead).
* Bugs in the target library may accumulate w/o being detected. E.g. a memory
  corruption that goes undetected at first and then leads to a crash while
  testing another input. This is why it is highly recommended to run this
  in-process fuzzer with all sanitizers to detect most bugs on the spot.
* It is harder to protect the in-process fuzzer from excessive memory
  consumption and infinite loops in the target library (still possible).
* The target library should not have significant global state that is not
  reset between the runs.
* Many interesting target libs are not designed in a way that supports
  the in-process fuzzer interface (e.g. require a file path instead of a
  byte array).
* If a single test run takes a considerable fraction of a second (or
  more) the speed benefit from the in-process fuzzer is negligible.
* If the target library runs persistent threads (that outlive
  execution of one test) the fuzzing results will be unreliable.

Q. So, what exactly this Fuzzer is good for?
--------------------------------------------

This Fuzzer might be a good choice for testing libraries that have relatively
small inputs, each input takes < 1ms to run, and the library code is not expected
to crash on invalid inputs.
Examples: regular expression matchers, text or binary format parsers.

.. _pcre2: http://www.pcre.org/

.. _AFL: http://lcamtuf.coredump.cx/afl/

.. _SanitizerCoverage: http://clang.llvm.org/docs/SanitizerCoverage.html

.. _Heartbleed: http://en.wikipedia.org/wiki/Heartbleed

.. _FuzzerInterface.h: https://github.com/llvm-mirror/llvm/blob/master/lib/Fuzzer/FuzzerInterface.h
