#!/usr/bin/env python3
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Usage: %prog <commit>*

Given a commit, finds the release where it first appeared (e.g. 47.0.2500.0) as
well as attempting to determine the branches to which it was merged.

Note that it uses the "cherry picked from" annotation to find merges, so it will
only work on merges that followed the "use cherry-pick -x" instructions.
"""

import optparse
import re
import sys

import gclient_utils
import git_common as git


def GetNameForCommit(sha1):
    name = re.sub(r'~.*$', '', git.run('name-rev', '--tags', '--name-only',
                                       sha1))
    if name == 'undefined':
        name = git.run('name-rev', '--refs', 'remotes/branch-heads/*',
                       '--name-only', sha1) + ' [untagged]'
    return name


def GetMergesForCommit(sha1):
    return [
        c.split()[0]
        for c in git.run('log', '--oneline', '-F', '--all', '--no-abbrev',
                         '--grep', 'cherry picked from commit %s' %
                         sha1).splitlines()
    ]


def main(args):
    if gclient_utils.IsEnvCog():
        print('find-releases command is not supported in non-git environment.',
              file=sys.stderr)
        return 1
    parser = optparse.OptionParser(usage=sys.modules[__name__].__doc__)
    _, args = parser.parse_args(args)

    if len(args) == 0:
        parser.error('Need at least one commit.')

    for arg in args:
        commit_name = GetNameForCommit(arg)
        if not commit_name:
            print('%s not found' % arg)
            return 1
        print('commit %s was:' % arg)
        print('  initially in ' + commit_name)
        merges = GetMergesForCommit(arg)
        for merge in merges:
            print('  merged to ' + GetNameForCommit(merge) + ' (as ' + merge +
                  ')')
        if not merges:
            print('No merges found. If this seems wrong, be sure that you did:')
            print('  git fetch origin && gclient sync --with_branch_heads')

    return 0


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
