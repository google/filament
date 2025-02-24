# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Quest for running a GTest in Swarming."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard.pinpoint.models.quest import run_performance_test

_DEFAULT_EXTRA_ARGS = ['--non-telemetry', 'true']


class RunGTest(run_performance_test.RunPerformanceTest):

  @classmethod
  def _ComputeCommand(cls, arguments):
    if 'target' not in arguments:
      raise ValueError('Missing "target" in arguments.')

    # Synthesize the command based on the target if we don't have the command
    # explicitly provided.
    command = arguments.get(
        'command', ['luci-auth', 'context', '--',
                    arguments.get('target')])

    # We'll assume that the relative directory in the isolate is consistent
    # with the Chromium layout, where the release outputs are being used. This
    # is now an input to the job too, so can be provided in a UI or when
    # requested through an API.
    relative_cwd = arguments.get('relative_cwd', 'out/Release')
    return relative_cwd, command

  @classmethod
  def _ExtraTestArgs(cls, arguments):
    extra_test_args = []

    test = arguments.get('test')
    if test:
      extra_test_args.append('--gtest_filter=' + test)

    extra_test_args.append('--gtest_repeat=1')
    benchmark = arguments.get('benchmark')
    if benchmark:
      extra_test_args.append('--gtest-benchmark-name')
      extra_test_args.append(benchmark)

    extra_test_args += _DEFAULT_EXTRA_ARGS
    extra_test_args += super(RunGTest, cls)._ExtraTestArgs(arguments)
    return extra_test_args
