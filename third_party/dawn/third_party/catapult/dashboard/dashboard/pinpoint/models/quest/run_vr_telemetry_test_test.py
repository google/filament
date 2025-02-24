# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.pinpoint.models.quest import run_performance_test
from dashboard.pinpoint.models.quest import run_telemetry_test
from dashboard.pinpoint.models.quest import run_vr_telemetry_test
from dashboard.pinpoint.models.quest import run_test_test

_BASE_ARGUMENTS = {
    'swarming_server': 'server',
    'dimensions': run_test_test.DIMENSIONS,
    'browser': 'android-chromium-bundle',
}
_BROWSING_ARGUMENTS = _BASE_ARGUMENTS.copy()
_BROWSING_ARGUMENTS['benchmark'] = 'xr.browsing.static'
_CARDBOARD_ARGUMENTS = _BASE_ARGUMENTS.copy()
_CARDBOARD_ARGUMENTS['benchmark'] = 'xr.webxr.static'

_COMBINED_DEFAULT_EXTRA_ARGS = (
    run_telemetry_test._DEFAULT_EXTRA_ARGS +
    run_performance_test._DEFAULT_EXTRA_ARGS)

_BASE_EXTRA_ARGS = [
    '--pageset-repeat',
    '1',
    '--browser',
    'android-chromium-bundle',
] + _COMBINED_DEFAULT_EXTRA_ARGS
_BROWSING_EXTRA_ARGS = [
    '--install-bundle-module', 'vr', '--remove-system-vrcore',
    '--shared-prefs-file', run_vr_telemetry_test.DAYDREAM_PREFS,
    '--profile-dir', run_vr_telemetry_test.ASSET_PROFILE_PATH, '-d',
    '--benchmarks', 'xr.browsing.static'
] + _BASE_EXTRA_ARGS
_CARDBOARD_EXTRA_ARGS = [
    '--install-bundle-module', 'vr', '--remove-system-vrcore',
    '--shared-prefs-file', run_vr_telemetry_test.CARDBOARD_PREFS, '-d',
    '--benchmarks', 'xr.webxr.static'
] + _BASE_EXTRA_ARGS

_TELEMETRY_COMMAND = [
    'luci-auth', 'context', '--', 'vpython3', '../../testing/test_env.py',
    '../../testing/scripts/run_performance_tests.py',
    '../../tools/perf/run_benchmark'
]
_BASE_SWARMING_TAGS = {}


class AndroidFromDictTest(unittest.TestCase):

  def testMinimumArgs(self):
    with self.assertRaises(TypeError):
      run_vr_telemetry_test.RunVrTelemetryTest.FromDict(_BASE_ARGUMENTS)

  def testNonBundle(self):
    with self.assertRaises(TypeError):
      arguments = dict(_CARDBOARD_ARGUMENTS)
      arguments['browser'] = 'android-chromium'
      run_vr_telemetry_test.RunVrTelemetryTest.FromDict(arguments)

  def testCardboardArgs(self):
    quest = run_vr_telemetry_test.RunVrTelemetryTest.FromDict(
        _CARDBOARD_ARGUMENTS)
    expected = run_vr_telemetry_test.RunVrTelemetryTest(
        'server', run_test_test.DIMENSIONS, _CARDBOARD_EXTRA_ARGS,
        _BASE_SWARMING_TAGS, _TELEMETRY_COMMAND, 'out/Release')
    self.assertEqual(quest, expected)

  def testBrowsingArgs(self):
    quest = run_vr_telemetry_test.RunVrTelemetryTest.FromDict(
        _BROWSING_ARGUMENTS)
    expected = run_vr_telemetry_test.RunVrTelemetryTest(
        'server', run_test_test.DIMENSIONS, _BROWSING_EXTRA_ARGS,
        _BASE_SWARMING_TAGS, _TELEMETRY_COMMAND, 'out/Release')
    self.assertEqual(quest, expected)


_BASE_WINDOWS_ARGUMENTS = {
    'swarming_server': 'server',
    'dimensions': run_test_test.DIMENSIONS,
    'browser': 'release',
    'benchmark': 'xr.webxr.static',
}
_WINDOWS_EXTRA_ARGS = [
    '-d',
    '--benchmarks',
    'xr.webxr.static',
    '--pageset-repeat',
    '1',
    '--browser',
    'release',
] + _COMBINED_DEFAULT_EXTRA_ARGS


class WindowsFromDictTest(unittest.TestCase):

  def testMinimumArgs(self):
    quest = run_vr_telemetry_test.RunVrTelemetryTest.FromDict(
        _BASE_WINDOWS_ARGUMENTS)
    expected = run_vr_telemetry_test.RunVrTelemetryTest(
        'server', run_test_test.DIMENSIONS, _WINDOWS_EXTRA_ARGS,
        _BASE_SWARMING_TAGS, _TELEMETRY_COMMAND, 'out/Release')
    self.assertEqual(quest, expected)

  def testContentShell(self):
    with self.assertRaises(TypeError):
      arguments = dict(_BASE_WINDOWS_ARGUMENTS)
      arguments['browser'] = 'content-shell-release'
      run_vr_telemetry_test.RunVrTelemetryTest.FromDict(arguments)
