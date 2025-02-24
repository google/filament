#! /usr/bin/env python3
#
# Copyright 2022 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# run_angle_android_test.py:
#   Runs ANGLE tests using android_helper wrapper. Example:
#     (cd out/Android; ../../src/tests/run_angle_android_test.py \
#       --suite=angle_trace_tests --filter='*among_us' \
#       --verbose --fixed-test-time-with-warmup=10)

import argparse
import logging
import os
import pathlib
import sys

import angle_android_test_runner


def main():
    parser = argparse.ArgumentParser()
    angle_android_test_runner.AddCommonParserArgs(parser)

    args, extra_args = parser.parse_known_args()

    return angle_android_test_runner.RunWithAngleTestRunner(args, extra_args)


if __name__ == '__main__':
    sys.exit(main())
