#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Rename the current branch while maintaining correct dependencies."""

import argparse
import sys

import subprocess2

from git_common import current_branch, run, set_branch_config, branch_config
from git_common import branch_config_map

import gclient_utils


def main(args):
    if gclient_utils.IsEnvCog():
        print('rename-branch command is not supported in non-git environment.',
              file=sys.stderr)
        return 1
    current = current_branch()
    if current == 'HEAD':
        current = None
    old_name_help = 'The old branch to rename.'
    if current:
        old_name_help += ' (default %(default)r)'

    parser = argparse.ArgumentParser()
    parser.add_argument('old_name',
                        nargs=('?' if current else 1),
                        help=old_name_help,
                        default=current)
    parser.add_argument('new_name', help='The new branch name.')

    opts = parser.parse_args(args)

    # when nargs=1, we get a list :(
    if isinstance(opts.old_name, list):
        opts.old_name = opts.old_name[0]

    try:
        run('branch', '-m', opts.old_name, opts.new_name)

        # update the downstreams
        for branch, merge in branch_config_map('merge').items():
            if merge == 'refs/heads/' + opts.old_name:
                # Only care about local branches
                if branch_config(branch, 'remote') == '.':
                    set_branch_config(branch, 'merge',
                                      'refs/heads/' + opts.new_name)
    except subprocess2.CalledProcessError as cpe:
        sys.stderr.write(cpe.stderr.decode('utf-8', 'replace'))
        return 1
    return 0


if __name__ == '__main__':  # pragma: no cover
    try:
        sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
