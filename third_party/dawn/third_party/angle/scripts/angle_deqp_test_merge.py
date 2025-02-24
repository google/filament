#!/usr/bin/env python
#
# Copyright 2021 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
""" Merges dEQP sharded test results in the ANGLE testing infrastucture."""

import os
import sys
if sys.version_info.major != 3 and __name__ == '__main__':
    # Swarming prepends sys.executable so we get python2 regardless of shebang.
    # Spawn itself with vpython3 instead.
    import subprocess
    sys.exit(subprocess.call(['vpython3', os.path.realpath(__file__)] + sys.argv[1:]))

import pathlib  # python3

PY_UTILS = str(pathlib.Path(__file__).resolve().parents[1] / 'src' / 'tests' / 'py_utils')
if PY_UTILS not in sys.path:
    os.stat(PY_UTILS) and sys.path.insert(0, PY_UTILS)
import angle_path_util

angle_path_util.AddDepsDirToPath('testing/merge_scripts')
import merge_api
import standard_isolated_script_merge


def main(raw_args):

    parser = merge_api.ArgumentParser()
    args = parser.parse_args(raw_args)

    # TODO(jmadill): Merge QPA files into one. http://anglebug.com/42263789

    return standard_isolated_script_merge.StandardIsolatedScriptMerge(
        args.output_json, args.summary_json, args.jsons_to_merge)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
