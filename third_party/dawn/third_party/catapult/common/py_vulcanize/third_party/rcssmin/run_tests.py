#!/usr/bin/env python
# -*- coding: ascii -*-
#
# Copyright 2014
# Andr\xe9 Malo or his licensors, as applicable
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""
===========
 Run tests
===========

Run tests.
"""
__author__ = "Andr\xe9 Malo"
__author__ = getattr(__author__, 'decode', lambda x: __author__)('latin-1')
__docformat__ = "restructuredtext en"

import os as _os
import re as _re
import sys as _sys

from _setup import shell
from _setup import term


def run_tests(basedir, libdir):
    """ Run output based tests """
    import rcssmin as _rcssmin
    py_cssmin = _rcssmin._make_cssmin(python_only=True)
    c_cssmin = _rcssmin._make_cssmin(python_only=False)

    def run_test(example, output_file):
        """ Run it """
        try:
            fp = open(example, 'r')
        except IOError:
            return
        else:
            try:
                input = fp.read()
            finally:
                fp.close()

        def load_output(filename):
            try:
                fp = open(filename, 'r')
            except IOError:
                return None
            else:
                try:
                    output = fp.read()
                finally:
                    fp.close()
            output = output.strip()
            if _re.search(r'(?<!\\)(?:\\\\)*\\[0-9a-zA-Z]{1,6}$', output):
                output += ' '
            return output

        output = load_output(output_file)
        output_b = load_output(output_file + '.b')

        def do_test(cssmin, output, **options):
            try:
                genout = cssmin(input, **options)
            except (KeyboardInterrupt, SystemExit):
                raise
            except:
                return 1, "%(RED)s exc%(NORMAL)s "
            else:
                if output is None:
                    return 1, "%(RED)smiss%(NORMAL)s "
                elif genout == output or genout == output.rstrip():
                    return 0, "%(GREEN)sOK%(NORMAL)s   "
                else:
                    return 1, "%(RED)sfail%(NORMAL)s "

        erred, out = do_test(py_cssmin, output)
        erred, c_out = do_test(c_cssmin, output)
        erred, out_b = do_test(py_cssmin, output_b, keep_bang_comments=True)
        erred, c_out_b = do_test(c_cssmin, output_b, keep_bang_comments=True)

        term.write(
            "%(out)s %(out_b)s  |  %(c_out)s %(c_out_b)s - %%(example)s\n"
                % locals(),
            example=_os.path.basename(example),
        )
        return erred

    # end
    # begin main test code

    erred = 0
    basedir = shell.native(basedir)
    strip = len(basedir) - len(_os.path.basename(basedir))
    for dirname, dirs, files in shell.walk(basedir):
        dirs[:] = [
            item for item in dirs if item not in ('.svn', '.git', 'out')
        ]
        dirs.sort()
        files = [item for item in files if item.endswith('.css')]
        if not files:
            continue
        if not _os.path.isdir(_os.path.join(basedir, dirname, 'out')):
            continue
        term.yellow("---> %s" % (dirname[strip:],))
        files.sort()
        for filename in files:
            if run_test(
                _os.path.join(dirname, filename),
                _os.path.join(dirname, 'out', filename[:-4] + '.out'),
            ): erred = 1
        term.yellow("<--- %s" % (dirname[strip:],))
    return erred


def main():
    """ Main """
    basedir, libdir = None, None
    accept_opts = True
    args = []
    for arg in _sys.argv[1:]:
        if accept_opts:
            if arg == '--':
                accept_opts = False
                continue
            elif arg == '-q':
                term.write = term.green = term.red = term.yellow = \
                    term.announce = \
                    lambda fmt, **kwargs: None
                continue
            elif arg == '-p':
                info = {}
                for key in term.terminfo():
                    info[key] = ''
                info['ERASE'] = '\n'
                term.terminfo.info = info
                continue
            elif arg.startswith('-'):
                _sys.stderr.write("Unrecognized option %r\n" % (arg,))
                return 2
        args.append(arg)
    if len(args) > 2:
        _sys.stderr.write("Too many arguments\n")
        return 2
    elif len(args) < 1:
        _sys.stderr.write("Missing arguments\n")
        return 2
    basedir = args[0]
    if len(args) > 1:
        libdir = args[1]
    return run_tests(basedir, libdir)


if __name__ == '__main__':
    _sys.exit(main())
