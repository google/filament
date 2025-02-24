# Copyright 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""This provides an easy way to access args.gn."""

import os
import re


def _find_root(output_dir):
    curdir = output_dir
    while True:
        if os.path.exists(os.path.join(curdir, ".gn")):
            return curdir
        nextdir = os.path.join(curdir, "..")
        if os.path.abspath(curdir) == os.path.abspath(nextdir):
            raise Exception(
                'Could not find checkout in any parent of the current path.')
        curdir = nextdir


def _gn_lines(output_dir, path):
    """
    Generator function that returns args.gn lines one at a time, following
    import directives as needed.
    """
    import_re = re.compile(r'\s*import\("(.*)"\)')
    with open(path, encoding="utf-8") as f:
        for line in f:
            match = import_re.match(line)
            if match:
                raw_import_path = match.groups()[0]
                if raw_import_path[:2] == "//":
                    import_path = os.path.normpath(
                        os.path.join(_find_root(output_dir),
                                     raw_import_path[2:]))
                else:
                    import_path = os.path.normpath(
                        os.path.join(os.path.dirname(path), raw_import_path))
                yield from _gn_lines(output_dir, import_path)
            else:
                yield line


def _path(output_dir):
    return os.path.join(output_dir, "args.gn")


def exists(output_dir):
    """Checks args.gn exists in output_dir."""
    return os.path.exists(_path(output_dir))


def lines(output_dir):
    """Generator of args.gn lines. comment is removed."""
    if not exists(output_dir):
        return
    for line in _gn_lines(output_dir, _path(output_dir)):
        line_without_comment = line.split("#")[0]
        yield line_without_comment


_gn_arg_pattern = re.compile(r"(^|\s*)([^=\s]*)\s*=\s*(\S*)\s*$")


def args(output_dir):
    """Generator of args.gn's key,value pair."""
    for line in lines(output_dir):
        m = _gn_arg_pattern.match(line)
        if not m:
            continue
        yield (m.group(2), m.group(3))
