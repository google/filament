#!/usr/bin/env python

import os
import subprocess


def git_get_commit_hash():
    try:
        return subprocess.check_output(
            ['git', 'rev-parse', '--short=8', 'HEAD']).decode('ascii').strip()
    except subprocess.CalledProcessError:
        return "00000000"


def git_get_commit_count():
    try:
        return subprocess.check_output(
            ['git', 'rev-list', '--count', 'HEAD']).decode('ascii').strip()
    except subprocess.CalledProcessError:
        return 0


def compose_commit_namespace(git_count, git_hash):
    return ('namespace {{\n'
            '  const uint32_t kGitCommitCount = {}u;\n'
            '  const char kGitCommitHash[] = "{}";\n'
            '}}\n').format(git_count, git_hash)

def update(srcdir, commit_file):
    # Read the original commit info
    try:
        f = open(commit_file)
        prev_commit = f.read()
        f.close()
    except:
        prev_commit = ''

    prev_cwd = os.getcwd()
    os.chdir(srcdir)
    cur_commit = compose_commit_namespace(
        git_get_commit_count(), git_get_commit_hash())
    os.chdir(prev_cwd)

    # Update if different: avoid triggering rebuilding unnecessarily
    if cur_commit != prev_commit:
        with open(commit_file, 'w') as f:
            f.write(cur_commit)


def main():
    import argparse

    parser = argparse.ArgumentParser(
        description='Generate file containing Git commit information')
    parser.add_argument('srcpath', metavar='<src-dir-path>', type=str,
                        help='Path to the source code directory')
    parser.add_argument('dstpath', metavar='<dst-file-path>', type=str,
                        help='Path to the generated file')

    args = parser.parse_args()

    update(args.srcpath, args.dstpath)


if __name__ == '__main__':
    main()
