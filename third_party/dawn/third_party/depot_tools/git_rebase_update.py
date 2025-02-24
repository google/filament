#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Tool to update all branches to have the latest changes from their upstreams.
"""

import argparse
import collections
import logging
import sys
import textwrap
import os

from fnmatch import fnmatch
from pprint import pformat

import gclient_utils
import git_common as git

STARTING_BRANCH_KEY = 'depot-tools.rebase-update.starting-branch'
STARTING_WORKDIR_KEY = 'depot-tools.rebase-update.starting-workdir'


def find_return_branch_workdir():
    """Finds the branch and working directory which we should return to after
    rebase-update completes.

    These values may persist across multiple invocations of rebase-update, if
    rebase-update runs into a conflict mid-way.
    """
    return_branch = git.get_config(STARTING_BRANCH_KEY)
    workdir = git.get_config(STARTING_WORKDIR_KEY)
    if not return_branch:
        workdir = os.getcwd()
        git.set_config(STARTING_WORKDIR_KEY, workdir)
        return_branch = git.current_branch()
        if return_branch != 'HEAD':
            git.set_config(STARTING_BRANCH_KEY, return_branch)

    return return_branch, workdir


def fetch_remotes(branch_tree):
    """Fetches all remotes which are needed to update |branch_tree|."""
    fetch_tags = False
    remotes = set()
    tag_set = git.tags()
    fetchspec_map = {}
    all_fetchspec_configs = git.get_config_regexp(r'^remote\..*\.fetch')
    for key, fetchspec in all_fetchspec_configs:
        dest_spec = fetchspec.partition(':')[2]
        remote_name = key.split('.')[1]
        fetchspec_map[dest_spec] = remote_name
    for parent in branch_tree.values():
        if parent in tag_set:
            fetch_tags = True
        else:
            full_ref = git.run('rev-parse', '--symbolic-full-name', parent)
            for dest_spec, remote_name in fetchspec_map.items():
                if fnmatch(full_ref, dest_spec):
                    remotes.add(remote_name)
                    break

    fetch_args = []
    if fetch_tags:
        # Need to fetch all because we don't know what remote the tag comes from
        # :( TODO(iannucci): assert that the tags are in the remote fetch
        # refspec
        fetch_args = ['--all']
    else:
        fetch_args.append('--multiple')
        fetch_args.extend(remotes)
    # TODO(iannucci): Should we fetch git-svn?

    if not fetch_args:  # pragma: no cover
        print('Nothing to fetch.')
    else:
        git.run_with_stderr('fetch',
                            *fetch_args,
                            stdout=sys.stdout,
                            stderr=sys.stderr)


def remove_empty_branches(branch_tree):
    tag_set = git.tags()
    ensure_root_checkout = git.once(lambda: git.run('checkout', git.root()))

    deletions = {}
    reparents = {}
    downstreams = collections.defaultdict(list)
    for branch, parent in git.topo_iter(branch_tree, top_down=False):
        if git.is_dormant(branch):
            continue

        downstreams[parent].append(branch)

        # If branch and parent have the same tree, then branch has to be marked
        # for deletion and its children and grand-children reparented to parent.
        if git.hash_one(branch + ":") == git.hash_one(parent + ":"):
            ensure_root_checkout()

            logging.debug('branch %s merged to %s', branch, parent)

            # Mark branch for deletion while remembering the ordering, then add
            # all its children as grand-children of its parent and record
            # reparenting information if necessary.
            deletions[branch] = len(deletions)

            for down in downstreams[branch]:
                if down in deletions:
                    continue

                # Record the new and old parent for down, or update such a
                # record if it already exists. Keep track of the ordering so
                # that reparenting happen in topological order.
                downstreams[parent].append(down)
                if down not in reparents:
                    reparents[down] = (len(reparents), parent, branch)
                else:
                    order, _, old_parent = reparents[down]
                    reparents[down] = (order, parent, old_parent)

    # Apply all reparenting recorded, in order.
    for branch, value in sorted(reparents.items(), key=lambda x: x[1][0]):
        _, parent, old_parent = value
        if parent in tag_set:
            git.set_branch_config(branch, 'remote', '.')
            git.set_branch_config(branch, 'merge', 'refs/tags/%s' % parent)
            print('Reparented %s to track %s [tag] (was tracking %s)' %
                  (branch, parent, old_parent))
        else:
            git.run('branch', '--set-upstream-to', parent, branch)
            print('Reparented %s to track %s (was tracking %s)' %
                  (branch, parent, old_parent))

    # Apply all deletions recorded, in order.
    for branch, _ in sorted(deletions.items(), key=lambda x: x[1]):
        print(git.run('branch', '-d', branch))


def rebase_branch(branch, parent, start_hash):
    logging.debug('considering %s(%s) -> %s(%s) : %s', branch,
                  git.hash_one(branch), parent, git.hash_one(parent),
                  start_hash)

    # If parent has FROZEN commits, don't base branch on top of them. Instead,
    # base branch on top of whatever commit is before them.
    back_ups = 0
    orig_parent = parent
    while git.run('log', '-n1', '--format=%s', parent,
                  '--').startswith(git.FREEZE):
        back_ups += 1
        parent = git.run('rev-parse', parent + '~')

    if back_ups:
        logging.debug('Backed parent up by %d from %s to %s', back_ups,
                      orig_parent, parent)

    if git.hash_one(parent) != start_hash:
        # Try a plain rebase first
        print('Rebasing:', branch)
        rebase_ret = git.rebase(parent, start_hash, branch, abort=False)
        if not rebase_ret.success:
            mid_rebase_message = textwrap.dedent("""\
                Your working copy is in mid-rebase. Either:
                 * completely resolve like a normal git-rebase; OR
                 * try squashing your branch first
                   (git rebase --abort && git squash-branch) and try
                   again; OR
                 * abort the rebase and mark this branch as dormant:
                       git rebase --abort && \\
                       git config branch.%s.dormant true

                And then run `git rebase-update -n` to resume.
                """ % branch)
            print(mid_rebase_message)
            return False
    else:
        print('%s up-to-date' % branch)

    git.remove_merge_base(branch)
    git.get_or_create_merge_base(branch)

    return True


def with_downstream_branches(base_branches, branch_tree):
    """Returns a set of base_branches and all downstream branches."""
    downstream_branches = set()
    for branch, parent in git.topo_iter(branch_tree):
        if parent in base_branches or parent in downstream_branches:
            downstream_branches.add(branch)

    return downstream_branches.union(base_branches)


def main(args=None):
    if gclient_utils.IsEnvCog():
        print(
            'rebase-update command is not supported. Please navigate to source '
            'control view in the activity bar to rebase your changes.',
            file=sys.stderr)
        return 1
    parser = argparse.ArgumentParser()
    parser.add_argument('--verbose', '-v', action='store_true')
    parser.add_argument('--keep-going',
                        '-k',
                        action='store_true',
                        help='Keep processing past failed rebases.')
    parser.add_argument('--no_fetch',
                        '--no-fetch',
                        '-n',
                        action='store_true',
                        help='Skip fetching remotes.')
    parser.add_argument('--current',
                        action='store_true',
                        help='Only rebase the current branch.')
    parser.add_argument('--tree',
                        action='store_true',
                        help='Rebase all branches downstream from the '
                        'selected branch(es).')
    parser.add_argument('branches',
                        nargs='*',
                        help='Branches to be rebased. All branches are assumed '
                        'if none specified.')
    parser.add_argument('--keep-empty',
                        '-e',
                        action='store_true',
                        help='Do not automatically delete empty branches.')

    opts = parser.parse_args(args)

    if opts.verbose:  # pragma: no cover
        logging.getLogger().setLevel(logging.DEBUG)

    # TODO(iannucci): snapshot all branches somehow, so we can implement
    #                 `git rebase-update --undo`.
    #   * Perhaps just copy packed-refs + refs/ + logs/ to the side?
    #     * commit them to a secret ref?
    #       * Then we could view a summary of each run as a
    #         `diff --stat` on that secret ref.

    if git.in_rebase():
        # TODO(iannucci): Be able to resume rebase with flags like --continue,
        # etc.
        print('Rebase in progress. Please complete the rebase before running '
              '`git rebase-update`.')
        return 1

    return_branch, return_workdir = find_return_branch_workdir()
    os.chdir(git.run('rev-parse', '--show-toplevel'))

    if git.current_branch() == 'HEAD':
        if git.run('status', '--porcelain', '--ignore-submodules=all'):
            print(
                'Cannot rebase-update with detached head + uncommitted changes.'
            )
            return 1
    else:
        git.freeze()  # just in case there are any local changes.

    branches_to_rebase = set(opts.branches)
    if opts.current:
        branches_to_rebase.add(git.current_branch())

    skipped, branch_tree = git.get_branch_tree(use_limit=not opts.current)
    if opts.tree:
        branches_to_rebase = with_downstream_branches(branches_to_rebase,
                                                      branch_tree)

    if branches_to_rebase:
        skipped = set(skipped).intersection(branches_to_rebase)
    for branch in skipped:
        print('Skipping %s: No upstream specified' % branch)

    if not opts.no_fetch:
        fetch_remotes(branch_tree)

    merge_base = {}
    for branch, parent in branch_tree.items():
        merge_base[branch] = git.get_or_create_merge_base(branch, parent)

    logging.debug('branch_tree: %s' % pformat(branch_tree))
    logging.debug('merge_base: %s' % pformat(merge_base))

    retcode = 0
    unrebased_branches = []
    # Rebase each branch starting with the root-most branches and working
    # towards the leaves.
    for branch, parent in git.topo_iter(branch_tree):
        # Only rebase specified branches, unless none specified.
        if branches_to_rebase and branch not in branches_to_rebase:
            continue
        if git.is_dormant(branch):
            print('Skipping dormant branch', branch)
        else:
            ret = rebase_branch(branch, parent, merge_base[branch])
            if not ret:
                retcode = 1

                if opts.keep_going:
                    print('--keep-going set, continuing with next branch.')
                    unrebased_branches.append(branch)
                    if git.in_rebase():
                        git.run_with_retcode('rebase', '--abort')
                    if git.in_rebase():  # pragma: no cover
                        print(
                            'Failed to abort rebase. Something is really wrong.'
                        )
                        break
                else:
                    break

    if unrebased_branches:
        print()
        print('The following branches could not be cleanly rebased:')
        for branch in unrebased_branches:
            print('  %s' % branch)

    if not retcode:
        if not opts.keep_empty:
            remove_empty_branches(branch_tree)

        # return_branch may not be there any more.
        if return_branch in git.branches(use_limit=False):
            git.run('checkout', return_branch)
            git.thaw()
        else:
            root_branch = git.root()
            if return_branch != 'HEAD':
                print(
                    "%s was merged with its parent, checking out %s instead." %
                    (git.unicode_repr(return_branch),
                     git.unicode_repr(root_branch)))
            git.run('checkout', root_branch)

        # return_workdir may also not be there any more.
        if return_workdir:
            try:
                os.chdir(return_workdir)
            except OSError as e:
                print("Unable to return to original workdir %r: %s" %
                      (return_workdir, e))
        git.set_config(STARTING_BRANCH_KEY, '')
        git.set_config(STARTING_WORKDIR_KEY, '')

        print()
        print("Running `git gc --auto` - Ctrl-C to abort is OK.")
        git.run('gc', '--auto')

    return retcode


if __name__ == '__main__':  # pragma: no cover
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
