#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Create new branch tracking origin HEAD by default.
"""

import argparse
import sys

import gclient_utils
import git_common
import subprocess2


def create_new_branch(branch_name,
                      upstream_current=False,
                      upstream=None,
                      inject_current=False):
    upstream = upstream or git_common.root()
    try:
        if inject_current:
            below = git_common.current_branch()
            if below is None:
                raise Exception('no current branch')
            above = git_common.upstream(below)
            if above is None:
                raise Exception('branch %s has no upstream' % (below))
            git_common.run('checkout', '--track', above, '-b', branch_name)
            git_common.run('branch', '--set-upstream-to', branch_name, below)
        elif upstream_current:
            git_common.run('checkout', '--track', '-b', branch_name)
        else:
            if upstream in git_common.tags():
                # TODO(iannucci): ensure that basis_ref is an ancestor of HEAD?
                git_common.run('checkout', '--no-track', '-b', branch_name,
                               git_common.hash_one(upstream))
                git_common.set_config('branch.%s.remote' % branch_name, '.')
                git_common.set_config('branch.%s.merge' % branch_name, upstream)
            else:
                # TODO(iannucci): Detect unclean workdir then stash+pop if we
                # need to teleport to a conflicting portion of history?
                git_common.run('checkout', '--track', upstream, '-b',
                               branch_name)
        git_common.get_or_create_merge_base(branch_name)
    except subprocess2.CalledProcessError as cpe:
        sys.stdout.write(cpe.stdout.decode('utf-8', 'replace'))
        sys.stderr.write(cpe.stderr.decode('utf-8', 'replace'))
        return 1
    sys.stderr.write('Switched to branch %s.\n' % branch_name)
    return 0


def main(args):
    if gclient_utils.IsEnvCog():
        print('new-branch command is not supported in non-git environment.',
              file=sys.stderr)
        return 1
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        description=__doc__,
    )
    parser.add_argument('branch_name')
    g = parser.add_mutually_exclusive_group()
    g.add_argument('--upstream-current',
                   '--upstream_current',
                   action='store_true',
                   help='set upstream branch to current branch.')
    g.add_argument('--upstream',
                   metavar='REF',
                   help='upstream branch (or tag) to track.')
    g.add_argument('--inject-current',
                   '--inject_current',
                   action='store_true',
                   help='new branch adopts current branch\'s upstream,' +
                   ' and new branch becomes current branch\'s upstream.')
    g.add_argument('--lkgr',
                   action='store_const',
                   const='origin/lkgr',
                   dest='upstream',
                   help='set basis ref for new branch to lkgr.')

    opts = parser.parse_args(args)

    return create_new_branch(opts.branch_name, opts.upstream_current,
                             opts.upstream, opts.inject_current)


if __name__ == '__main__':  # pragma: no cover
    try:
        sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
