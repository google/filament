#!/usr/bin/python3
# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# generate_parser.py:
#   Generate lexer and parser for ANGLE's shader preprocessor.

import sys

sys.path.append('..')
import generate_parser_tools

basename = 'preprocessor'


def main():
    return generate_parser_tools.generate_parser(basename, False)


if __name__ == '__main__':
    sys.exit(main())
