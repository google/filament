# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.pinpoint.models.quest import run_web_engine_telemetry_test
from dashboard.pinpoint.models.quest import run_performance_test
from dashboard.pinpoint.models.quest import run_telemetry_test
from dashboard.pinpoint.models.quest import run_test_test

_BASE_ARGUMENTS = {
    'configuration': 'some_configuration',
    'swarming_server': 'server',
    'dimensions': run_test_test.DIMENSIONS,
    'benchmark': 'some_benchmark',
    'browser': 'web-engine-shell',
    'builder': 'builder name',
    'target': 'performance_web_engine_test_suite',
}
_COMBINED_DEFAULT_EXTRA_ARGS = (
    run_telemetry_test._DEFAULT_EXTRA_ARGS +
    run_performance_test._DEFAULT_EXTRA_ARGS +
    run_web_engine_telemetry_test._DEFAULT_EXTRA_ARGS)
_BASE_EXTRA_ARGS = [
    '-d',
    '--benchmarks',
    'some_benchmark',
    '--pageset-repeat',
    '1',
    '--browser',
    'web-engine-shell',
] + _COMBINED_DEFAULT_EXTRA_ARGS
_TELEMETRY_COMMAND = [
    'luci-auth',
    'context',
    '--',
    'vpython3',
    '../../testing/test_env.py',
    '../../testing/scripts/run_performance_tests.py',
    '../../content/test/gpu/run_telemetry_benchmark_fuchsia.py',
]
_BASE_SWARMING_TAGS = {}


class FromDictTest(unittest.TestCase):

  def testMinimumArgumentsWebEngine(self):
    quest = run_web_engine_telemetry_test.RunWebEngineTelemetryTest.FromDict(
        _BASE_ARGUMENTS)
    expected = run_web_engine_telemetry_test.RunWebEngineTelemetryTest(
        'server', run_test_test.DIMENSIONS, _BASE_EXTRA_ARGS,
        _BASE_SWARMING_TAGS, _TELEMETRY_COMMAND, 'out/Release')
    self.assertEqual(quest, expected)

  def testSettingDeviceTypeCorrectlySetsImageDir(self):
    platforms = (
        list(run_web_engine_telemetry_test.IMAGE_MAP.keys()) +
        list(run_web_engine_telemetry_test.PB_IMAGE_MAP.keys()))
    for platform in platforms:
      # Set up new dimensions.
      new_args = dict(_BASE_ARGUMENTS)
      dimensions = run_test_test.DIMENSIONS[:]
      dimensions.append({'key': 'device_type', 'value': platform})
      new_args['dimensions'] = dimensions

      quest = run_web_engine_telemetry_test.RunWebEngineTelemetryTest.FromDict(
          new_args)

      # Asserts image dir is found.
      system_image_flag = [
          arg for arg in quest._extra_args if 'system-image-dir' in arg
      ]
      self.assertTrue(system_image_flag)

      # Assert components from IMAGE_MAP are found in the path.
      extra_args = _BASE_EXTRA_ARGS[:]
      if platform in run_web_engine_telemetry_test.IMAGE_MAP:
        path_parts = run_web_engine_telemetry_test.IMAGE_MAP[platform]
        extra_args.append(run_web_engine_telemetry_test.IMAGE_FLAG +
                          run_web_engine_telemetry_test.DEFAULT_IMAGE_PATH %
                          path_parts)
      elif platform in run_web_engine_telemetry_test.PB_IMAGE_MAP:
        path_parts = (run_web_engine_telemetry_test.PB_IMAGE_MAP[platform],)
        extra_args.append(run_web_engine_telemetry_test.IMAGE_FLAG +
                          path_parts[0])
      for path_part in path_parts:
        self.assertIn(path_part, system_image_flag[0])

      expected = run_web_engine_telemetry_test.RunWebEngineTelemetryTest(
          'server', dimensions, extra_args, _BASE_SWARMING_TAGS,
          _TELEMETRY_COMMAND, 'out/Release')
      self.assertEqual(quest, expected)
