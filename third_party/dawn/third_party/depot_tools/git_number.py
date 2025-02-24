#!/usr/bin/env vpython3
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Usage: %prog [options] [<commitref>]*

If no <commitref>'s are supplied, it defaults to HEAD.

Calculates the generation number for one or more commits in a git repo.

Generation number of a commit C with parents P is defined as:
  generation_number(C, []) = 0
  generation_number(C, P)  = max(map(generation_number, P)) + 1

This number can be used to order commits relative to each other, as long as for
any pair of the commits, one is an ancestor of the other.

Since calculating the generation number of a commit requires walking that
commit's entire history, this script caches all calculated data inside the git
repo that it operates on in the ref 'refs/number/commits'.
"""

import binascii
import collections
import logging
import optparse
import os
import struct
import sys
import tempfile

import git_common as git
import subprocess2

CHUNK_FMT = '!20sL'
CHUNK_SIZE = struct.calcsize(CHUNK_FMT)
DIRTY_TREES = collections.defaultdict(int)
REF = 'refs/number/commits'
AUTHOR_NAME = 'git-number'
AUTHOR_EMAIL = 'chrome-infrastructure-team@google.com'

# Number of bytes to use for the prefix on our internal number structure.
# 0 is slow to deserialize. 2 creates way too much bookkeeping overhead (would
# need to reimplement cache data structures to be a bit more sophisticated than
# dicts. 1 seems to be just right.
PREFIX_LEN = 1

# Set this to 'threads' to gather coverage data while testing.
POOL_KIND = 'procs'


def pathlify(hash_prefix):
    """Converts a binary object hash prefix into a posix path, one folder per
    byte.

    >>> pathlify('\xDE\xAD')
    'de/ad'
    """
    return '/'.join('%02x' % b for b in hash_prefix)


@git.memoize_one(threadsafe=False)
def get_number_tree(prefix_bytes):
    """Returns a dictionary of the git-number registry specified by
    |prefix_bytes|.

    This is in the form of {<full binary ref>: <gen num> ...}

    >>> get_number_tree('\x83\xb4')
    {'\x83\xb4\xe3\xe4W\xf9J*\x8f/c\x16\xecD\xd1\x04\x8b\xa9qz': 169, ...}
    """
    ref = '%s:%s' % (REF, pathlify(prefix_bytes))

    try:
        raw = git.run('cat-file', 'blob', ref, autostrip=False, decode=False)
        return dict(
            struct.unpack_from(CHUNK_FMT, raw, i * CHUNK_SIZE)
            for i in range(len(raw) // CHUNK_SIZE))
    except subprocess2.CalledProcessError:
        return {}


@git.memoize_one(threadsafe=False)
def get_num(commit_hash):
    """Returns the generation number for a commit.

    Returns None if the generation number for this commit hasn't been calculated
    yet (see load_generation_numbers()).
    """
    return get_number_tree(commit_hash[:PREFIX_LEN]).get(commit_hash)


def clear_caches(on_disk=False):
    """Clears in-process caches for e.g. unit testing."""
    get_number_tree.clear()
    get_num.clear()
    if on_disk:
        git.run('update-ref', '-d', REF)


def intern_number_tree(tree):
    """Transforms a number tree (in the form returned by |get_number_tree|) into
    a git blob.

    Returns the git blob id as hex-encoded string.

    >>> d = {'\x83\xb4\xe3\xe4W\xf9J*\x8f/c\x16\xecD\xd1\x04\x8b\xa9qz': 169}
    >>> intern_number_tree(d)
    'c552317aa95ca8c3f6aae3357a4be299fbcb25ce'
    """
    with tempfile.TemporaryFile() as f:
        for k, v in sorted(tree.items()):
            f.write(struct.pack(CHUNK_FMT, k, v))
        f.seek(0)
        return git.intern_f(f)


def leaf_map_fn(pre_tree):
    """Converts a prefix and number tree into a git index line."""
    pre, tree = pre_tree
    return '100644 blob %s\t%s\0' % (intern_number_tree(tree), pathlify(pre))


def finalize(targets):
    """Saves all cache data to the git repository.

    After calculating the generation number for |targets|, call finalize() to
    save all the work to the git repository.

    This in particular saves the trees referred to by DIRTY_TREES.
    """
    if not DIRTY_TREES:
        return

    msg = 'git-number Added %s numbers' % sum(DIRTY_TREES.values())

    idx = os.path.join(git.run('rev-parse', '--git-dir'), 'number.idx')
    env = os.environ.copy()
    env['GIT_INDEX_FILE'] = str(idx)

    progress_message = 'Finalizing: (%%(count)d/%d)' % len(DIRTY_TREES)
    with git.ProgressPrinter(progress_message) as inc:
        git.run('read-tree', REF, env=env)

        prefixes_trees = ((p, get_number_tree(p)) for p in sorted(DIRTY_TREES))
        updater = subprocess2.Popen(
            ['git', 'update-index', '-z', '--index-info'],
            stdin=subprocess2.PIPE,
            env=env)

        with git.ScopedPool(kind=POOL_KIND) as leaf_pool:
            for item in leaf_pool.imap(leaf_map_fn, prefixes_trees):
                updater.stdin.write(item.encode())
                inc()

        updater.stdin.close()
        updater.wait()
        assert updater.returncode == 0

        tree_id = git.run('write-tree', env=env)
        commit_cmd = [
            # Git user.name and/or user.email may not be configured, so
            # specifying them explicitly. They are not used, but required by
            # Git.
            '-c',
            'user.name=%s' % AUTHOR_NAME,
            '-c',
            'user.email=%s' % AUTHOR_EMAIL,
            'commit-tree',
            '-m',
            msg,
            '-p'
        ] + git.hash_multi(REF)
        for t in targets:
            commit_cmd.extend(['-p', binascii.hexlify(t).decode()])
        commit_cmd.append(tree_id)
        commit_hash = git.run(*commit_cmd)
        git.run('update-ref', REF, commit_hash)
    DIRTY_TREES.clear()


def preload_tree(prefix):
    """Returns the prefix and parsed tree object for the specified prefix."""
    return prefix, get_number_tree(prefix)


def all_prefixes(depth=PREFIX_LEN):
    prefixes = [bytes([i]) for i in range(255)]
    for x in prefixes:
        # This isn't covered because PREFIX_LEN currently == 1
        if depth > 1:  # pragma: no cover
            for r in all_prefixes(depth - 1):
                yield x + r
        else:
            yield x


def load_generation_numbers(targets):
    """Populates the caches of get_num and get_number_tree so they contain
    the results for |targets|.

    Loads cached numbers from disk, and calculates missing numbers if one or
    more of |targets| is newer than the cached calculations.

    Args:
        targets - An iterable of binary-encoded full git commit hashes.
    """
    # In case they pass us a generator, listify targets.
    targets = list(targets)

    if all(get_num(t) is not None for t in targets):
        return

    if git.tree(REF) is None:
        empty = git.mktree({})
        commit_hash = git.run(
            # Git user.name and/or user.email may not be configured, so
            # specifying them explicitly. They are not used, but required by
            # Git.
            '-c',
            'user.name=%s' % AUTHOR_NAME,
            '-c',
            'user.email=%s' % AUTHOR_EMAIL,
            'commit-tree',
            '-m',
            'Initial commit from git-number',
            empty)
        git.run('update-ref', REF, commit_hash)

    with git.ScopedPool(kind=POOL_KIND) as pool:
        preload_iter = pool.imap_unordered(preload_tree, all_prefixes())

        rev_list = []

        with git.ProgressPrinter('Loading commits: %(count)d') as inc:
            # Curiously, buffering the list into memory seems to be the fastest
            # approach in python (as opposed to iterating over the lines in the
            # stdout as they're produced). GIL strikes again :/
            cmd = [
                'rev-list',
                '--topo-order',
                '--parents',
                '--reverse',
                '^' + REF,
            ] + [binascii.hexlify(target).decode() for target in targets]
            for line in git.run(*cmd).splitlines():
                tokens = [binascii.unhexlify(token) for token in line.split()]
                rev_list.append((tokens[0], tokens[1:]))
                inc()

        get_number_tree.update(preload_iter)

    with git.ProgressPrinter('Counting: %%(count)d/%d' % len(rev_list)) as inc:
        for commit_hash, pars in rev_list:
            num = max(map(get_num, pars)) + 1 if pars else 0

            prefix = commit_hash[:PREFIX_LEN]
            get_number_tree(prefix)[commit_hash] = num
            DIRTY_TREES[prefix] += 1
            get_num.set(commit_hash, num)

            inc()


def main():  # pragma: no cover
    parser = optparse.OptionParser(usage=sys.modules[__name__].__doc__)
    parser.add_option('--no-cache',
                      action='store_true',
                      help='Do not actually cache anything we calculate.')
    parser.add_option('--reset',
                      action='store_true',
                      help='Reset the generation number cache and quit.')
    parser.add_option('-v',
                      '--verbose',
                      action='count',
                      default=0,
                      help='Be verbose. Use more times for more verbosity.')
    opts, args = parser.parse_args()

    levels = [logging.ERROR, logging.INFO, logging.DEBUG]
    logging.basicConfig(level=levels[min(opts.verbose, len(levels) - 1)])

    # 'git number' should only be used on bots.
    if os.getenv('CHROME_HEADLESS') != '1':
        logging.error(
            "'git-number' is an infrastructure tool that is only "
            "intended to be used internally by bots. Developers should "
            "use the 'Cr-Commit-Position' value in the commit's message.")
        return 1

    if opts.reset:
        clear_caches(on_disk=True)
        return

    try:
        targets = git.parse_commitrefs(*(args or ['HEAD']))
    except git.BadCommitRefException as e:
        parser.error(e)

    load_generation_numbers(targets)
    if not opts.no_cache:
        finalize(targets)

    print('\n'.join(map(str, map(get_num, targets))))
    return 0


if __name__ == '__main__':  # pragma: no cover
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
