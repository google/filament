#!/usr/bin/env python3
# Copyright (c) 2020-2025 Valve Corporation
# Copyright (c) 2020-2025 LunarG, Inc.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Script to determine if source code in Pull Request is properly formatted.
#
# This script checks for:
#   -- clang-format errors in the PR source code
#   -- out-of-date copyrights in PR source files
#   -- improperly formatted commit messages (using the function above)
#   -- assigning stype instead of using vku::InitStruct
#
# Notes:
#    Exits with non 0 exit code if formatting is needed.
#    Requires python3 to run correctly
#    In standalone mode (outside of CI), changes must be rebased on main
#        to get meaningful and complete results

import os
import argparse
import re
import subprocess
from subprocess import check_output
from argparse import RawDescriptionHelpFormatter

def repo_relative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', path))

#
#
# Color print routine, takes a string matching a txtcolor above and the output string, resets color upon exit
def CPrint(msg_type, msg_string):
    txtcolors = {'HELP_MSG':    '\033[0;36m',
                 'SUCCESS_MSG': '\033[1;32m',
                 'CONTENT':     '\033[1;39m',
                 'ERR_MSG':     '\033[1;31m',
                 'NO_COLOR':    '\033[0m'}
    print(txtcolors.get(msg_type, txtcolors['NO_COLOR']) + msg_string + txtcolors['NO_COLOR'])
#
#
# Check clang-formatting of source code diff
def VerifyClangFormatSource(commit, target_files):
    target_refspec = f'{commit}^...{commit}'
    retval = 0
    good_file_pattern = re.compile('.*\\.(cpp|cc|c\+\+|cxx|c|h|hpp)$')
    diff_files_list = [item for item in target_files if good_file_pattern.search(item)]
    diff_files = ' '.join([str(elem) for elem in diff_files_list])
    retval = 0
    if diff_files != '':
        git_diff = subprocess.Popen(('git', 'diff', '-U0', target_refspec, '--', diff_files), stdout=subprocess.PIPE)
        diff_files_data = subprocess.check_output(('python3', repo_relative('scripts/clang-format-diff.py'), '-p1', '-style=file'), stdin=git_diff.stdout)
        diff_files_data = diff_files_data.decode('utf-8')
        if diff_files_data != '':
            CPrint('ERR_MSG', "\nFound formatting errors!")
            CPrint('CONTENT', "\n" + diff_files_data)
            retval = 1
    return retval
#
#
# Check copyright dates for modified files
def VerifyCopyrights(commit, target_files):
    retval = 0
    is_lunarg_author = False
    authors = check_output(['git', 'log', '-n' , '1', '--format=%ae', commit])
    for author in authors.split(b'\n'):
        if author.endswith(b'@lunarg.com'):
            is_lunarg_author = True
            break

    if not is_lunarg_author:
        return 0

    # Handle year changes by respecting when the author wrote the code, rather
    # the day the script runs. This isn't exactly right yet, because really
    # we should evaluate it commit's files against that commit's date.
    commit_year = None
    # get all the author dates in YYYY-MM-DD format
    commit_dates = check_output(['git', 'log', '-n', '1', '--format=%as', commit])
    for cd in commit_dates.split(b'\n'):
        if len(cd) == 0:
            continue
        year = cd.split(b'-')[0]
        if not commit_year or int(commit_year) < int(year):
            commit_year = year.decode('utf-8')
    for file in target_files:
        if file is None:
            continue
        file_path = repo_relative(file)
        if not os.path.isfile(file_path):
            continue
        for company in ["LunarG", "Valve"]:
            # Capture the last year on the line as a separate match. It should be the highest (or only year of the range)
            copyright_match = re.search('Copyright .*(\d{4}) ' + company, open(file_path, encoding="utf-8", errors='ignore').read(1024))
            if copyright_match:
                copyright_year = copyright_match.group(1)
                if int(commit_year) > int(copyright_year):
                    msg = f'Change written in {commit_year} but copyright ends in {copyright_year}.'
                    CPrint('ERR_MSG', f'\n{file_path} has an out-of-date {company} copyright notice. {msg}')
                    retval = 1
    return retval
#
#
# Check commit message formats for commits in this PR/Branch
def VerifyCommitMessageFormat(commit):
    retval = 0

    # Construct correct commit list
    commit_text= check_output(['git', 'log', '-n', '1', '--pretty=format:%B', commit]).decode('utf-8')
    if commit_text is None:
        return retval

    msg_cur_line = 0
    msg_prev_line = ''
    for msg_line_text in commit_text.splitlines():
        msg_cur_line += 1
        line_length = len(msg_line_text)

        if msg_cur_line == 1:
            # Enforce subject line must be 64 chars or less
            if line_length > 64:
                CPrint('ERR_MSG', "The following subject line exceeds 64 characters in length.")
                CPrint('CONTENT', f"     '{msg_line_text}'\n")
                retval = 1
            # Output error if last char of subject line is not alpha-numeric
            if msg_line_text[-1] in '.,':
                CPrint('ERR_MSG', "For the following commit, the last character of the subject line must not be a period or comma.")
                CPrint('CONTENT',  f"     '{msg_line_text}'\n")
                retval = 1
            # Output error if subject line doesn't start with 'module: '
            if 'Revert' not in msg_line_text:
                module_name = msg_line_text.split(' ')[0]
                if module_name[-1] != ':':
                    CPrint('ERR_MSG', "The following subject line must start with a single word specifying the functional area of the change, followed by a colon and space.")
                    CPrint('ERR_MSG', "e.g., 'layers: Subject line here' or 'corechecks: Fix off-by-one error in ValidateFences'.")
                    CPrint('ERR_MSG', "Other common module names include layers, build, cmake, tests, docs, scripts, stateless, gpu, syncval, practices, etc.")
                    CPrint('CONTENT',  f"     '{msg_line_text}'\n")
                    retval = 1
                else:
                    # Check if first character after the colon is lower-case
                    subject_body = msg_line_text.split(': ')[1]
                    if not subject_body[0].isupper():
                        CPrint('ERR_MSG', "The first word of the subject line after the ':' character must be capitalized.")
                        CPrint('CONTENT',  f"     '{msg_line_text}'\n")
                        retval = 1
            # Check that first character of subject line is not capitalized
            if msg_line_text[0].isupper():
                CPrint('ERR_MSG', "The first word of the subject line must be lower case.")
                CPrint('CONTENT', f"     '{msg_line_text}'\n")
                retval = 1
        elif msg_cur_line == 2:
            # Commit message must have a blank line between subject and body
            if line_length != 0:
                CPrint('ERR_MSG', "The following subject line must be followed by a blank line.")
                CPrint('CONTENT', f"     '{msg_prev_line}'\n")
                retval = 1
        else:
            # Lines in a commit message body must be less than 72 characters in length (but give some slack)
            if line_length > 76:
                CPrint('ERR_MSG', "The following commit message body line exceeds the 72 character limit.")
                CPrint('CONTENT', f"     '{msg_line_text}'\n")
                retval = 1
        msg_prev_line = msg_line_text
    if retval != 0:
        CPrint('HELP_MSG', "Commit Message Format Requirements:")
        CPrint('HELP_MSG', "-----------------------------------")
        CPrint('HELP_MSG', "o  Subject lines must be <= 64 characters in length")
        CPrint('HELP_MSG', "o  Subject lines must start with a module keyword which is lower-case and followed by a colon and a space")
        CPrint('HELP_MSG', "o  The first word following the colon must be capitalized and the subject line must not end in a '.'")
        CPrint('HELP_MSG', "o  The subject line must be followed by a blank line")
        CPrint('HELP_MSG', "o  The commit description must be <= 72 characters in width\n")
        CPrint('HELP_MSG', "Examples:")
        CPrint('HELP_MSG', "---------")
        CPrint('HELP_MSG', "     build: Fix Vulkan header/registry detection for SDK")
        CPrint('HELP_MSG', "     tests: Fix QueryPerformanceIncompletePasses stride usage")
        CPrint('HELP_MSG', "     corechecks: Fix validation of VU 03227")
        CPrint('HELP_MSG', "     state_tracker: Remove 'using std::*' statements")
        CPrint('HELP_MSG', "     stateless: Account for DynStateWithCount for multiViewport\n")
        CPrint('HELP_MSG', "Refer to this document for additional detail:")
        CPrint('HELP_MSG', "https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/main/CONTRIBUTING.md#coding-conventions-and-formatting")
    return retval

#
#
# Check for test code assigning sType instead of using vku::InitStruc in this PR/Branch
def VerifyTypeAssign(commit, target_files):
    retval = 0
    target_refspec = f'{commit}^...{commit}'

    test_files_list = [item for item in target_files if item.startswith('tests/')]
    test_files = ' '.join([str(elem) for elem in test_files_list])
    if not test_files:
        return 0
    test_diff = subprocess.Popen(('git', 'diff', '-U0', target_refspec, '--', test_files), stdout=subprocess.PIPE)
    stdout, stderr = test_diff.communicate()
    stdout = stdout.decode('utf-8')
    stype_regex = re.compile(r'\.sType\s*=')
    on_regex = re.compile(r'stype-check\s*on')
    off_regex = re.compile(r'stype-check\s*off')
    checking = True
    for line in stdout.split('\n'):
        if not line.startswith('-'):
            if checking:
                if off_regex.search(line, re.IGNORECASE):
                    checking = False
                elif stype_regex.search(line):
                    CPrint('ERR_MSG', "Test assigning sType instead of using vku::InitStruct")
                    CPrint('ERR_MSG', "If this is a case where vku::InitStruct cannot be used, //stype-check off can be used to turn off sType checking")
                    CPrint('CONTENT', "     '" + line + "'\n")
                    retval = 1
            else:
                if on_regex.search(line, re.IGNORECASE):
                    checking = True
    return retval
#
#
# Entrypoint
def main():
    DEFAULT_REFSPEC = 'origin/main'

    parser = argparse.ArgumentParser(description='''Usage: python ./scripts/check_code_format.py
    - Reqires python3 and clang-format
    - Run script in repo root
    - May produce inaccurate clang-format results if local branch is not rebased on the TARGET_REFSPEC
    ''', formatter_class=RawDescriptionHelpFormatter)
    parser.add_argument('--target-refspec', metavar='TARGET_REFSPEC', type=str, dest='target_refspec', help = 'Refspec to '
        + 'diff against (default is origin/main)', default=DEFAULT_REFSPEC)
    parser.add_argument('--base-refspec', metavar='BASE_REFSPEC', type=str, dest='base_refspec', help = 'Base refspec to '
        + ' compare (default is HEAD)', default='HEAD')
    parser.add_argument('--fetch-main', dest='fetch_main', action='store_true', help='Fetch the main branch first.'
        + ' Useful with --target-refspec=FETCH_HEAD to compare against what is currently on main')
    args = parser.parse_args()

    if os.path.isfile('check_code_format.py'):
        os.chdir('..')

    target_refspec = args.target_refspec
    base_refspec = args.base_refspec

    if args.fetch_main:
        print('Fetching main branch...')
        subprocess.check_call(['git', 'fetch', 'https://github.com/KhronosGroup/Vulkan-ValidationLayers.git', 'main'])

    # Check if this is a merge commit
    commit_parents = check_output(['git', 'rev-list', '--parents', '-n', '1', 'HEAD'])
    if len(commit_parents.split(b' ')) > 2:
        # If this is a merge commit, this is a PR being built, and has been merged into main for testing.
        # The first parent (HEAD^) is going to be main, the second parent (HEAD^2) is going to be the PR commit.
        # TODO (ncesario) We should *ONLY* get here when on github CI, building a PR. Should probably print a
        #      warning if this happens locally.
        target_refspec = 'HEAD^'
        base_refspec = 'HEAD^2'

    orig_branch = check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).decode('utf-8').splitlines()[0]
    if orig_branch == 'HEAD':
        orig_branch = check_output(['git', 'rev-parse', 'HEAD']).decode('utf-8').splitlines()[0]

    commits = check_output(['git', 'log', '--format=%h', f'{base_refspec}...{target_refspec}']).split(b'\n')
    commits.reverse()

    # Run code format check on each commit in a PR so that we ensure that each commit is correct.
    failure = 0
    for c in commits:
        if len(c) == 0:
            continue

        commit = c.decode('utf-8')
        diff_range = f'{commit}^...{commit}'

        commit_message = check_output(['git', 'log', '--pretty="%h %s"', diff_range])
        CPrint('CONTENT', "\nChecking commit: " + commit_message.decode('utf-8'))

        subprocess.run(['git', 'checkout', '-q', commit])

        # Get list of files involved in this commit
        target_files_data = subprocess.check_output(['git', 'log', '-n', '1', '--name-only', commit])
        target_files = target_files_data.decode('utf-8')
        target_files = target_files.split("\n")

        # Exceptions of files we don't want to check (TODO - need better way to do this)
        if 'layers/external/vma/vk_mem_alloc.h' in target_files:
            target_files.remove('layers/external/vma/vk_mem_alloc.h')

        # Skip checking dependabot commits
        authors = subprocess.check_output(['git', 'log', '-n' , '1', '--format=%ae', commit]).decode('utf-8')
        if "dependabot" in authors:
            continue

        failure |= VerifyClangFormatSource(commit, target_files)
        failure |= VerifyCopyrights(commit, target_files)
        failure |= VerifyCommitMessageFormat(commit)
        failure |= VerifyTypeAssign(commit, target_files)

    subprocess.run(['git', 'checkout', '-q', orig_branch])

    if failure:
        CPrint('ERR_MSG', "One or more format checks failed.\n")
        exit(1)

    CPrint('SUCCESS_MSG', "All format checks passed.\n")

if __name__ == '__main__':
  main()
