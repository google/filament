#!/usr/bin/python
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

from __future__ import print_function

import argparse
import sys
import textwrap

is_python3 = bool(sys.version_info.major == 3)


ALL_PRAGMAS = ['no cover', 'no win32', 'python2', 'python3', 'untested',
               'win32']
DEFAULT_PRAGMAS = ALL_PRAGMAS[:]

if is_python3:
    DEFAULT_PRAGMAS.remove('python3')
else:
    DEFAULT_PRAGMAS.remove('python2')

if sys.platform == 'win32':
    DEFAULT_PRAGMAS.remove('win32')
else:
    DEFAULT_PRAGMAS.remove('no win32')


def add_arguments(parser):
    parser.add_argument('--no-pragmas', action='store_true', default=False,
                        help='Show all uncovered lines (no pragmas).')
    parser.add_argument('--path', action='append', default=[],
                        help='Prepend given directories to sys.path.')
    parser.add_argument('--pragma', action='append', default=[],
                        help=('The coverage pragmas to honor '
                              '(defaults to %s).' % DEFAULT_PRAGMAS))
    parser.add_argument('--show', action='append', default=[],
                        help='Show code protected by the specified pragmas '
                             '(uses all pragmas *except* for the ones '
                             'specified).')
    parser.add_argument('--show-missing', action='store_true',
                        default=False, help='Show missing lines.')
    parser.add_argument('--source', action='append', default=[],
                        help='Limit coverage data to the given directories.')

    parser.formatter_class = argparse.RawTextHelpFormatter
    parser.epilog = textwrap.dedent("""
    Valid pragma values are:
        'no cover': The default coverage pragma, this now means we
                    truly cannot cover it.
        'no win32': Code that only executes when not on Windows.
        'python2':  Code that only executes under Python2.
        'python3':  Code that only executes under Python3.
        'untested': Code that does not yet have tests.
        'win32':    Code that only executes on Windows.

    In typ, we aim for 'no cover' to only apply to code that executes only
    when coverage is not available (and hence can never be counted). Most
    code, if annotated at all, should be 'untested', and we should strive
    for 'untested' to not be used, either.
    """)


def argv_from_args(args):
    argv = []
    if args.no_pragmas:
        argv.append('--no-pragmas')
    for arg in args.path:
        argv.extend(['--path', arg])
    for arg in args.show:
        argv.extend(['--show', arg])
    if args.show_missing:
        argv.append('--show-missing')
    for arg in args.source:
        argv.extend(['--source', arg])
    for arg in args.pragma:
        argv.extend(['--pragma', arg])
    return argv


def main(argv=None):
    parser = argparse.ArgumentParser()
    add_arguments(parser)
    args, remaining_args = parser.parse_known_args(argv)

    for path in args.path:
        if path not in sys.path:
            sys.path.append(path)

    try:
        import coverage
        from coverage.execfile import run_python_module, run_python_file
    except ImportError:
        print("Error: coverage is not available.")
        sys.exit(1)

    cov = coverage.coverage(source=args.source)
    cov.erase()
    cov.clear_exclude()

    if args.no_pragmas:
        args.pragma = []

    args.pragma = args.pragma or DEFAULT_PRAGMAS

    if args.show:
        args.show_missing = True
    for pragma in args.show:
        if pragma in args.pragma:
            args.pragma.remove(pragma)

    for pragma in args.pragma:
        cov.exclude('pragma: %s' % pragma)

    ret = 0
    cov.start()
    try:
        if remaining_args[0] == '-m':
            run_python_module(remaining_args[1], remaining_args[1:])
        else:
            run_python_file(remaining_args[0], remaining_args)
    except SystemExit as e:
        ret = e.code
    cov.stop()
    cov.save()
    cov.report(show_missing=args.show_missing)
    return ret


if __name__ == '__main__':
    sys.exit(main())
