#!/usr/bin/env python3

# Copyright (C) 2022 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from github import Github
import sys, os

def print_usage():
    print('Verify that a GitHub pull request modifies a RELEASE_NOTES file')
    print()
    print('Usage:')
    print('  verify_release_notes.py [arguments]')
    print()
    print('Arguments:')
    print('1. github token')
    print('2. pull request number')
    print('3. bypass label name')
    print('4. release notes file path')
    print('5. pull request repo full name')
    print('6. pull request head ref')
    print()
    print('The GITHUB_REPOSITORY environment variable must be set (e.g., google/filament).')

def leave_single_comment(pull_request, comment_body):
    """ Leaves a comment on a PR once, without leaving a duplicate comment. """
    # To avoid spamming the PR author, we'll use this comment tag (which will render invisibly on
    # GitHub) to check if we've already left a comment on this PR.
    COMMENT_TAG = '<!-- verify_release_notes -->\n'
    comments = pull_request.get_issue_comments()
    for comment in comments:
        if comment.body.find(COMMENT_TAG) != -1:
            return
    # The GitHub token may not have WRITE permissions to leave a comment (for example, for 3P
    # contributors). In that case, we simply won't leave a comment.
    try:
        pull_request.create_issue_comment(f'{COMMENT_TAG}{comment_body}')
    except:
        print("Unable to leave comment. Continuing.")

# The first argument is the path to this script.
if len(sys.argv) != 7:
    print_usage()
    sys.exit(1)

authentication_token = sys.argv[1]
pull_number = sys.argv[2]
bypass_label_name = sys.argv[3]
release_notes_file = sys.argv[4]
pr_repo_full_name = sys.argv[5]
pr_head_ref = sys.argv[6]

g = Github(authentication_token)

repo_name = os.environ.get('GITHUB_REPOSITORY')
if repo_name is None:
    print("The GITHUB_REPOSITORY environment variable must be set.")
    sys.exit(1)

repo = g.get_repo(repo_name)

pull_request = repo.get_pull(int(pull_number))

# First check if the PR has the "bypass" label. This label is used for PRs that don't need to update
# RELEASE_NOTES. If so, we can exit immediately.
labels = [l.name for l in pull_request.labels]
if bypass_label_name in labels:
    print(f"PR number {pull_number} in repo {repo_name} contains the '{bypass_label_name}' label.")
    print("Exiting with success.")
    sys.exit(0)

# Next, check if the release notes file (RELEASE_NOTES.md or similar) has been modified.
files = pull_request.get_files()
for file in files:
    if file.filename == release_notes_file:
        print(f"PR number {pull_number} in repo {repo_name} modifies '{release_notes_file}'.")
        print("Exiting with success.")
        sys.exit(0)

# At this point, we issue a warning to the PR author to remember to modify the release notes, and
# exit with failure.
edit_url = f"https://github.com/{pr_repo_full_name}/edit/{pr_head_ref}/{release_notes_file}"
comment = (f"Please add a release note line to [{release_notes_file}]({edit_url}). "
    f"If this PR does not warrant a release note, add the '{bypass_label_name}' label "
    f"to this PR.")
print(comment)
leave_single_comment(pull_request, comment)

sys.exit(1)
