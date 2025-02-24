typ (Test Your Program)
=======================
typ is a simple program for testing command line executables and Python code.

When testing Python code, it is basically a wrapper around the standard
unittest module, but it provides the following bits of additional
functionality:

* Parallel test execution.
* Clean output in the style of the Ninja build tool.
* A more flexible mechanism for discovering tests from the
  command line and controlling how they are run:

  * Support for importing tests by directory, filename, or module.
  * Support for specifying tests to skip, tests to run in parallel,
    and tests that need to be run by themselves

* Support for producing traces of test times compatible with Chrome's
  tracing infrastructure (trace_viewer).
* Integrated test coverage reporting (including parallel coverage).
* Integrated support for debugging tests.
* Support for uploading test results automatically to a server
  (useful for continuous integration monitoring of test results).
* An abstraction of operating system functionality called the
  Host class. This can be used by other python code to write more
  portable and easily testable code by wrapping the multiprocessing,
  os, subprocess, and time modules.
* Simple libraries for integrating Ninja-style statistics and line
  printing into your own code (the Stats and Printer classes).
* Support for processing arbitrary arguments from calling code to
  test cases.
* Support for once-per-process setup and teardown hooks.

(These last two bullet points allow one to write tests that do not require
Python globals).

History
-------

typ originated out of work on the Blink and Chromium projects, as a way to
provide a friendlier interface to the Python unittest modules.

Work remaining
--------------

typ is still a work in progress, but it's getting close to being done.
Things remaining for 1.0, roughly in priority order:

- Implement a non-python file format for testing command line interfaces
- Write documentation

Possible future work
--------------------

- MainTestCase.check() improvements:

  - check all arguments and show all errors at once?
  - make multi-line regexp matches easier to follow?

- --debugger improvements:

  - make it skip the initial breakpoint?

- Support testing javascript, java, c++/gtest-style binaries?
- Support for test sharding in addition to parallel execution (so that
  run-webkit-tests can re-use as much of the code as possible)?
- Support for non-unittest runtest invocation (for run-webkit-tests,
  other harnesses?)
