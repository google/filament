#! /usr/bin/env vpython3
#
# Copyright 2023 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

import argparse
import contextlib
import difflib
import json
import logging
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
import time

SCRIPT_DIR = str(pathlib.Path(__file__).resolve().parent)
PY_UTILS = str(pathlib.Path(SCRIPT_DIR) / '..' / 'py_utils')
if PY_UTILS not in sys.path:
    os.stat(PY_UTILS) and sys.path.insert(0, PY_UTILS)
import angle_test_util


@contextlib.contextmanager
def temporary_dir(prefix=''):
    path = tempfile.mkdtemp(prefix=prefix)
    try:
        yield path
    finally:
        logging.info("Removing temporary directory: %s" % path)
        shutil.rmtree(path)


def file_content(path):
    with open(path, 'rb') as f:
        content = f.read()

    if path.endswith('.json'):
        info = json.loads(content)
        info['TraceMetadata']['CaptureRevision'] = '<ignored>'
        return json.dumps(info, indent=2).encode()

    return content


def diff_files(path, expected_path):
    content = file_content(path)
    expected_content = file_content(expected_path)
    fn = os.path.basename(path)

    if content == expected_content:
        return False

    if fn.endswith('.angledata'):
        logging.error('Checks failed. Binary file contents mismatch: %s', fn)
        return True

    # Captured files are expected to have LF line endings.
    # Note that git's EOL conversion for these files is disabled via .gitattributes
    assert b'\r\n' not in content
    assert b'\r\n' not in expected_content

    diff = list(
        difflib.unified_diff(
            expected_content.decode().splitlines(),
            content.decode().splitlines(),
            fromfile=fn,
            tofile=fn,
        ))

    logging.error('Checks failed. Found diff in %s:\n%s\n', fn, '\n'.join(diff))
    return True


def run_test(test_name, overwrite_expected):
    with temporary_dir() as temp_dir:
        cmd = [angle_test_util.ExecutablePathInCurrentDir('angle_end2end_tests')]
        if angle_test_util.IsAndroid():
            cmd.append('--angle-test-runner')

        test_args = ['--gtest_filter=%s' % test_name, '--angle-per-test-capture-label']
        extra_env = {
            'ANGLE_CAPTURE_ENABLED': '1',
            'ANGLE_CAPTURE_FRAME_START': '2',
            'ANGLE_CAPTURE_FRAME_END': '5',
            'ANGLE_CAPTURE_OUT_DIR': temp_dir,
            'ANGLE_CAPTURE_COMPRESSION': '0',
        }
        subprocess.check_call(cmd + test_args, env={**os.environ.copy(), **extra_env})
        logging.info('Capture finished, comparing files')
        files = sorted(fn for fn in os.listdir(temp_dir))
        expected_dir = os.path.join(SCRIPT_DIR, 'expected')
        expected_files = sorted(fn for fn in os.listdir(expected_dir) if not fn.startswith('.'))

        if overwrite_expected:
            for f in expected_files:
                os.remove(os.path.join(expected_dir, f))
            shutil.copytree(temp_dir, expected_dir, dirs_exist_ok=True)
            return True

        if files != expected_files:
            logging.error(
                'Checks failed. Capture produced a different set of files: %s\nDiff:\n%s\n', files,
                '\n'.join(difflib.unified_diff(expected_files, files)))
            return False

        has_diffs = False
        for fn in files:
            has_diffs |= diff_files(os.path.join(temp_dir, fn), os.path.join(expected_dir, fn))

        return not has_diffs


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--isolated-script-test-output', type=str)
    parser.add_argument('--log', help='Logging level.', default='info')
    parser.add_argument(
        '--overwrite-expected', help='Overwrite contents of expected/', action='store_true')
    args, extra_flags = parser.parse_known_args()

    logging.basicConfig(level=args.log.upper())

    angle_test_util.Initialize('angle_end2end_tests')

    test_name = 'CapturedTest*/ES3_Vulkan'
    had_error = False
    try:
        if not run_test(test_name, args.overwrite_expected):
            had_error = True
            logging.error(
                'Found capture diffs. If diffs are expected, build angle_end2end_tests and run '
                '(cd out/<build>; ../../src/tests/capture_tests/capture_tests.py --overwrite-expected)'
            )
    except Exception as e:
        logging.exception(e)
        had_error = True

    if args.isolated_script_test_output:
        results = {
            'tests': {
                'capture_test': {}
            },
            'interrupted': False,
            'seconds_since_epoch': time.time(),
            'path_delimiter': '.',
            'version': 3,
            'num_failures_by_type': {
                'FAIL': 0,
                'PASS': 0,
                'SKIP': 0,
            },
        }
        result = 'FAIL' if had_error else 'PASS'
        results['tests']['capture_test'][test_name] = {'expected': 'PASS', 'actual': result}
        results['num_failures_by_type'][result] += 1

        with open(args.isolated_script_test_output, 'w') as f:
            f.write(json.dumps(results, indent=2))

    return 1 if had_error else 0


if __name__ == '__main__':
    sys.exit(main())
