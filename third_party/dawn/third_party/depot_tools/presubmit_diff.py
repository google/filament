#!/usr/bin/env python3
# Copyright (c) 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Tool for generating a unified git diff outside of a git workspace.

This is intended as a preprocessor for presubmit_support.py.
"""
from __future__ import annotations

import argparse
import base64
import concurrent.futures
import os
import platform
import sys

import gclient_utils
from gerrit_util import (CreateHttpConn, ReadHttpResponse,
                         MAX_CONCURRENT_CONNECTION)
import subprocess2

DEV_NULL = "/dev/null"
HEADER_DELIMITER = "@@"


def fetch_content(host: str, repo: str, ref: str, file: str) -> bytes:
    """Fetches the content of a file from Gitiles.

    If the file does not exist at the commit, returns an empty bytes object.

    Args:
      host: Gerrit host.
      repo: Gerrit repo.
      ref: Gerrit commit.
      file: Path of file to fetch.

    Returns:
        Bytes of the file at the commit or an empty bytes object if the file
        does not exist at the commit.
    """
    conn = CreateHttpConn(f"{host}.googlesource.com",
                          f"{repo}/+show/{ref}/{file}?format=text")
    response = ReadHttpResponse(conn, accept_statuses=[200, 404])
    return base64.b64decode(response.read())


def git_diff(src: str | None,
             dest: str | None,
             unified: int | None = None) -> str:
    """Returns the result of `git diff --no-index` between two paths.

    If a path is not specified, the diff is against /dev/null. At least one of
    src or dest must be specified.

    Args:
      src: Source path.
      dest: Destination path.
      unified: Number of lines of context. If None, git diff uses 3 as
        the default value.

    Returns:
        A string containing the git diff.
    """
    args = ["git", "diff", "--no-index"]
    if unified is not None:
        # git diff doesn't error out even if it's given a negative <n> value.
        # e.g., --unified=-3323, -U-3
        #
        # It just ignores the value and treats it as 0.
        # hence, this script doesn't bother validating the <n> value.
        args.append(f"-U{unified}")

    args.extend(["--", src or DEV_NULL, dest or DEV_NULL])
    return subprocess2.capture(args).decode("utf-8")


def _process_diff(diff: str, src_root: str, dst_root: str) -> str:
    """Adjust paths in the diff header so they're relative to the root.

    This also modifies paths on Windows to use forward slashes.
    """
    if not diff:
        return ""

    has_chunk_header = HEADER_DELIMITER in diff
    if has_chunk_header:
        header, body = diff.split(HEADER_DELIMITER, maxsplit=1)
    else:
        # Only the file mode changed.
        header = diff

    norm_src = src_root.rstrip(os.sep)
    norm_dst = dst_root.rstrip(os.sep)

    if platform.system() == "Windows":
        # Absolute paths on Windows use the format:
        #   "a/C:\\abspath\\to\\file.txt"
        header = header.replace("\\\\", "\\")
        header = header.replace('"', "")
        header = header.replace(norm_src + "\\", "")
        header = header.replace(norm_dst + "\\", "")
    else:
        # Other systems use:
        #  a/abspath/to/file.txt
        header = header.replace(norm_src, "")
        header = header.replace(norm_dst, "")

    if has_chunk_header:
        return header + HEADER_DELIMITER + body
    return header


def _create_diff(host: str, repo: str, ref: str, root: str, file: str,
                 unified: int | None) -> str:
    new_file = os.path.join(root, file)
    if not os.path.exists(new_file):
        new_file = None

    with gclient_utils.temporary_directory() as tmp_root:
        old_file = None
        old_content = fetch_content(host, repo, ref, file)
        if old_content:
            old_file = os.path.join(tmp_root, file)
            os.makedirs(os.path.dirname(old_file), exist_ok=True)
            with open(old_file, "wb") as f:
                f.write(old_content)

        if not old_file and not new_file:
            raise RuntimeError(f"Could not access file {file} from {root} "
                               f"or from {host}/{repo}:{ref}.")

        diff = git_diff(old_file, new_file, unified)
        return _process_diff(diff, tmp_root, root)


def create_diffs(host: str,
                 repo: str,
                 ref: str,
                 root: str,
                 files: list[str],
                 unified: int | None = None) -> dict[str, str]:
    """Calculates diffs of files in a directory against a commit.

    Args:
      host: Gerrit host.
      repo: Gerrit repo.
      ref: Gerrit commit.
      root: Path of local directory containing modified files.
      files: List of file paths relative to root.
      unified: Number of lines of context. If None, git diff uses 3 as
        the default value.

    Returns:
        A dict mapping file paths to diffs.

    Raises:
        RuntimeError: If a file is missing in both the root and the repo.
    """
    diffs = {}
    with concurrent.futures.ThreadPoolExecutor(
            max_workers=MAX_CONCURRENT_CONNECTION) as executor:
        futures_to_file = {
            executor.submit(_create_diff, host, repo, ref, root, file, unified):
            file
            for file in files
        }
        for future in concurrent.futures.as_completed(futures_to_file):
            file = futures_to_file[future]
            diffs[file] = future.result()
    return diffs


def main(argv):
    parser = argparse.ArgumentParser(
        usage="%(prog)s [options] <files...>",
        description="Makes a unified git diff against a Gerrit commit.",
    )
    parser.add_argument("--output", help="File to write the diff to.")
    parser.add_argument("--host", required=True, help="Gerrit host.")
    parser.add_argument("--repo", required=True, help="Gerrit repo.")
    parser.add_argument("--ref",
                        required=True,
                        help="Gerrit ref to diff against.")
    parser.add_argument("--root",
                        required=True,
                        help="Folder containing modified files.")
    parser.add_argument("-U",
                        "--unified",
                        required=False,
                        type=int,
                        help="generate diffs with <n> lines context",
                        metavar='<n>')
    parser.add_argument(
        "files",
        nargs="+",
        help="List of changed files. Paths are relative to the repo root.",
    )
    options = parser.parse_args(argv)

    diffs = create_diffs(options.host, options.repo, options.ref, options.root,
                         options.files, options.unified)

    unified_diff = "\n".join([d for d in diffs.values() if d])
    if options.output:
        with open(options.output, "w") as f:
            f.write(unified_diff)
    else:
        print(unified_diff)

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
