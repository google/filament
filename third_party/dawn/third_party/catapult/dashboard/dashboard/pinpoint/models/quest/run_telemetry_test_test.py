# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest
from unittest import mock

from dashboard.pinpoint.models.change import change as change_module
from dashboard.pinpoint.models.change import commit
from dashboard.pinpoint.models.quest import run_performance_test
from dashboard.pinpoint.models.quest import run_telemetry_test
from dashboard.pinpoint.models.quest import run_test_test

_BASE_ARGUMENTS = {
    'swarming_server': 'server',
    'dimensions': run_test_test.DIMENSIONS,
    'benchmark': 'speedometer',
    'browser': 'release',
}
_COMBINED_DEFAULT_EXTRA_ARGS = (
    run_telemetry_test._DEFAULT_EXTRA_ARGS +
    run_performance_test._DEFAULT_EXTRA_ARGS)
_BASE_EXTRA_ARGS = [
    '-d',
    '--benchmarks',
    'speedometer',
    '--pageset-repeat',
    '1',
    '--browser',
    'release',
] + _COMBINED_DEFAULT_EXTRA_ARGS
_TELEMETRY_COMMAND = [
    'luci-auth', 'context', '--', 'vpython3', '../../testing/test_env.py',
    '../../testing/scripts/run_performance_tests.py',
    '../../tools/perf/run_benchmark'
]
_BASE_SWARMING_TAGS = {}


class StartTest(unittest.TestCase):

  def testStart(self):
    quest = run_telemetry_test.RunTelemetryTest('server',
                                                run_test_test.DIMENSIONS,
                                                ['arg'], _BASE_SWARMING_TAGS,
                                                _TELEMETRY_COMMAND,
                                                'out/Release')
    change = mock.MagicMock(spec=change_module.Change)
    change.base_commit = mock.MagicMock(spec=commit.Commit)
    change.base_commit.AsDict = mock.MagicMock(
        return_value={'commit_position': 999999})
    execution = quest.Start(change, 'https://isolate.server', 'isolate hash')
    self.assertEqual(execution._extra_args,
                     ['arg', '--results-label', mock.ANY])
    self.assertIn('vpython3', execution.command)

  def testSwarmingTags(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['browser'] = 'android-webview'
    quest = run_telemetry_test.RunTelemetryTest.FromDict(arguments)
    change = mock.MagicMock(spec=change_module.Change)
    change.base_commit = mock.MagicMock(spec=commit.Commit)
    change.base_commit.AsDict = mock.MagicMock(
        return_value={'commit_position': 675460})
    execution = quest.Start(change, 'https://isolate.server', 'isolate hash')
    self.assertEqual(execution._swarming_tags, {
        'benchmark': 'speedometer',
        'change': str(change),
        'hasfilter': '0'
    })

  def testSwarmingTagsWithStoryFilter(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['browser'] = 'android-webview'
    arguments['story'] = 'sfilter'
    quest = run_telemetry_test.RunTelemetryTest.FromDict(arguments)
    change = mock.MagicMock(spec=change_module.Change)
    change.base_commit = mock.MagicMock(spec=commit.Commit)
    change.base_commit.AsDict = mock.MagicMock(
        return_value={'commit_position': 675460})
    execution = quest.Start(change, 'https://isolate.server', 'isolate hash')
    self.assertEqual(
        execution._swarming_tags, {
            'benchmark': 'speedometer',
            'change': str(change),
            'hasfilter': '1',
            'storyfilter': 'sfilter'
        })

  def testExtraArgsInChange(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['browser'] = 'android-webview'
    arguments['story'] = 'sfilter'
    quest = run_telemetry_test.RunTelemetryTest.FromDict(arguments)
    change = mock.MagicMock(spec=change_module.Change)
    change.base_commit = mock.MagicMock(spec=commit.Commit)
    change.base_commit.AsDict = mock.MagicMock(
        return_value={'commit_position': 675460})
    change.change_label = mock.MagicMock(return_value='base')
    type(change).change_args = mock.PropertyMock(
        return_value=['--extra-browser-args'])
    with mock.patch(
        'dashboard.pinpoint.models.quest.run_test.RunTest._Start',
        wraps=quest._Start) as internal_start:
      execution = quest.Start(change, 'https://isolate.server', 'isolate hash')
      call_args = internal_start.call_args[0][3]
      self.assertIn('--extra-browser-args', call_args)
    self.assertEqual(
        execution._swarming_tags, {
            'benchmark': 'speedometer',
            'change': str(change),
            'hasfilter': '1',
            'storyfilter': 'sfilter'
        })

  def testSwarmingTagsWithStoryFilter_RevMissingCommitPosition(self):
    """Reproduce crbug/1051943."""
    arguments = dict(_BASE_ARGUMENTS)
    arguments['browser'] = 'android-webview'
    arguments['story'] = 'sfilter'
    quest = run_telemetry_test.RunTelemetryTest.FromDict(arguments)
    change = mock.MagicMock(spec=change_module.Change)
    change.base_commit = mock.MagicMock(spec=commit.Commit)
    # No 'commit_position' property in base_commit.
    change.base_commit.AsDict = mock.MagicMock(return_value={})
    with mock.patch(
        'dashboard.pinpoint.models.quest.run_test.RunTest._Start',
        wraps=quest._Start):
      quest.Start(change, 'https://isolate.server', 'isolate hash')

  def testSwarmingTagsWithStoryTagFilter(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['browser'] = 'android-webview'
    arguments['story_tags'] = 'tfilter'
    quest = run_telemetry_test.RunTelemetryTest.FromDict(arguments)
    change = mock.MagicMock(spec=change_module.Change)
    change.base_commit = mock.MagicMock(spec=commit.Commit)
    change.base_commit.AsDict = mock.MagicMock(
        return_value={'commit_position': 675460})
    execution = quest.Start(change, 'https://isolate.server', 'isolate hash')
    self.assertEqual(
        execution._swarming_tags, {
            'benchmark': 'speedometer',
            'change': str(change),
            'hasfilter': '1',
            'tagfilter': 'tfilter'
        })


class FromDictTest(unittest.TestCase):

  def testMinimumArguments(self):
    quest = run_telemetry_test.RunTelemetryTest.FromDict(_BASE_ARGUMENTS)
    expected = run_telemetry_test.RunTelemetryTest(
        'server', run_test_test.DIMENSIONS, _BASE_EXTRA_ARGS,
        _BASE_SWARMING_TAGS, _TELEMETRY_COMMAND, 'out/Release')
    self.assertEqual(quest, expected)

  def testAllArguments(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['story'] = 'http://www.fifa.com/'
    arguments['story_tags'] = 'tag1,tag2'
    quest = run_telemetry_test.RunTelemetryTest.FromDict(arguments)

    extra_args = [
        '--benchmarks',
        'speedometer',
        '--story-filter',
        '^http...www.fifa.com.$',
        '--story-tag-filter',
        'tag1,tag2',
        '--pageset-repeat',
        '1',
        '--browser',
        'release',
    ] + _COMBINED_DEFAULT_EXTRA_ARGS
    expected = run_telemetry_test.RunTelemetryTest(
        'server', run_test_test.DIMENSIONS, extra_args, _BASE_SWARMING_TAGS,
        _TELEMETRY_COMMAND, 'out/Release')
    self.assertEqual(quest, expected)

  def testMissingBenchmark(self):
    arguments = dict(_BASE_ARGUMENTS)
    del arguments['benchmark']
    with self.assertRaises(TypeError):
      run_telemetry_test.RunTelemetryTest.FromDict(arguments)

  def testMissingBrowser(self):
    arguments = dict(_BASE_ARGUMENTS)
    del arguments['browser']
    with self.assertRaises(TypeError):
      run_telemetry_test.RunTelemetryTest.FromDict(arguments)

  def testWebview(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['browser'] = 'android-webview'
    quest = run_telemetry_test.RunTelemetryTest.FromDict(arguments)

    extra_args = [
        '-d',
        '--benchmarks',
        'speedometer',
        '--pageset-repeat',
        '1',
        '--browser',
        'android-webview',
    ] + _COMBINED_DEFAULT_EXTRA_ARGS
    expected = run_telemetry_test.RunTelemetryTest(
        'server', run_test_test.DIMENSIONS, extra_args, _BASE_SWARMING_TAGS,
        _TELEMETRY_COMMAND, 'out/Release')
    self.assertEqual(quest, expected)

  def testCrossbench(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['benchmark'] = 'speedometer3.crossbench'
    quest = run_telemetry_test.RunTelemetryTest.FromDict(arguments)

    extra_args = [
        '--benchmark-display-name=speedometer3.crossbench',
        '--benchmarks=speedometer_3.0',
        '--browser=release',
    ] + run_performance_test._DEFAULT_EXTRA_ARGS
    expected = run_telemetry_test.RunTelemetryTest(
        'server', run_test_test.DIMENSIONS, extra_args, _BASE_SWARMING_TAGS,
        _TELEMETRY_COMMAND[:-1] + ['../../third_party/crossbench/cb.py'],
        'out/Release')
    self.assertEqual(quest, expected)
