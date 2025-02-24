# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import os
import unittest

from dashboard.pinpoint.models.quest import run_webrtc_test
from dashboard.pinpoint.models.quest import run_test_test

_BASE_ARGUMENTS = {
    'configuration': 'some_configuration',
    'swarming_server': 'server',
    'dimensions': run_test_test.DIMENSIONS,
    'benchmark': 'some_benchmark',
    'builder': 'builder name',
    'target': 'foo_test',
}
_BASE_EXTRA_ARGS = [
    '--nologs',
    '--isolated-script-test-perf-output='
    '${ISOLATED_OUTDIR}/webrtc_perf_tests/perf_results.json',
]
_WEBRTCTEST_COMMAND = [
    'vpython3',
    '../../tools_webrtc/flags_compatibility.py',
    '../../testing/test_env.py',
    os.path.join('.', 'foo_test'),
    '--test_artifacts_dir=${ISOLATED_OUTDIR}',
]
_BASE_SWARMING_TAGS = {}


class FromDictTest(unittest.TestCase):

  def testMinimumArguments(self):
    quest = run_webrtc_test.RunWebRtcTest.FromDict(_BASE_ARGUMENTS)
    expected = run_webrtc_test.RunWebRtcTest('server', run_test_test.DIMENSIONS,
                                             _BASE_EXTRA_ARGS,
                                             _BASE_SWARMING_TAGS,
                                             _WEBRTCTEST_COMMAND,
                                             'out/builder_name')
    self.assertEqual(quest.command, expected.command)
    self.assertEqual(quest.relative_cwd, expected.relative_cwd)
    self.assertEqual(quest, expected)

  def testAndroidConfiguration(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['configuration'] = '__android__'
    webrtc_android_command = [
        'vpython3',
        '../../build/android/test_wrapper/logdog_wrapper.py',
        '--target',
        'foo_test',
        '--logdog-bin-cmd',
        '../../bin/logdog_butler',
        '--logcat-output-file',
        '${ISOLATED_OUTDIR}/logcats',
    ]
    quest = run_webrtc_test.RunWebRtcTest.FromDict(arguments)
    expected = run_webrtc_test.RunWebRtcTest('server', run_test_test.DIMENSIONS,
                                             _BASE_EXTRA_ARGS,
                                             _BASE_SWARMING_TAGS,
                                             webrtc_android_command,
                                             'out/builder_name')
    self.assertEqual(quest.command, expected.command)
    self.assertEqual(quest.relative_cwd, expected.relative_cwd)
    self.assertEqual(quest, expected)

  def testGtestFilterEndingWithBob(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['story'] = 'foo_story_bob'
    extra_args = _BASE_EXTRA_ARGS + ['--gtest_filter=*.Foo_Story*']
    quest = run_webrtc_test.RunWebRtcTest.FromDict(arguments)
    expected = run_webrtc_test.RunWebRtcTest('server', run_test_test.DIMENSIONS,
                                             extra_args, _BASE_SWARMING_TAGS,
                                             _WEBRTCTEST_COMMAND,
                                             'out/builder_name')
    self.assertEqual(quest.command, expected.command)
    self.assertEqual(quest.relative_cwd, expected.relative_cwd)
    self.assertEqual(quest, expected)

  def testGtestFilterWithMidWordUppercase(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['story'] = 'fOO__stOry'
    extra_args = _BASE_EXTRA_ARGS + ['--gtest_filter=*.FOO__StOry*']
    quest = run_webrtc_test.RunWebRtcTest.FromDict(arguments)
    expected = run_webrtc_test.RunWebRtcTest('server', run_test_test.DIMENSIONS,
                                             extra_args, _BASE_SWARMING_TAGS,
                                             _WEBRTCTEST_COMMAND,
                                             'out/builder_name')
    self.assertEqual(quest.command, expected.command)
    self.assertEqual(quest.relative_cwd, expected.relative_cwd)
    self.assertEqual(quest, expected)

  def testGtestFilterRampUp(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['story'] = 'rampdown'
    extra_args = _BASE_EXTRA_ARGS + ['--gtest_filter=RampUpTest.*']
    quest = run_webrtc_test.RunWebRtcTest.FromDict(arguments)
    expected = run_webrtc_test.RunWebRtcTest('server', run_test_test.DIMENSIONS,
                                             extra_args, _BASE_SWARMING_TAGS,
                                             _WEBRTCTEST_COMMAND,
                                             'out/builder_name')
    self.assertEqual(quest.command, expected.command)
    self.assertEqual(quest.relative_cwd, expected.relative_cwd)
    self.assertEqual(quest, expected)


class StartTest(unittest.TestCase):

  def testStart(self):
    quest = run_webrtc_test.RunWebRtcTest('server', run_test_test.DIMENSIONS,
                                          _BASE_EXTRA_ARGS, _BASE_SWARMING_TAGS,
                                          _WEBRTCTEST_COMMAND,
                                          'out/builder_name')
    execution = quest.Start('change', 'https://isolate.server', 'isolate hash')
    self.assertEqual(execution._execution_timeout_secs, 10800)
