#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Explicitly set/remove/print the merge-base for the current branch.

This manually set merge base will be a stand-in for `git merge-base` for the
purposes of the chromium depot_tools git extensions. Passing no arguments will
just print the effective merge base for the current branch.
"""

import argparse
import sys

from subprocess2 import CalledProcessError

from git_common import remove_merge_base, manual_merge_base, current_branch
from git_common import get_or_create_merge_base, hash_one, upstream

import gclient_utils


def main(argv):
    if gclient_utils.IsEnvCog():
        print('mark-merge-base command is not supported in non-git '
              'environment.', file=sys.stderr)
        return 1
    parser = argparse.ArgumentParser(
        description=__doc__.strip().splitlines()[0],
        epilog=' '.join(__doc__.strip().splitlines()[1:]))
    g = parser.add_mutually_exclusive_group()
    g.add_argument(
        'merge_base',
        nargs='?',
        help='The new hash to use as the merge base for the current branch')
    g.add_argument('--delete',
                   '-d',
                   action='store_true',
                   help='Remove the set mark.')
    opts = parser.parse_args(argv)

    cur = current_branch()

    if opts.delete:
        try:
            remove_merge_base(cur)
        except CalledProcessError:
            print('No merge base currently exists for %s.' % cur)
        return 0

    if opts.merge_base:
        try:
            opts.merge_base = hash_one(opts.merge_base)
        except CalledProcessError:
            print('fatal: could not resolve %s as a commit' % opts.merge_base,
                  file=sys.stderr)
            return 1

        manual_merge_base(cur, opts.merge_base, upstream(cur))

    ret = 0
    actual = get_or_create_merge_base(cur)
    if opts.merge_base and opts.merge_base != actual:
        ret = 1
        print("Invalid merge_base %s" % opts.merge_base)

    print("merge_base(%s): %s" % (cur, actual))
    return ret


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
