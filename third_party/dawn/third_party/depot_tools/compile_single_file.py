#!/usr/bin/env python3
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import subprocess
import sys

DEPOT_TOOLS_DIR = os.path.dirname(os.path.realpath(__file__))


# This function is inspired from the one in src/tools/vim/ninja-build.vim in the
# Chromium repository.
def path_to_source_root(path):
    """Returns the absolute path to the chromium source root."""
    candidate = os.path.dirname(path)
    # This is a list of directories that need to identify the src directory. The
    # shorter it is, the more likely it's wrong (checking for just
    # "build/common.gypi" would find "src/v8" for files below "src/v8", as
    # "src/v8/build/common.gypi" exists). The longer it is, the more likely it
    # is to break when we rename directories.
    fingerprints = ['chrome', 'net', 'v8', 'build', 'skia']
    while candidate and not all(
            os.path.isdir(os.path.join(candidate, fp)) for fp in fingerprints):
        new_candidate = os.path.dirname(candidate)
        if new_candidate == candidate:
            raise Exception("Couldn't find source-dir from %s" % path)
        candidate = os.path.dirname(candidate)
    return candidate


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--file-path',
        help='The file path, could be absolute or relative to the current '
        'directory.',
        required=True)
    parser.add_argument(
        '--build-dir',
        help='The build directory, relative to the source directory.',
        required=True)

    options = parser.parse_args()

    src_dir = path_to_source_root(os.path.abspath(options.file_path))
    abs_build_dir = os.path.join(src_dir, options.build_dir)
    src_relpath = os.path.relpath(options.file_path, abs_build_dir)

    print('Building %s' % options.file_path)

    carets = '^'
    if sys.platform == 'win32':
        # The caret character has to be escaped on Windows as it's an escape
        # character.
        carets = '^^'

    command = [
        sys.executable,
        os.path.join(DEPOT_TOOLS_DIR, 'autoninja.py'), '-C', abs_build_dir,
        '%s%s' % (src_relpath, carets)
    ]
    # |shell| should be set to True on Windows otherwise the carets characters
    # get dropped from the command line.
    return subprocess.call(command, shell=sys.platform == 'win32')


if __name__ == '__main__':
    sys.exit(main())
