#!/usr/bin/env vpython3
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# [VPYTHON:BEGIN]
# python_version: "3.8"
# [VPYTHON:END]
"""Bazel launcher wrapper.

This script starts Bazel appropriate for the project you're working in.  It's
currently used by ChromiumOS, but is intended for use and to be updated by any
depot_tools users who are using Bazel.

In the case this script is not able to detect which project you're working in,
it will fall back to using the next "bazel" executable in your PATH.
"""

import itertools
import os
from pathlib import Path
import shutil
import sys
from typing import List, Optional


def _find_bazel_cros() -> Optional[Path]:
    """Find the bazel launcher for ChromiumOS."""
    cwd = Path.cwd()
    for parent in itertools.chain([cwd], cwd.parents):
        bazel_launcher = parent / "chromite" / "bin" / "bazel"
        if bazel_launcher.exists():
            return bazel_launcher
    return None


def _find_next_bazel_in_path() -> Optional[Path]:
    """The fallback method: search the remainder of PATH for bazel."""
    # Remove depot_tools from PATH if present.
    depot_tools = Path(__file__).resolve().parent
    path_env = os.environ.get("PATH", os.defpath)
    search_paths = []
    for path in path_env.split(os.pathsep):
        if Path(path).resolve() != depot_tools:
            search_paths.append(path)
    new_path_env = os.pathsep.join(search_paths)
    bazel = shutil.which("bazel", path=new_path_env)
    if bazel:
        return Path(bazel)
    return None


# All functions used to search for Bazel (in order of search).
_SEARCH_FUNCTIONS = (
    _find_bazel_cros,
    _find_next_bazel_in_path,
)

_FIND_FAILURE_MSG = """\
ERROR: The depot_tools bazel launcher was unable to find an appropriate bazel
executable to use.

For ChromiumOS developers:
  Make sure your current directory is inside a ChromiumOS checkout (e.g.,
  ~/chromiumos).  If you're already in a ChromiumOS checkout, it may be because
  you're working on a branch that's too old (i.e., prior to Bazel).

If you're not working on any of the above listed projects, this launcher assumes
that you have Bazel installed on your system somewhere else in PATH.  Check that
it's actually installed."""


def main(argv: List[str]) -> int:
    """Main."""
    for search_func in _SEARCH_FUNCTIONS:
        bazel = search_func()
        if bazel:
            os.execv(bazel, [str(bazel), *argv])

    print(_FIND_FAILURE_MSG, file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
