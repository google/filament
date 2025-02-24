#!/usr/bin/env python3
# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Redirects to the version of rustfmt present in the Chrome tree.

Rust binaries are pulled down from Google Cloud Storage whenever you sync
Chrome. This script knows how to locate those tools, assuming the script is
invoked from inside a Chromium checkout."""

import gclient_paths
import os
import subprocess
import sys


class NotFoundError(Exception):
    """A file could not be found."""
    def __init__(self, e):
        Exception.__init__(
            self, 'Problem while looking for rustfmt in Chromium source tree:\n'
            '%s' % e)


def FindRustfmtToolInChromiumTree():
    """Return a path to the rustfmt executable, or die trying."""
    chromium_src_path = gclient_paths.GetPrimarySolutionPath()
    if not chromium_src_path:
        raise NotFoundError(
            'Could not find checkout in any parent of the current path.\n'
            'Set CHROMIUM_BUILDTOOLS_PATH to use outside of a chromium '
            'checkout.')

    tool_path = os.path.join(chromium_src_path, 'third_party', 'rust-toolchain',
                             'bin', 'rustfmt' + gclient_paths.GetExeSuffix())
    if not os.path.exists(tool_path):
        raise NotFoundError('File does not exist: %s' % tool_path)
    return tool_path


def IsRustfmtSupported():
    try:
        FindRustfmtToolInChromiumTree()
        return True
    except NotFoundError:
        return False


def main(args):
    try:
        tool = FindRustfmtToolInChromiumTree()
    except NotFoundError as e:
        sys.stderr.write("%s\n" % str(e))
        return 1

    # Add some visibility to --help showing where the tool lives, since this
    # redirection can be a little opaque.
    help_syntax = ('-h', '--help', '-help', '-help-list', '--help-list')
    if any(match in args for match in help_syntax):
        print('\nDepot tools redirects you to the rustfmt at:\n    %s\n' % tool)

    return subprocess.call([tool] + args)


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
