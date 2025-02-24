# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Quest for running an Android instrumentation test in Swarming."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard.pinpoint.models.quest import run_test

_DEFAULT_EXTRA_ARGS = ['--recover-devices']


class RunInstrumentationTest(run_test.RunTest):

  @classmethod
  def _ComputeCommand(cls, arguments):
    target = arguments.get('target')
    if not target:
      raise ValueError('Missing "target" in arguments.')
    command = arguments.get('command', ['luci-auth', 'context', '--', target])
    relative_cwd = arguments.get('relative_cwd', 'out/Release')
    return relative_cwd, command

  @classmethod
  def _ExtraTestArgs(cls, arguments):
    extra_test_args = []

    test_filter = arguments.get('test-filter')
    if test_filter:
      extra_test_args += ('--test-filter', test_filter)

    extra_test_args += _DEFAULT_EXTRA_ARGS
    extra_test_args += super(RunInstrumentationTest,
                             cls)._ExtraTestArgs(arguments)
    return extra_test_args
