#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Print dependency tree of branches in local repo.

Example:
origin/main
  cool_feature
    dependent_feature
    other_dependent_feature
  other_feature

Branches are colorized as follows:
  * Red - a remote branch (usually the root of all local branches)
  * Cyan - a local branch which is the same as HEAD
    * Note that multiple branches may be Cyan, if they are all on the same
      commit, and you have that commit checked out.
  * Green - a local branch
  * Blue - a 'branch-heads' branch
  * Magenta - a tag
  * Magenta '{NO UPSTREAM}' - If you have local branches which do not track any
    upstream, then you will see this.
"""

import argparse
import collections
import metrics
import subprocess2
import sys

from git_common import current_branch, upstream, tags, get_branches_info
from git_common import get_git_version, MIN_UPSTREAM_TRACK_GIT_VERSION, hash_one
from git_common import get_config, run

import gclient_utils
import setup_color

from third_party.colorama import Fore, Style

DEFAULT_SEPARATOR = ' ' * 4


class OutputManager(object):
    """Manages a number of OutputLines and formats them into aligned columns."""
    def __init__(self):
        self.lines = []
        self.nocolor = False
        self.max_column_lengths = []
        self.num_columns = None

    def append(self, line):
        # All lines must have the same number of columns.
        if not self.num_columns:
            self.num_columns = len(line.columns)
            self.max_column_lengths = [0] * self.num_columns
        assert self.num_columns == len(line.columns)

        if self.nocolor:
            line.colors = [''] * self.num_columns

        self.lines.append(line)

        # Update maximum column lengths.
        for i, col in enumerate(line.columns):
            self.max_column_lengths[i] = max(self.max_column_lengths[i],
                                             len(col))

    def merge(self, other):
        for line in other.lines:
            self.append(line)

    def as_formatted_string(self):
        return '\n'.join(
            l.as_padded_string(self.max_column_lengths) for l in self.lines)


class OutputLine(object):
    """A single line of data.

    This consists of an equal number of columns, colors and separators."""
    def __init__(self):
        self.columns = []
        self.separators = []
        self.colors = []

    def append(self, data, separator=DEFAULT_SEPARATOR, color=Fore.WHITE):
        self.columns.append(data)
        self.separators.append(separator)
        self.colors.append(color)

    def as_padded_string(self, max_column_lengths):
        """"Returns the data as a string with each column padded to
        |max_column_lengths|."""
        output_string = ''
        for i, (color, data, separator) in enumerate(
                zip(self.colors, self.columns, self.separators)):
            if max_column_lengths[i] == 0:
                continue

            padding = (max_column_lengths[i] - len(data)) * ' '
            output_string += color + data + padding + separator

        return output_string.rstrip()


class BranchMapper(object):
    """A class which constructs output representing the tree's branch structure.

    Attributes:
        __branches_info: a map of branches to their BranchesInfo objects which
            consist of the branch hash, upstream and ahead/behind status.
        __gone_branches: a set of upstreams which are not fetchable by git
    """
    def __init__(self):
        self.verbosity = 0
        self.maxjobs = 0
        self.show_subject = False
        self.hide_dormant = False
        self.output = OutputManager()
        self.__gone_branches = set()
        self.__branches_info = None
        self.__parent_map = collections.defaultdict(list)
        self.__current_branch = None
        self.__current_hash = None
        self.__tag_set = None
        self.__status_info = {}

    def start(self):
        self.__branches_info = get_branches_info(
            include_tracking_status=self.verbosity >= 1)
        if (self.verbosity >= 2):
            # Avoid heavy import unless necessary.
            from git_cl import get_cl_statuses, color_for_status, Changelist

            change_cls = [
                Changelist(branchref='refs/heads/' + b)
                for b in self.__branches_info.keys() if b
            ]
            status_info = get_cl_statuses(change_cls,
                                          fine_grained=self.verbosity > 2,
                                          max_processes=self.maxjobs)

            # This is a blocking get which waits for the remote CL status to be
            # retrieved.
            for cl, status in status_info:
                self.__status_info[cl.GetBranch()] = (cl.GetIssueURL(
                    short=True), color_for_status(status), status)

        roots = set()

        # A map of parents to a list of their children.
        for branch, branch_info in self.__branches_info.items():
            if not branch_info:
                continue

            if self.__check_cycle(branch):
                continue
            parent = branch_info.upstream
            if not self.__branches_info[parent]:
                # If the parent is not a known branch, it may be an upstream
                # branch like origin/main or it may be gone. Determine which it
                # is, but don't re-query the same parent multiple times.
                if parent not in roots:
                    if not upstream(branch):
                        self.__gone_branches.add(parent)
                    roots.add(parent)

            self.__parent_map[parent].append(branch)

        self.__current_branch = current_branch()
        self.__current_hash = hash_one('HEAD', short=True)
        self.__tag_set = tags()

        if roots:
            for root in sorted(roots):
                self.__append_branch(root, self.output)
        else:
            no_branches = OutputLine()
            no_branches.append('No User Branches')
            self.output.append(no_branches)

    def __check_cycle(self, branch):
        # Maximum length of the cycle is `num_branches`. This limit avoids
        # running into a cycle which does *not* contain `branch`.
        num_branches = len(self.__branches_info)
        cycle = [branch]
        while len(cycle) < num_branches and self.__branches_info[cycle[-1]]:
            parent = self.__branches_info[cycle[-1]].upstream
            cycle.append(parent)
            if parent == branch:
                print('Warning: Detected cycle in branches: {}'.format(
                    ' -> '.join(cycle)),
                      file=sys.stderr)
                return True
        return False

    def __is_invalid_parent(self, parent):
        return not parent or parent in self.__gone_branches

    def __color_for_branch(self, branch, branch_hash):
        if branch.startswith('origin/'):
            color = Fore.RED
        elif branch.startswith('branch-heads'):
            color = Fore.BLUE
        elif self.__is_invalid_parent(branch) or branch in self.__tag_set:
            color = Fore.MAGENTA
        elif self.__current_hash.startswith(branch_hash):
            color = Fore.CYAN
        else:
            color = Fore.GREEN

        if branch_hash and self.__current_hash.startswith(branch_hash):
            color += Style.BRIGHT
        else:
            color += Style.NORMAL

        return color

    def __is_dormant_branch(self, branch):
        if '/' in branch:
            return False

        return get_config(f'branch.{branch}.dormant') == 'true'

    def __append_branch(self, branch, output, depth=0):
        """Recurses through the tree structure and appends an OutputLine to the
        OutputManager for each branch."""
        child_output = OutputManager()
        for child in sorted(self.__parent_map.pop(branch, ())):
            self.__append_branch(child, child_output, depth=depth + 1)

        is_dormant_branch = self.__is_dormant_branch(branch)
        if self.hide_dormant and is_dormant_branch and not child_output.lines:
            return

        branch_info = self.__branches_info[branch]
        if branch_info:
            branch_hash = branch_info.hash
        else:
            try:
                branch_hash = hash_one(branch, short=True)
            except subprocess2.CalledProcessError:
                branch_hash = None

        line = OutputLine()

        # The branch name with appropriate indentation.
        suffix = ''
        if branch == self.__current_branch or (self.__current_branch == 'HEAD'
                                               and branch
                                               == self.__current_hash):
            suffix = ' *'
        branch_string = branch
        if branch in self.__gone_branches:
            branch_string = '{%s:GONE}' % branch
        if not branch:
            branch_string = '{NO_UPSTREAM}'
        main_string = '  ' * depth + branch_string + suffix
        line.append(main_string,
                    color=self.__color_for_branch(branch, branch_hash))

        # The branch hash.
        if self.verbosity >= 2:
            line.append(branch_hash or '', separator=' ', color=Fore.RED)

        # The branch tracking status.
        if self.verbosity >= 1:
            commits_string = ''
            behind_string = ''
            front_separator = ''
            center_separator = ''
            back_separator = ''
            if branch_info and not self.__is_invalid_parent(
                    branch_info.upstream):
                behind = branch_info.behind
                commits = branch_info.commits

                if commits:
                    commits_string = '%d commit' % commits
                    commits_string += 's' if commits > 1 else ' '
                if behind:
                    behind_string = 'behind %d' % behind

                if commits or behind:
                    front_separator = '['
                    back_separator = ']'

                if commits and behind:
                    center_separator = '|'

            line.append(front_separator, separator=' ')
            line.append(commits_string, separator=' ', color=Fore.MAGENTA)
            line.append(center_separator, separator=' ')
            line.append(behind_string, separator=' ', color=Fore.MAGENTA)
            line.append(back_separator)

        if self.verbosity >= 4:
            line.append(' (dormant)' if is_dormant_branch else '          ',
                        separator='  ',
                        color=Fore.RED)

        # The Rietveld issue associated with the branch.
        if self.verbosity >= 2:
            (url, color,
             status) = (('', '', '') if self.__is_invalid_parent(branch) else
                        self.__status_info[branch])
            if self.verbosity > 2:
                line.append('{} ({})'.format(url, status) if url else '',
                            color=color)
            else:
                line.append(url or '', color=color)

        # The subject of the most recent commit on the branch.
        if self.show_subject:
            if not self.__is_invalid_parent(branch):
                line.append(run('log', '-n1', '--format=%s', branch, '--'))
            else:
                line.append('')

        output.append(line)

        output.merge(child_output)


def print_desc():
    for line in __doc__.splitlines():
        starpos = line.find('* ')
        if starpos == -1 or '-' not in line:
            print(line)
        else:
            _, color, rest = line.split(None, 2)
            outline = line[:starpos + 1]
            outline += getattr(Fore,
                               color.upper()) + " " + color + " " + Fore.RESET
            outline += rest
            print(outline)
    print('')


@metrics.collector.collect_metrics('git map-branches')
def main(argv):
    if gclient_utils.IsEnvCog():
        print('map-branches command is not supported in non-git environment.',
              file=sys.stderr)
        return 1
    setup_color.init()
    if get_git_version() < MIN_UPSTREAM_TRACK_GIT_VERSION:
        print(
            'This tool will not show all tracking information for git version '
            'earlier than ' +
            '.'.join(str(x) for x in MIN_UPSTREAM_TRACK_GIT_VERSION) +
            '. Please consider upgrading.',
            file=sys.stderr)

    if '-h' in argv:
        print_desc()

    parser = argparse.ArgumentParser()
    parser.add_argument('-v',
                        action='count',
                        default=0,
                        help=('Pass once to show tracking info, '
                              'twice for hash and review url, '
                              'thrice for review status, '
                              'four times to mark dormant branches'))
    parser.add_argument('--no-color',
                        action='store_true',
                        dest='nocolor',
                        help='Turn off colors.')
    parser.add_argument(
        '-j',
        '--maxjobs',
        action='store',
        type=int,
        help='The number of jobs to use when retrieving review status')
    parser.add_argument('--show-subject',
                        action='store_true',
                        dest='show_subject',
                        help='Show the commit subject.')
    parser.add_argument('--hide-dormant',
                        action='store_true',
                        dest='hide_dormant',
                        help='Hides dormant branches.')

    opts = parser.parse_args(argv)

    mapper = BranchMapper()
    mapper.verbosity = opts.v
    mapper.output.nocolor = opts.nocolor
    mapper.maxjobs = opts.maxjobs
    mapper.show_subject = opts.show_subject
    mapper.hide_dormant = opts.hide_dormant
    mapper.start()
    print(mapper.output.as_formatted_string())
    return 0


if __name__ == '__main__':
    try:
        with metrics.collector.print_notice_and_exit():
            sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
