#!/usr/bin/env python3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
usage: git map [-h] [--help] [<args>]

Enhances `git log --graph` view with information on commit branches + tags that
point to them. Items are colorized as follows:

  * Cyan    - Currently checked out branch
  * Green   - Local branch
  * Red     - Remote branches
  * Magenta - Tags
  * White   - Merge Base Markers
  * Blue background - The currently checked out commit
"""

import os
import sys

import gclient_utils
import git_common
import setup_color
import subprocess2

from third_party import colorama

RESET = colorama.Fore.RESET + colorama.Back.RESET + colorama.Style.RESET_ALL
BRIGHT = colorama.Style.BRIGHT

BLUE_BACK = colorama.Back.BLUE + BRIGHT
BRIGHT_RED = colorama.Fore.RED + BRIGHT
CYAN = colorama.Fore.CYAN + BRIGHT
GREEN = colorama.Fore.GREEN + BRIGHT
MAGENTA = colorama.Fore.MAGENTA + BRIGHT
RED = colorama.Fore.RED
WHITE = colorama.Fore.WHITE + BRIGHT
YELLOW = colorama.Fore.YELLOW


def _print_help(outbuf):
    names = {
        'Cyan': CYAN,
        'Green': GREEN,
        'Magenta': MAGENTA,
        'Red': RED,
        'White': WHITE,
        'Blue background': BLUE_BACK,
    }
    msg = ''
    for line in __doc__.splitlines():
        for name, color in names.items():
            if name in line:
                msg += line.replace('* ' + name,
                                    color + '* ' + name + RESET) + '\n'
                break
        else:
            msg += line + '\n'
    outbuf.write(msg.encode('utf-8', 'replace'))


def _color_branch(branch, all_branches, all_tags, current):
    if branch in (current, 'HEAD -> ' + current):
        color = CYAN
        current = None
    elif branch in all_branches:
        color = GREEN
        all_branches.remove(branch)
    elif branch in all_tags:
        color = MAGENTA
    elif branch.startswith('tag: '):
        color = MAGENTA
        branch = branch[len('tag: '):]
    else:
        color = RED
    return color + branch + RESET


def _color_branch_list(branch_list, all_branches, all_tags, current):
    if not branch_list:
        return ''
    colored_branches = (GREEN + ', ').join(
        _color_branch(branch, all_branches, all_tags, current)
        for branch in branch_list if branch != 'HEAD')
    return (GREEN + '(' + colored_branches + GREEN + ') ' + RESET)


def _parse_log_line(line):
    graph, branch_list, commit_date, subject = (line.decode(
        'utf-8', 'replace').strip().split('\x00'))
    branch_list = [] if not branch_list else branch_list.split(', ')
    commit = graph.split()[-1]
    graph = graph[:-len(commit)]
    return graph, commit, branch_list, commit_date, subject


def main(argv, outbuf):
    if '-h' in argv or '--help' in argv:
        _print_help(outbuf)
        return 0
    if gclient_utils.IsEnvCog():
        print('map command is not supported in non-git environment.',
              file=sys.stderr)
        return 1

    map_extra = git_common.get_config_list('depot_tools.map_extra')
    cmd = [
        git_common.GIT_EXE, 'log',
        git_common.root(), '--graph', '--branches', '--tags', '--color=always',
        '--date=short', '--pretty=format:%H%x00%D%x00%cd%x00%s'
    ] + map_extra + argv

    log_proc = subprocess2.Popen(cmd, stdout=subprocess2.PIPE, shell=False)

    current = git_common.current_branch()
    all_tags = set(git_common.tags())
    all_branches = set(git_common.branches())
    if current in all_branches:
        all_branches.remove(current)

    merge_base_map = {}
    for branch in all_branches:
        merge_base = git_common.get_or_create_merge_base(branch)
        if merge_base:
            merge_base_map.setdefault(merge_base, set()).add(branch)

    for merge_base, branches in merge_base_map.items():
        merge_base_map[merge_base] = ', '.join(branches)

    try:
        for line in log_proc.stdout:
            if b'\x00' not in line:
                outbuf.write(line)
                continue

            graph, commit, branch_list, commit_date, subject = _parse_log_line(
                line)

            if 'HEAD' in branch_list:
                graph = graph.replace('*', BLUE_BACK + '*')

            line = '{graph}{commit}\t{branches}{date} ~ {subject}'.format(
                graph=graph,
                commit=BRIGHT_RED + commit[:10] + RESET,
                branches=_color_branch_list(branch_list, all_branches, all_tags,
                                            current),
                date=YELLOW + commit_date + RESET,
                subject=subject)

            if commit in merge_base_map:
                line += '    <({})'.format(WHITE + merge_base_map[commit] +
                                           RESET)

            line += os.linesep
            outbuf.write(line.encode('utf-8', 'replace'))
    except (BrokenPipeError, KeyboardInterrupt):
        pass
    return 0


if __name__ == '__main__':
    setup_color.init()
    with git_common.less() as less_input:
        sys.exit(main(sys.argv[1:], less_input))
