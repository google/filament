#!/usr/bin/env python3
# Copyright (c) 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Script used to test subprocess2."""

import optparse
import os
import sys
import time

if sys.platform == 'win32':
    # Annoying, make sure the output is not translated on Windows.
    # pylint: disable=no-member,import-error
    import msvcrt
    msvcrt.setmode(sys.stdout.fileno(), os.O_BINARY)
    msvcrt.setmode(sys.stderr.fileno(), os.O_BINARY)

parser = optparse.OptionParser()
parser.add_option('--fail',
                  dest='return_value',
                  action='store_const',
                  default=0,
                  const=64)
parser.add_option('--crlf',
                  action='store_const',
                  const='\r\n',
                  dest='eol',
                  default='\n')
parser.add_option('--cr', action='store_const', const='\r', dest='eol')
parser.add_option('--stdout', action='store_true')
parser.add_option('--stderr', action='store_true')
parser.add_option('--read', action='store_true')
options, args = parser.parse_args()
if args:
    parser.error('Internal error')


def do(string):
    if options.stdout:
        sys.stdout.buffer.write(string.upper().encode('utf-8'))
        sys.stdout.buffer.write(options.eol.encode('utf-8'))
    if options.stderr:
        sys.stderr.buffer.write(string.lower().encode('utf-8'))
        sys.stderr.buffer.write(options.eol.encode('utf-8'))
        sys.stderr.flush()


do('A')
do('BB')
do('CCC')
if options.read:
    assert options.return_value == 0
    try:
        while sys.stdin.read(1):
            options.return_value += 1
    except OSError:
        pass

sys.exit(options.return_value)
