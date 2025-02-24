# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.pinpoint.models.quest import run_lacros_telemetry_test
from dashboard.pinpoint.models.quest import run_performance_test
from dashboard.pinpoint.models.quest import run_telemetry_test
from dashboard.pinpoint.models.quest import run_test_test

_BASE_ARGUMENTS = {
    'configuration': 'some_configuration',
    'swarming_server': 'server',
    'dimensions': run_test_test.DIMENSIONS,
    'benchmark': 'some_benchmark',
    'browser': 'lacros-chrome',
    'builder': 'builder name',
    'target': 'performance_test_suite_eve',
}
_COMBINED_DEFAULT_EXTRA_ARGS = (
    run_telemetry_test._DEFAULT_EXTRA_ARGS +
    run_performance_test._DEFAULT_EXTRA_ARGS)
_BASE_EXTRA_ARGS = [
    '-d',
    '--benchmarks',
    'some_benchmark',
    '--pageset-repeat',
    '1',
    '--browser',
    'lacros-chrome',
] + _COMBINED_DEFAULT_EXTRA_ARGS
_TELEMETRY_COMMAND = [
    'luci-auth', 'context', '--', 'vpython3',
    'bin/run_performance_test_suite_eve',
    '--remote=variable_chromeos_device_hostname'
]
_BASE_SWARMING_TAGS = {}


class FromDictTest(unittest.TestCase):

  def testMinimumArgumentsEve(self):
    quest = run_lacros_telemetry_test.RunLacrosTelemetryTest.FromDict(
        _BASE_ARGUMENTS)
    expected = run_lacros_telemetry_test.RunLacrosTelemetryTest(
        'server', run_test_test.DIMENSIONS, _BASE_EXTRA_ARGS,
        _BASE_SWARMING_TAGS, _TELEMETRY_COMMAND, 'out/Release')
    self.assertEqual(quest, expected)
