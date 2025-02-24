# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import distutils.version
import os
import sys
import textwrap
import unittest

ROOT_PATH = os.path.abspath(
    os.path.join(os.path.dirname(os.path.dirname(__file__))))


def native_error(msg, version):
    print(
        textwrap.dedent("""\
  ERROR: Native python-coverage (version: %s) is required to be
  installed on your PYTHONPATH to run this test. Recommendation:
     sudo apt-get install pip
     sudo pip install --upgrade coverage
  %s""") % (version, msg))
    sys.exit(1)


def covered_main(includes,
                 require_native=None,
                 required_percentage=100.0,
                 disable_coverage=True):
    """Equivalent of unittest.main(), except that it gathers coverage data, and
    asserts if the test is not at 100% coverage.

    Args:
        includes (list(str) or str) - List of paths to include in coverage
            report. May also be a single path instead of a list.
        require_native (str) - If non-None, will require that
            at least |require_native| version of coverage is installed on the
            system with CTracer.
        disable_coverage (bool) - If True, just run unittest.main() without any
            coverage tracking. Bug: crbug.com/662277
    """
    if disable_coverage:
        unittest.main()
        return

    try:
        import coverage
        if require_native is not None:
            got_ver = coverage.__version__
            if not getattr(coverage.collector, 'CTracer', None):
                native_error(
                    ("Native python-coverage module required.\n"
                     "Pure-python implementation (version: %s) found: %s") %
                    (got_ver, coverage), require_native)
            if got_ver < distutils.version.LooseVersion(require_native):
                native_error(
                    "Wrong version (%s) found: %s" % (got_ver, coverage),
                    require_native)
    except ImportError:
        if require_native is None:
            sys.path.insert(0, os.path.join(ROOT_PATH, 'third_party'))
            import coverage
        else:
            print("ERROR: python-coverage (%s) is required to be installed on "
                  "your PYTHONPATH to run this test." % require_native)
            sys.exit(1)

    COVERAGE = coverage.coverage(include=includes)
    COVERAGE.start()

    retcode = 0
    try:
        unittest.main()
    except SystemExit as e:
        retcode = e.code or retcode

    COVERAGE.stop()
    if COVERAGE.report() < required_percentage:
        print('FATAL: not at required %f%% coverage.' % required_percentage)
        retcode = 2

    return retcode
