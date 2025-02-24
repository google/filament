#!/usr/bin/env python3
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Wrapper around git blame that ignores certain commits.
"""

import argparse
import collections
import logging
import os
import subprocess2
import sys

import gclient_utils
import git_common
import git_dates
import setup_color

logging.getLogger().setLevel(logging.INFO)


class Commit(object):
    """Info about a commit."""
    def __init__(self, commithash):
        self.commithash = commithash
        self.author = None
        self.author_mail = None
        self.author_time = None
        self.author_tz = None
        self.committer = None
        self.committer_mail = None
        self.committer_time = None
        self.committer_tz = None
        self.summary = None
        self.boundary = None
        self.previous = None
        self.filename = None

    def __repr__(self):  # pragma: no cover
        return '<Commit %s>' % self.commithash


BlameLine = collections.namedtuple(
    'BlameLine', 'commit context lineno_then lineno_now modified')


def parse_blame(blameoutput):
    """Parses the output of git blame -p into a data structure."""
    lines = blameoutput.split('\n')
    i = 0
    commits = {}

    while i < len(lines):
        # Read a commit line and parse it.
        line = lines[i]
        i += 1
        if not line.strip():
            continue
        commitline = line.split()
        commithash = commitline[0]
        lineno_then = int(commitline[1])
        lineno_now = int(commitline[2])

        try:
            commit = commits[commithash]
        except KeyError:
            commit = Commit(commithash)
            commits[commithash] = commit

        # Read commit details until we find a context line.
        while i < len(lines):
            line = lines[i]
            i += 1
            if line.startswith('\t'):
                break

            try:
                key, value = line.split(' ', 1)
            except ValueError:
                key = line
                value = True
            setattr(commit, key.replace('-', '_'), value)

        context = line[1:]

        yield BlameLine(commit, context, lineno_then, lineno_now, False)


def print_table(outbuf, table, align):
    """Print a 2D rectangular array, aligning columns with spaces.

    Args:
        align: string of 'l' and 'r', designating whether each column is
            left- or right-aligned.
    """
    if len(table) == 0:
        return

    colwidths = None
    for row in table:
        if colwidths is None:
            colwidths = [len(x) for x in row]
        else:
            colwidths = [max(colwidths[i], len(x)) for i, x in enumerate(row)]

    for row in table:
        cells = []
        for i, cell in enumerate(row):
            padding = ' ' * (colwidths[i] - len(cell))
            if align[i] == 'r':
                cell = padding + cell
            elif i < len(row) - 1:
                # Do not pad the final column if left-aligned.
                cell += padding
            cells.append(cell.encode('utf-8', 'replace'))
        try:
            outbuf.write(b' '.join(cells) + b'\n')
        except IOError:  # pragma: no cover
            # Can happen on Windows if the pipe is closed early.
            pass


def pretty_print(outbuf, parsedblame, show_filenames=False):
    """Pretty-prints the output of parse_blame."""
    table = []
    for line in parsedblame:
        author_time = git_dates.timestamp_offset_to_datetime(
            line.commit.author_time, line.commit.author_tz)
        row = [
            line.commit.commithash[:8], '(' + line.commit.author,
            git_dates.datetime_string(author_time),
            str(line.lineno_now) + ('*' if line.modified else '') + ')',
            line.context
        ]
        if show_filenames:
            row.insert(1, line.commit.filename)
        table.append(row)
    print_table(outbuf, table, align='llllrl' if show_filenames else 'lllrl')


def get_parsed_blame(filename, revision='HEAD'):
    blame = git_common.blame(filename, revision=revision, porcelain=True)
    return list(parse_blame(blame))


# Map from (oldrev, newrev) to hunk list (caching the results of git diff, but
# only the hunk line numbers, not the actual diff contents).
# hunk list contains (old, new) pairs, where old and new are (start, length)
# pairs. A hunk list can also be None (if the diff failed).
diff_hunks_cache = {}


def cache_diff_hunks(oldrev, newrev):
    def parse_start_length(s):
        # Chop the '-' or '+'.
        s = s[1:]
        # Length is optional (defaults to 1).
        try:
            start, length = s.split(',')
        except ValueError:
            start = s
            length = 1
        return int(start), int(length)

    try:
        return diff_hunks_cache[(oldrev, newrev)]
    except KeyError:
        pass

    # Use -U0 to get the smallest possible hunks.
    diff = git_common.diff(oldrev, newrev, '-U0')

    # Get all the hunks.
    hunks = []
    for line in diff.split('\n'):
        if not line.startswith('@@'):
            continue
        ranges = line.split(' ', 3)[1:3]
        ranges = tuple(parse_start_length(r) for r in ranges)
        hunks.append(ranges)

    diff_hunks_cache[(oldrev, newrev)] = hunks
    return hunks


def approx_lineno_across_revs(filename, newfilename, revision, newrevision,
                              lineno):
    """Computes the approximate movement of a line number between two revisions.

    Consider line |lineno| in |filename| at |revision|. This function computes
    the line number of that line in |newfilename| at |newrevision|. This is
    necessarily approximate.

    Args:
        filename: The file (within the repo) at |revision|.
        newfilename: The name of the same file at |newrevision|.
        revision: A git revision.
        newrevision: Another git revision. Note: Can be ahead or behind
            |revision|.
        lineno: Line number within |filename| at |revision|.

    Returns:
        Line number within |newfilename| at |newrevision|.
    """
    # This doesn't work that well if there are a lot of line changes within the
    # hunk (demonstrated by
    # GitHyperBlameLineMotionTest.testIntraHunkLineMotion). A fuzzy heuristic
    # that takes the text of the new line and tries to find a deleted line
    # within the hunk that mostly matches the new line could help.

    # Use the <revision>:<filename> syntax to diff between two blobs. This is
    # the only way to diff a file that has been renamed.
    old = '%s:%s' % (revision, filename)
    new = '%s:%s' % (newrevision, newfilename)
    hunks = cache_diff_hunks(old, new)

    cumulative_offset = 0

    # Find the hunk containing lineno (if any).
    for (oldstart, oldlength), (newstart, newlength) in hunks:
        cumulative_offset += newlength - oldlength

        if lineno >= oldstart + oldlength:
            # Not there yet.
            continue

        if lineno < oldstart:
            # Gone too far.
            break

        # lineno is in [oldstart, oldlength] at revision; [newstart, newlength]
        # at newrevision.

        # If newlength == 0, newstart will be the line before the deleted hunk.
        # Since the line must have been deleted, just return that as the nearest
        # line in the new file. Caution: newstart can be 0 in this case.
        if newlength == 0:
            return max(1, newstart)

        newend = newstart + newlength - 1

        # Move lineno based on the amount the entire hunk shifted.
        lineno = lineno + newstart - oldstart
        # Constrain the output within the range [newstart, newend].
        return min(newend, max(newstart, lineno))

    # Wasn't in a hunk. Figure out the line motion based on the difference in
    # length between the hunks seen so far.
    return lineno + cumulative_offset


def hyper_blame(outbuf, ignored, filename, revision):
    # Map from commit to parsed blame from that commit.
    blame_from = {}
    filename = os.path.normpath(filename)

    def cache_blame_from(filename, commithash):
        try:
            return blame_from[commithash]
        except KeyError:
            parsed = get_parsed_blame(filename, commithash)
            blame_from[commithash] = parsed
            return parsed

    try:
        parsed = cache_blame_from(filename, git_common.hash_one(revision))
    except subprocess2.CalledProcessError as e:
        sys.stderr.write(e.stderr.decode())
        return e.returncode

    new_parsed = []

    # We don't show filenames in blame output unless we have to.
    show_filenames = False

    for line in parsed:
        # If a line references an ignored commit, blame that commit's parent
        # repeatedly until we find a non-ignored commit.
        while line.commit.commithash in ignored:
            if line.commit.previous is None:
                # You can't ignore the commit that added this file.
                break

            previouscommit, previousfilename = line.commit.previous.split(
                ' ', 1)
            parent_blame = cache_blame_from(previousfilename, previouscommit)

            if len(parent_blame) == 0:
                # The previous version of this file was empty, therefore, you
                # can't ignore this commit.
                break

            # line.lineno_then is the line number in question at line.commit. We
            # need to translate that line number so that it refers to the
            # position of the same line on previouscommit.
            lineno_previous = approx_lineno_across_revs(line.commit.filename,
                                                        previousfilename,
                                                        line.commit.commithash,
                                                        previouscommit,
                                                        line.lineno_then)
            logging.debug('ignore commit %s on line p%d/t%d/n%d',
                          line.commit.commithash, lineno_previous,
                          line.lineno_then, line.lineno_now)

            # Get the line at lineno_previous in the parent commit.
            assert 1 <= lineno_previous <= len(parent_blame)
            newline = parent_blame[lineno_previous - 1]

            # Replace the commit and lineno_then, but not the lineno_now or
            # context.
            line = BlameLine(newline.commit, line.context, newline.lineno_then,
                             line.lineno_now, True)
            logging.debug('    replacing with %r', line)

        # If any line has a different filename to the file's current name, turn
        # on filename display for the entire blame output. Use normpath to make
        # variable consistent across platforms.
        if os.path.normpath(line.commit.filename) != filename:
            show_filenames = True

        new_parsed.append(line)

    pretty_print(outbuf, new_parsed, show_filenames=show_filenames)

    return 0


def parse_ignore_file(ignore_file):
    for line in ignore_file:
        line = line.split('#', 1)[0].strip()
        if line:
            yield line


def main(args, outbuf):
    if gclient_utils.IsEnvCog():
        print('hyper-blame command is not supported in non-git environment.',
              file=sys.stderr)
        return 1
    parser = argparse.ArgumentParser(
        prog='git hyper-blame',
        description='git blame with support for ignoring certain commits.')
    parser.add_argument('-i',
                        metavar='REVISION',
                        action='append',
                        dest='ignored',
                        default=[],
                        help='a revision to ignore')
    parser.add_argument('--ignore-file',
                        metavar='FILE',
                        dest='ignore_file',
                        help='a file containing a list of revisions to ignore')
    parser.add_argument(
        '--no-default-ignores',
        dest='no_default_ignores',
        action='store_true',
        help='Do not ignore commits from .git-blame-ignore-revs.')
    parser.add_argument('revision',
                        nargs='?',
                        default='HEAD',
                        metavar='REVISION',
                        help='revision to look at')
    parser.add_argument('filename', metavar='FILE', help='filename to blame')

    args = parser.parse_args(args)
    try:
        repo_root = git_common.repo_root()
    except subprocess2.CalledProcessError as e:
        sys.stderr.write(e.stderr.decode())
        return e.returncode

    # Make filename relative to the repository root, and cd to the root dir (so
    # all filenames throughout this script are relative to the root).
    filename = os.path.relpath(args.filename, repo_root)
    os.chdir(repo_root)

    # Normalize filename so we can compare it to other filenames git gives us.
    filename = os.path.normpath(filename)
    filename = os.path.normcase(filename)

    ignored_list = list(args.ignored)
    if not args.no_default_ignores and \
        os.path.exists(git_common.GIT_BLAME_IGNORE_REV_FILE):
        with open(git_common.GIT_BLAME_IGNORE_REV_FILE) as ignore_file:
            ignored_list.extend(parse_ignore_file(ignore_file))

    if args.ignore_file:
        with open(args.ignore_file) as ignore_file:
            ignored_list.extend(parse_ignore_file(ignore_file))

    ignored = set()
    for c in ignored_list:
        try:
            ignored.add(git_common.hash_one(c))
        except subprocess2.CalledProcessError as e:
            # Custom warning string (the message from git-rev-parse is
            # inappropriate).
            sys.stderr.write('warning: unknown revision \'%s\'.\n' % c)

    return hyper_blame(outbuf, ignored, filename, args.revision)


if __name__ == '__main__':  # pragma: no cover
    setup_color.init()
    with git_common.less() as less_input:
        sys.exit(main(sys.argv[1:], less_input))
