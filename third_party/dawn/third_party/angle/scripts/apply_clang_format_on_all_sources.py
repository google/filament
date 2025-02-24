#!/usr/bin/env python

# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# apply_clang_format_on_all_sources.py:
#   Script to apply clang-format recursively on directory,
#   example usage:
#       ./scripts/apply_clang_format_on_all_sources.py src

from __future__ import print_function

import os
import sys
import platform
import subprocess

# inplace change and use style from .clang-format
CLANG_FORMAT_ARGS = ['-i', '-style=file']


def main(directory):
    system = platform.system()

    clang_format_exe = 'clang-format'
    if system == 'Windows':
        clang_format_exe += '.bat'

    partial_cmd = [clang_format_exe] + CLANG_FORMAT_ARGS

    for subdir, _, files in os.walk(directory):
        if 'third_party' in subdir:
            continue

        for f in files:
            if f.endswith(('.c', '.h', '.cpp', '.hpp')):
                f_abspath = os.path.join(subdir, f)
                print("Applying clang-format on ", f_abspath)
                subprocess.check_call(partial_cmd + [f_abspath])


if __name__ == '__main__':
    if len(sys.argv) > 2:
        print('Too mang args', file=sys.stderr)

    elif len(sys.argv) == 2:
        main(os.path.join(os.getcwd(), sys.argv[1]))

    else:
        main(os.getcwd())
