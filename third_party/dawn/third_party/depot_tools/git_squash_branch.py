#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import sys

import gclient_utils
import git_common


def main(args):
    if gclient_utils.IsEnvCog():
        print('squash-branch command is not supported in non-git environment.',
              file=sys.stderr)
        return 1
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-m',
        '--message',
        metavar='<msg>',
        default=None,
        help='Use the given <msg> as the first line of the commit message.')
    opts = parser.parse_args(args)
    if git_common.is_dirty_git_tree('squash-branch'):
        return 1
    git_common.squash_current_branch(opts.message)
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
