#! /usr/bin/env vpython3
#
# Copyright 2023 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# angle_android_test_runner.py:
#   Runs (a subset of) ANGLE tests on Android using android_helper
#   instead of chromium test_runner scripts. This script is integrated into
#   gn builds via the android_test_runner_script var, which creates wrappers
#   (out/<config>/angle_trace_tests) calling into this script
#   with additional args like --output-directory=out/<config> etc

import argparse
import json
import logging
import os
import pathlib
import subprocess
import sys

PY_UTILS = str(pathlib.Path(__file__).resolve().parent / 'py_utils')
if PY_UTILS not in sys.path:
    os.stat(PY_UTILS) and sys.path.insert(0, PY_UTILS)
import android_helper
import angle_path_util
import angle_test_util


def AddCommonParserArgs(parser):
    parser.add_argument(
        '--suite',
        help='Test suite to run.',
        required=True,
        choices=[
            'angle_end2end_tests',
            'angle_perftests',
            'angle_trace_tests',
            'angle_unittests',
            # dEQP - grep \"angle_deqp infra/specs/gn_isolate_map.pyl
            'angle_deqp_egl_tests',
            'angle_deqp_gles2_tests',
            'angle_deqp_gles31_rotate180_tests',
            'angle_deqp_gles31_rotate270_tests',
            'angle_deqp_gles31_rotate90_tests',
            'angle_deqp_gles31_tests',
            'angle_deqp_gles3_rotate180_tests',
            'angle_deqp_gles3_rotate270_tests',
            'angle_deqp_gles3_rotate90_tests',
            'angle_deqp_gles3_tests',
            'angle_deqp_khr_gles2_tests',
            'angle_deqp_khr_gles31_tests',
            'angle_deqp_khr_gles32_tests',
            'angle_deqp_khr_gles3_tests',
            'angle_deqp_khr_noctx_gles2_tests',
            'angle_deqp_khr_noctx_gles32_tests',
            'angle_deqp_khr_single_gles32_tests',
        ])
    parser.add_argument('-l', '--log', help='Logging level.', default='info')
    parser.add_argument('--list-tests', help='List tests.', action='store_true')
    parser.add_argument(
        '-f',
        '--filter',
        '--isolated-script-test-filter',
        '--gtest_filter',
        type=str,
        help='Test filter.')
    parser.add_argument(
        '--prepare-only',
        help='Only prepare traces, but do not actually run them.',
        action='store_true')


def RunWithAngleTestRunner(args, extra_args):
    angle_test_util.SetupLogging(args.log.upper())

    android_helper.Initialize(args.suite)
    assert android_helper.IsAndroid()

    rc, output, _ = android_helper.RunTests(
        args.suite, ['--list-tests', '--verbose'] + extra_args, log_output=False)
    if rc != 0:
        logging.fatal('Could not find test list from test output:\n%s' % output)
        return rc

    tests = angle_test_util.GetTestsFromOutput(output)
    if not tests:
        logging.fatal('Could not find test list from test output:\n%s' % output)
        return 1

    if args.filter:
        tests = angle_test_util.FilterTests(tests, args.filter)

    if args.list_tests:
        print('\n'.join(['Tests list:'] + tests + ['End tests list.']))
        return 0

    if args.suite == 'angle_trace_tests':
        traces = set(android_helper.GetTraceFromTestName(test) for test in tests)
        android_helper.PrepareRestrictedTraces(traces)

        if args.prepare_only:
            print('Prepared traces: %s' % traces)
            return 0

    flags = ['--gtest_filter=' + args.filter] if args.filter else []
    return android_helper.RunTests(args.suite, flags + extra_args)[0]


def RunWithChromiumTestRunner():
    # Import only here to avoid breaking trace bundles and other catapult imports
    angle_path_util.AddDepsDirToPath('build/android')
    import test_runner

    # Workaround from test_runner's __main__
    exit_code = test_runner.main()
    if exit_code == test_runner.constants.INFRA_EXIT_CODE:
        sys.stdout.flush()
        sys.stderr.flush()
        os._exit(exit_code)
    return exit_code


def CheckDeviceScreenState():
    screen_state = subprocess.check_output(
        [android_helper.FindAdb(), 'shell',
         'dumpsys deviceidle | grep mScreen || true']).decode().split()
    expected_screen_state = ['mScreenOn=true', 'mScreenLocked=false']
    if not set(expected_screen_state).issubset(screen_state):
        raise Exception('Unexpected device screen state: %s; expected: %s' %
                        (screen_state, expected_screen_state))


def main():
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument('-h', '--help', action='store_true')
    parser.add_argument('test_type', choices=['gtest'])
    parser.add_argument('--output-directory', required=True)
    parser.add_argument('--wrapper-script-args')
    parser.add_argument('--runtime-deps-path')
    AddCommonParserArgs(parser)

    parser.add_argument(
        '--angle-test-runner',
        help='Use ANGLE test runner instead of Chromium/catapult',
        action='store_true')

    args, extra_args = parser.parse_known_args()

    if not args.help:
        CheckDeviceScreenState()

    if args.angle_test_runner or args.suite == 'angle_trace_tests':
        if args.help:
            parser.print_help()
        else:
            os.chdir(args.output_directory)
            return RunWithAngleTestRunner(args, extra_args)
    else:
        # Chromium runner parses args by itself, including --help
        return RunWithChromiumTestRunner()


if __name__ == "__main__":
    sys.exit(main())
