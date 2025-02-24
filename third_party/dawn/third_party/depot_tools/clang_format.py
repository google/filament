#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Redirects to the version of clang-format checked into the Chrome tree.

clang-format binaries are pulled down from Google Cloud Storage whenever you
sync Chrome, to platform-specific locations. This script knows how to locate
those tools, assuming the script is invoked from inside a Chromium checkout."""

import detect_host_arch
import gclient_paths
import os
import subprocess
import sys


class NotFoundError(Exception):
    """A file could not be found."""
    def __init__(self, e):
        Exception.__init__(
            self,
            'Problem while looking for clang-format in Chromium source tree:\n'
            '%s' % e)


def FindClangFormatToolInChromiumTree():
    """Return a path to the clang-format executable, or die trying."""
    primary_solution_path = gclient_paths.GetPrimarySolutionPath()
    if primary_solution_path:
        bin_path = os.path.join(primary_solution_path, 'third_party',
                                'clang-format',
                                'clang-format' + gclient_paths.GetExeSuffix())
        if os.path.exists(bin_path):
            return bin_path

    bin_path = gclient_paths.GetBuildtoolsPlatformBinaryPath()
    if not bin_path:
        raise NotFoundError(
            'Could not find checkout in any parent of the current path.\n'
            'Set CHROMIUM_BUILDTOOLS_PATH to use outside of a chromium '
            'checkout.')

    # TODO(b/336843583): Remove old_tool_path when migrated over
    old_tool_path = os.path.join(bin_path,
                                 'clang-format' + gclient_paths.GetExeSuffix())

    new_bin_path = bin_path
    arch = detect_host_arch.HostArch()
    if sys.platform == 'darwin' and arch == 'arm64':
        new_bin_path += '_arm64'
    old_new_tool_path = os.path.join(
        new_bin_path, 'format', 'clang-format' + gclient_paths.GetExeSuffix())
    latest_new_tool_path = os.path.join(
        f'{new_bin_path}-format', 'clang-format' + gclient_paths.GetExeSuffix())

    possible_paths = [latest_new_tool_path, old_new_tool_path, old_tool_path]
    for path in possible_paths:
        if os.path.exists(path):
            return path
    raise NotFoundError('File does not exist in either path: %s' %
                        possible_paths)


def FindClangFormatScriptInChromiumTree(script_name):
    """Return a path to a clang-format helper script, or die trying."""
    primary_solution_path = gclient_paths.GetPrimarySolutionPath()
    if primary_solution_path:
        script_path = os.path.join(primary_solution_path, 'third_party',
                                   'clang-format', 'script', script_name)
        if os.path.exists(script_path):
            return script_path

    tools_path = gclient_paths.GetBuildtoolsPath()
    if not tools_path:
        raise NotFoundError(
            'Could not find checkout in any parent of the current path.\n',
            'Set CHROMIUM_BUILDTOOLS_PATH to use outside of a chromium '
            'checkout.')

    script_path = os.path.join(tools_path, 'clang_format', 'script',
                               script_name)
    if not os.path.exists(script_path):
        raise NotFoundError('File does not exist: %s' % script_path)
    return script_path


def main(args):
    try:
        tool = FindClangFormatToolInChromiumTree()
    except NotFoundError as e:
        sys.stderr.write("%s\n" % str(e))
        return 1

    # Add some visibility to --help showing where the tool lives, since this
    # redirection can be a little opaque.
    help_syntax = ('-h', '--help', '-help', '-help-list', '--help-list')
    if any(match in args for match in help_syntax):
        print('\nDepot tools redirects you to the clang-format at:\n    %s\n' %
              tool)

    return subprocess.call([tool] + args)


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
