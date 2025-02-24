#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Change the upstream of the current branch."""

import argparse
import sys

import subprocess2

from git_common import upstream, current_branch, run, tags, set_branch_config
from git_common import get_or_create_merge_base, root, manual_merge_base
from git_common import get_branch_tree, topo_iter

import gclient_utils
import git_rebase_update
import metrics


@metrics.collector.collect_metrics('git reparent-branch')
def main(args):
    if gclient_utils.IsEnvCog():
        print(
            'reparent-branch command is not supported in non-git environment.',
            file=sys.stderr)
        return 1
    root_ref = root()

    parser = argparse.ArgumentParser()
    g = parser.add_mutually_exclusive_group()
    g.add_argument('new_parent',
                   nargs='?',
                   help='New parent branch (or tag) to reparent to.')
    g.add_argument('--root',
                   action='store_true',
                   help='Reparent to the configured root branch (%s).' %
                   root_ref)
    g.add_argument('--lkgr',
                   action='store_true',
                   help='Reparent to the lkgr tag.')
    opts = parser.parse_args(args)

    # TODO(iannucci): Allow specification of the branch-to-reparent

    branch = current_branch()

    if opts.root:
        new_parent = root_ref
    elif opts.lkgr:
        new_parent = 'lkgr'
    else:
        if not opts.new_parent:
            parser.error('Must specify new parent somehow')
        new_parent = opts.new_parent
    cur_parent = upstream(branch)

    if branch == 'HEAD' or not branch:
        parser.error('Must be on the branch you want to reparent')
    if new_parent == cur_parent:
        parser.error('Cannot reparent a branch to its existing parent')

    if not cur_parent:
        msg = (
            "Unable to determine %s@{upstream}.\n\nThis can happen if you "
            "didn't use `git new-branch` to create the branch and haven't used "
            "`git branch --set-upstream-to` to assign it one.\n\nPlease assign "
            "an upstream branch and then run this command again.")
        print(msg % branch, file=sys.stderr)
        return 1

    mbase = get_or_create_merge_base(branch, cur_parent)

    all_tags = tags()
    if cur_parent in all_tags:
        cur_parent += ' [tag]'

    try:
        run('show-ref', new_parent)
    except subprocess2.CalledProcessError:
        print('fatal: invalid reference: %s' % new_parent, file=sys.stderr)
        return 1

    if new_parent in all_tags:
        print("Reparenting %s to track %s [tag] (was %s)" %
              (branch, new_parent, cur_parent))
        set_branch_config(branch, 'remote', '.')
        set_branch_config(branch, 'merge', new_parent)
    else:
        print("Reparenting %s to track %s (was %s)" %
              (branch, new_parent, cur_parent))
        run('branch', '--set-upstream-to', new_parent, branch)

    manual_merge_base(branch, mbase, new_parent)

    # ONLY rebase-update the branch which moved (and dependants)
    _, branch_tree = get_branch_tree()
    branches = [branch]
    for branch, parent in topo_iter(branch_tree):
        if parent in branches:
            branches.append(branch)
    return git_rebase_update.main(['--no-fetch', '--keep-empty'] + branches)


if __name__ == '__main__':  # pragma: no cover
    try:
        with metrics.collector.print_notice_and_exit():
            sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
