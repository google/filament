#!/usr/bin/env python
# -*- coding: ascii -*-
r"""
==================================
 Benchmark cssmin implementations
==================================

Benchmark cssmin implementations.

:Copyright:

 Copyright 2011 - 2014
 Andr\xe9 Malo or his licensors, as applicable

:License:

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

Usage::

    python -mbench.main [-c COUNT] [-p file] cssfile ...

    -c COUNT  number of runs per cssfile and minifier. Defaults to 10.
    -p file   File to write the benchmark results in (pickled)

"""
from __future__ import print_function
if __doc__:
    __doc__ = __doc__.encode('ascii').decode('unicode_escape')
__author__ = r"Andr\xe9 Malo".encode('ascii').decode('unicode_escape')
__docformat__ = "restructuredtext en"
__license__ = "Apache License, Version 2.0"
__version__ = "1.0.0"

import sys as _sys
import time as _time

import_notes = []
class _p_02__rcssmin(object):
    def __init__(self):
        import rcssmin
        cssmin = rcssmin._make_cssmin(python_only=True)
        self.cssmin = lambda x: cssmin(x, keep_bang_comments=True)

class _p_03__rcssmin(object):
    def __init__(self):
        import _rcssmin
        cssmin = _rcssmin.cssmin
        self.cssmin = lambda x: cssmin(x, keep_bang_comments=True)

class cssmins(object):
    from bench import cssmin as p_01_cssmin
    p_02_rcssmin = _p_02__rcssmin()
    try:
        p_03__rcssmin = _p_03__rcssmin()
    except ImportError:
        import_notes.append("_rcssmin (C-Port) not available")
        print(import_notes[-1])

print("Python Release: %s" % ".".join(map(str, _sys.version_info[:3])))
print("")


def slurp(filename):
    """ Load a file """
    fp = open(filename)
    try:
        return fp.read()
    finally:
        fp.close()


def print_(*value, **kwargs):
    """ Print stuff """
    (kwargs.get('file') or _sys.stdout).write(
        ''.join(value) + kwargs.get('end', '\n')
    )


def bench(filenames, count):
    """
    Benchmark the minifiers with given css samples

    :Parameters:
      `filenames` : sequence
        List of filenames

      `count` : ``int``
        Number of runs per css file and minifier

    :Exceptions:
      - `RuntimeError` : empty filenames sequence
    """
    if not filenames:
        raise RuntimeError("Missing files to benchmark")
    try:
        xrange
    except NameError:
        xrange = range
    try:
        cmp
    except NameError:
        cmp = lambda a, b: (a > b) - (a < b)

    ports = [item for item in dir(cssmins) if item.startswith('p_')]
    ports.sort()
    space = max(map(len, ports)) - 4
    ports = [(item[5:], getattr(cssmins, item).cssmin) for item in ports]
    flush = _sys.stdout.flush

    struct = []
    inputs = [(filename, slurp(filename)) for filename in filenames]
    for filename, style in inputs:
        print_("Benchmarking %r..." % filename, end=" ")
        flush()
        outputs = []
        for _, cssmin in ports:
            try:
                outputs.append(cssmin(style))
            except (SystemExit, KeyboardInterrupt):
                raise
            except:
                outputs.append(None)
        struct.append(dict(
            filename=filename,
            sizes=[
                (item is not None and len(item) or None) for item in outputs
            ],
            size=len(style),
            messages=[],
            times=[],
        ))
        print_("(%.1f KiB)" % (struct[-1]['size'] / 1024.0,))
        flush()
        times = []
        for idx, (name, cssmin) in enumerate(ports):
            if outputs[idx] is None:
                print_("  FAILED %s" % (name,))
                struct[-1]['times'].append((name, None))
            else:
                print_("  Timing %s%s... (%5.1f KiB %s)" % (
                    name,
                    " " * (space - len(name)),
                    len(outputs[idx]) / 1024.0,
                    idx == 0 and '*' or ['=', '>', '<'][
                        cmp(len(outputs[idx]), len(outputs[0]))
                    ],
                ), end=" ")
                flush()

                xcount = count
                while True:
                    counted = [None for _ in xrange(xcount)]
                    start = _time.time()
                    for _ in counted:
                        cssmin(style)
                    end = _time.time()
                    result = (end - start) * 1000
                    if result < 10: # avoid measuring within the error range
                        xcount *= 10
                        continue
                    times.append(result / xcount)
                    break

                print_("%8.2f ms" % times[-1], end=" ")
                flush()
                if len(times) <= 1:
                    print_()
                else:
                    print_("(factor: %s)" % (', '.join([
                        '%.2f' % (timed / times[-1]) for timed in times[:-1]
                    ])))
                struct[-1]['times'].append((name, times[-1]))

            flush()
        print_()

    return struct


def main(argv=None):
    """ Main """
    import getopt as _getopt
    import os as _os
    import pickle as _pickle

    if argv is None:
        argv = _sys.argv[1:]
    try:
        opts, args = _getopt.getopt(argv, "hc:p:", ["help"])
    except getopt.GetoptError:
        e = _sys.exc_info()[0](_sys.exc_info()[1])
        print(
            "%s\nTry %s -mbench.main --help" % (
                e,
                _os.path.basename(_sys.executable),
            ), file=_sys.stderr)
        _sys.exit(2)

    count, pickle = 10, None
    for key, value in opts:
        if key in ("-h", "--help"):
            print(
                "%s -mbench.main [-c count] [-p file] cssfile ..." % (
                    _os.path.basename(_sys.executable),
                ), file=_sys.stderr)
            _sys.exit(0)
        elif key == '-c':
            count = int(value)
        elif key == '-p':
            pickle = str(value)

    struct = bench(args, count)
    if pickle:
        fp = open(pickle, 'wb')
        try:
            fp.write(_pickle.dumps((
                ".".join(map(str, _sys.version_info[:3])),
                import_notes,
                struct,
            ), 0))
        finally:
            fp.close()


if __name__ == '__main__':
    main()
