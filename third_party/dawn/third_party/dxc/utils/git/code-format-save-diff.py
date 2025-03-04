#!/usr/bin/env python3
#
# ====- code-format-save-diff, save diff from comment --*- python -*---------==#
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# ==-------------------------------------------------------------------------==#

import argparse
import os
import re
import subprocess
import sys
import tempfile
from functools import cached_property

import github
from github import IssueComment, PullRequest

LF = '\n'
CRLF = '\r\n'
CR = '\r'


def get_diff_from_comment(comment: IssueComment.IssueComment) -> str:
    diff_pat = re.compile(r"``````````diff(?P<DIFF>.+)``````````", re.DOTALL)
    m = re.search(diff_pat, comment.body)
    if m is None:
        raise Exception(f"Could not find diff in comment {comment.id}")
    diff = m.group("DIFF")
    # force to linux line endings
    diff = diff.replace(CRLF, LF).replace(CR, LF)
    return diff

def run_cmd(cmd: [str]) -> None:
    print(f"Running: {' '.join(cmd)}")
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        print(proc.stdout)
        print(proc.stderr)
        raise Exception(f"Failed to run {' '.join(cmd)}")

def apply_patches(args: argparse.Namespace) -> None:
    repo = github.Github(args.token).get_repo(args.repo)
    pr = repo.get_issue(args.issue_number).as_pull_request()

    comment = pr.get_issue_comment(args.comment_id)
    if comment is None:
        raise Exception(f"Comment {args.comment_id} does not exist")

    # get the diff from the comment
    diff = get_diff_from_comment(comment)

    # write diff to temporary file and apply
    if os.path.exists(args.tmp_diff_file):
        os.remove(args.tmp_diff_file)
    
    with open(args.tmp_diff_file, 'w+') as tmp:
        tmp.write(diff)
        tmp.flush()
        tmp.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--token", type=str, required=True, help="GitHub authentiation token"
    )
    parser.add_argument(
        "--repo",
        type=str,
        default=os.getenv("GITHUB_REPOSITORY", "llvm/llvm-project"),
        help="The GitHub repository that we are working with in the form of <owner>/<repo> (e.g. llvm/llvm-project)",
    )
    parser.add_argument("--issue-number", type=int, required=True)
    parser.add_argument("--comment-id", type=int, required=True)
    parser.add_argument(
        "--tmp-diff-file",
        type=str,
        required=True,
        help="Temporary file to write diff to",
    )

    args = parser.parse_args()

    apply_patches(args)
