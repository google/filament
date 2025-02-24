# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Test Your Project

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

    * Integrated test coverage reporting.

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
      (These last two bullet points allow one to write tests that do not
       require Python globals).
"""

from typ.arg_parser import ArgumentParser
from typ.expectations_parser import TestExpectations
from typ.fakes.host_fake import FakeHost
from typ.host import Host
from typ.json_results import exit_code_from_full_results
from typ.json_results import make_full_results, make_upload_request
from typ.json_results import Result, ResultSet, ResultType
from typ.runner import Runner, TestInput, TestSet, WinMultiprocessing, main
from typ.stats import Stats
from typ.printer import Printer
from typ.test_case import convert_newlines, TestCase, MainTestCase
from typ.version import VERSION

__all__ = [
    'ArgumentParser',
    'Expectation',
    'FakeHost',
    'Host',
    'MainTestCase',
    'Printer',
    'Result',
    'ResultSet',
    'ResultType',
    'Runner',
    'Stats',
    'TestCase',
    'TestExpectationParser',
    'TestInput',
    'TestSet',
    'VERSION',
    'WinMultiprocessing',
    'convert_newlines',
    'exit_code_from_full_results',
    'main',
    'make_full_results',
    'make_upload_request',
]
