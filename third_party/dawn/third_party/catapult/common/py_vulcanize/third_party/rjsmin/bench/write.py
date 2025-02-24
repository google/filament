#!/usr/bin/env python
# -*- coding: ascii -*-
r"""
=========================
 Write benchmark results
=========================

Write benchmark results.

:Copyright:

 Copyright 2014 - 2015
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

    python -mbench.write [-p plain] [-t table] <pickled

    -p plain  Plain file to write to (like docs/BENCHMARKS).
    -t table  Table file to write to (like docs/_userdoc/benchmark.txt).

"""
from __future__ import print_function
if __doc__:
    __doc__ = __doc__.encode('ascii').decode('unicode_escape')
__author__ = r"Andr\xe9 Malo".encode('ascii').decode('unicode_escape')
__docformat__ = "restructuredtext en"
__license__ = "Apache License, Version 2.0"
__version__ = "1.0.0"

import os as _os
import re as _re
import sys as _sys


try:
    unicode
except NameError:
    def uni(v):
        if hasattr(v, 'decode'):
            return v.decode('latin-1')
        return str(v)
else:
    def uni(v):
        if isinstance(v, unicode):
            return v.encode('utf-8')
        return str(v)


def write_table(filename, results):
    """
    Output tabled benchmark results

    :Parameters:
      `filename` : ``str``
        Filename to write to

      `results` : ``list``
        Results
    """
    try:
        next
    except NameError:
        next = lambda i: (getattr(i, 'next', None) or i.__next__)()
    try:
        cmp
    except NameError:
        cmp = lambda a, b: (a > b) - (a < b)

    names = [
        ('simple_port', 'Simple Port'),
        ('jsmin_2_0_9', 'jsmin 2.0.9'),
        ('rjsmin', '|rjsmin|'),
        ('_rjsmin', r'_\ |rjsmin|'),
    ]
    benched_per_table = 2

    results = sorted(results, reverse=True)

    # First we transform our data into a table (list of lists)
    pythons, widths = [], [0] * (benched_per_table + 1)
    last_version = None
    for version, _, result in results:
        version = uni(version)
        if not(last_version is None or version.startswith('2.')):
            continue
        last_version = version

        namesub = _re.compile(r'(?:-\d+(?:\.\d+)*)?\.js$').sub
        result = iter(result)
        tables = []

        # given our data it's easier to create the table transposed...
        for benched in result:
            rows = [['Name'] + [desc for _, desc in names]]
            for _ in range(benched_per_table):
                if _:
                    try:
                        benched = next(result)
                    except StopIteration:
                        rows.append([''] + ['' for _ in names])
                        continue

                times = dict((
                    uni(port), (time, benched['sizes'][idx])
                ) for idx, (port, time) in enumerate(benched['times']))
                columns = ['%s (%.1f)' % (
                    namesub('', _os.path.basename(uni(benched['filename']))),
                    benched['size'] / 1024.0,
                )]
                for idx, (port, _) in enumerate(names):
                    if port not in times:
                        columns.append('n/a')
                        continue
                    time, size = times[port]
                    if time is None:
                        columns.append('(failed)')
                        continue
                    columns.append('%s%.2f ms (%.1f %s)' % (
                        idx == 0 and ' ' or '',
                        time,
                        size / 1024.0,
                        idx == 0 and '\\*' or ['=', '>', '<'][
                            cmp(size, benched['sizes'][0])
                        ],
                    ))
                rows.append(columns)

            # calculate column widths (global for all tables)
            for idx, row in enumerate(rows):
                widths[idx] = max(widths[idx], max(map(len, row)))

            # ... and transpose it back.
            tables.append(zip(*rows))
        pythons.append((version, tables))

        if last_version.startswith('2.'):
            break

    # Second we create a rest table from it
    lines = []
    separator = lambda c='-': '+'.join([''] + [
        c * (width + 2) for width in widths
    ] + [''])

    for idx, (version, tables) in enumerate(pythons):
        if idx:
            lines.append('')
            lines.append('')

        line = 'Python %s' % (version,)
        lines.append(line)
        lines.append('~' * len(line))

        for table in tables:
            lines.append('')
            lines.append('.. rst-class:: benchmark')
            lines.append('')

            for idx, row in enumerate(table):
                if idx == 0:
                    # header
                    lines.append(separator())
                    lines.append('|'.join([''] + [
                        ' %s%*s ' % (col, len(col) - width, '')
                        for width, col in zip(widths, row)
                    ] + ['']))
                    lines.append(separator('='))
                else: # data
                    lines.append('|'.join([''] + [
                        j == 0 and (
                            ' %s%*s ' % (col, len(col) - widths[j], '')
                        ) or (
                            ['%*s  ', ' %*s '][idx == 1] % (widths[j], col)
                        )
                        for j, col in enumerate(row)
                    ] + ['']))
                    lines.append(separator())

    fplines = []
    fp = open(filename)
    try:
        fpiter = iter(fp)
        for line in fpiter:
            line = line.rstrip()
            if line == '.. begin tables':
                buf = []
                for line in fpiter:
                    line = line.rstrip()
                    if line == '.. end tables':
                        fplines.append('.. begin tables')
                        fplines.append('')
                        fplines.extend(lines)
                        fplines.append('')
                        fplines.append('.. end tables')
                        buf = []
                        break
                    else:
                        buf.append(line)
                else:
                    fplines.extend(buf)
                    _sys.stderr.write("Placeholder container not found!\n")
            else:
                fplines.append(line)
    finally:
        fp.close()

    fp = open(filename, 'w')
    try:
        fp.write('\n'.join(fplines) + '\n')
    finally:
        fp.close()


def write_plain(filename, results):
    """
    Output plain benchmark results

    :Parameters:
      `filename` : ``str``
        Filename to write to

      `results` : ``list``
        Results
    """
    lines = []
    results = sorted(results, reverse=True)
    for idx, (version, import_notes, result) in enumerate(results):
        if idx:
            lines.append('')
            lines.append('')

        lines.append('$ python%s -OO bench/main.py bench/*.js' % (
            '.'.join(version.split('.')[:2])
        ))
        lines.append('~' * 72)
        for note in import_notes:
            lines.append(uni(note))
        lines.append('Python Release: %s' % (version,))

        for single in result:
            lines.append('')
            lines.append('Benchmarking %r... (%.1f KiB)' % (
                uni(single['filename']), single['size'] / 1024.0
            ))
            for msg in single['messages']:
                lines.append(msg)
            times = []
            space = max([len(uni(port)) for port, _ in single['times']])
            for idx, (port, time) in enumerate(single['times']):
                port = uni(port)
                if time is None:
                    lines.append("  FAILED %s" % (port,))
                else:
                    times.append(time)
                    lines.append(
                        "  Timing %s%s ... (%5.1f KiB %s) %8.2f ms" % (
                            port,
                            " " * (space - len(port)),
                            single['sizes'][idx] / 1024.0,
                            idx == 0 and '*' or ['=', '>', '<'][
                                cmp(single['sizes'][idx], single['sizes'][0])
                            ],
                            time
                        )
                    )
                    if len(times) > 1:
                        lines[-1] += " (factor: %s)" % (', '.join([
                            '%.2f' % (timed / time) for timed in times[:-1]
                        ]))

    lines.append('')
    lines.append('')
    lines.append('# vim: nowrap')
    fp = open(filename, 'w')
    try:
        fp.write('\n'.join(lines) + '\n')
    finally:
        fp.close()


def main(argv=None):
    """ Main """
    import getopt as _getopt
    import pickle as _pickle

    if argv is None:
        argv = _sys.argv[1:]
    try:
        opts, args = _getopt.getopt(argv, "hp:t:", ["help"])
    except getopt.GetoptError:
        e = _sys.exc_info()[0](_sys.exc_info()[1])
        print(
            "%s\nTry %s -mbench.write --help" % (
                e,
                _os.path.basename(_sys.executable),
            ), file=_sys.stderr)
        _sys.exit(2)

    plain, table = None, None
    for key, value in opts:
        if key in ("-h", "--help"):
            print(
                "%s -mbench.write [-p plain] [-t table] <pickled" % (
                    _os.path.basename(_sys.executable),
                ), file=_sys.stderr)
            _sys.exit(0)
        elif key == '-p':
            plain = str(value)
        elif key == '-t':
            table = str(value)

    struct = []
    _sys.stdin = getattr(_sys.stdin, 'detach', lambda: _sys.stdin)()
    try:
        while True:
            version, import_notes, result = _pickle.load(_sys.stdin)
            if hasattr(version, 'decode'):
                version = version.decode('latin-1')
            struct.append((version, import_notes, result))
    except EOFError:
        pass

    if plain:
        write_plain(plain, struct)

    if table:
        write_table(table, struct)


if __name__ == '__main__':
    main()
