# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import json
import logging
import os
import shutil
import sys
import tempfile
import unittest
from unittest import mock
import six

from py_utils import cloud_storage
from py_utils.constants import exit_codes

from telemetry import benchmark
from telemetry.core import exceptions
from telemetry.core import util
from telemetry.internal.actions import page_action
from telemetry.internal.results import page_test_results
from telemetry.internal.results import results_options
from telemetry.internal import story_runner
from telemetry.page import legacy_page_test
from telemetry import story as story_module
from telemetry.story import story_filter
from telemetry.testing import fakes
from telemetry.testing import options_for_unittests
from telemetry.testing import test_stories
from telemetry.web_perf import story_test
from telemetry.wpr import archive_info


class RunStorySetTest(unittest.TestCase):
  """Tests that run dummy story sets with a mock StoryTest.

  The main entry point for these tests is story_runner.RunStorySet.
  """
  def setUp(self):
    self.options = options_for_unittests.GetRunOptions(
        output_dir=tempfile.mkdtemp())
    # We use a mock platform and story set, so tests can inspect which methods
    # were called and easily override their behavior.
    self.mock_platform = test_stories.TestSharedState.mock_platform
    self.mock_story_test = mock.Mock(spec=story_test.StoryTest)

  def tearDown(self):
    shutil.rmtree(self.options.output_dir)

  def RunStories(self, stories, **kwargs):
    story_set = test_stories.DummyStorySet(stories)
    with results_options.CreateResults(
        self.options, benchmark_name='benchmark') as results:
      story_runner.RunStorySet(
          self.mock_story_test, story_set, self.options, results, **kwargs)

  def ReadTestResults(self):
    return results_options.ReadTestResults(self.options.intermediate_dir)

  def testRunStorySet(self):
    self.RunStories(['story1', 'story2', 'story3'])
    test_results = self.ReadTestResults()
    self.assertTrue(['PASS', 'PASS', 'PASS'],
                    [test['status'] for test in test_results])

  def testRunStoryWithLongName(self):
    with self.assertRaises(ValueError):
      self.RunStories(['l' * 182])

  def testCallOrderInStoryTest(self):
    """Check the call order of StoryTest methods is as expected."""
    self.RunStories(['foo', 'bar', 'baz'])
    self.assertEqual([call[0] for call in self.mock_story_test.mock_calls],
                     ['WillRunStory', 'Measure', 'DidRunStory'] * 3)

  @mock.patch.object(test_stories.TestSharedState, 'DidRunStory')
  @mock.patch.object(test_stories.TestSharedState, 'RunStory')
  @mock.patch.object(test_stories.TestSharedState, 'WillRunStory')
  def testCallOrderBetweenStoryTestAndSharedState(
      self, will_run_story, run_story, did_run_story):
    """Check the call order between StoryTest and SharedState is correct."""
    root_mock = mock.MagicMock()
    root_mock.attach_mock(self.mock_story_test, 'test')
    root_mock.attach_mock(will_run_story, 'state.WillRunStory')
    root_mock.attach_mock(run_story, 'state.RunStory')
    root_mock.attach_mock(did_run_story, 'state.DidRunStory')

    self.RunStories(['story1'])
    self.assertEqual([call[0] for call in root_mock.mock_calls], [
        'test.WillRunStory',
        'state.WillRunStory',
        'state.RunStory',
        'test.Measure',
        'test.DidRunStory',
        'state.DidRunStory'
    ])

  def testAppCrashExceptionCausesFailure(self):
    self.RunStories([test_stories.DummyStory(
        'story',
        run_side_effect=exceptions.AppCrashException(msg='App Foo crashes'))])
    test_results = self.ReadTestResults()
    self.assertEqual(['FAIL'],
                     [test['status'] for test in test_results])
    self.assertIn('App Foo crashes', sys.stderr.getvalue())

  @mock.patch.object(test_stories.TestSharedState, 'TearDownState')
  def testExceptionRaisedInSharedStateTearDown(self, tear_down_state):
    class TestOnlyException(Exception):
      pass

    tear_down_state.side_effect = TestOnlyException()
    with self.assertRaises(TestOnlyException):
      self.RunStories(['story'])

  def testUnknownExceptionIsNotFatal(self):
    class UnknownException(Exception):
      pass

    self.RunStories([
        test_stories.DummyStory(
            'foo', run_side_effect=UnknownException('FooException')),
        test_stories.DummyStory('bar')])
    test_results = self.ReadTestResults()
    self.assertEqual(['FAIL', 'PASS'],
                     [test['status'] for test in test_results])
    self.assertIn('FooException', sys.stderr.getvalue())

  def testRaiseBrowserGoneExceptionFromRunPage(self):
    self.RunStories([
        test_stories.DummyStory(
            'foo', run_side_effect=exceptions.BrowserGoneException(
                None, 'i am a browser crash message')),
        test_stories.DummyStory('bar')])
    test_results = self.ReadTestResults()
    self.assertEqual(['FAIL', 'PASS'],
                     [test['status'] for test in test_results])
    self.assertIn('i am a browser crash message', sys.stderr.getvalue())

  @mock.patch.object(test_stories.TestSharedState,
                     'DumpStateUponStoryRunFailure')
  @mock.patch.object(test_stories.TestSharedState, 'TearDownState')
  def testAppCrashThenRaiseInTearDown_Interrupted(
      self, tear_down_state, dump_state_upon_story_run_failure):
    class TearDownStateException(Exception):
      pass

    tear_down_state.side_effect = TearDownStateException()
    root_mock = mock.Mock()
    root_mock.attach_mock(tear_down_state, 'state.TearDownState')
    root_mock.attach_mock(dump_state_upon_story_run_failure,
                          'state.DumpStateUponStoryRunFailure')
    self.RunStories([
        test_stories.DummyStory(
            'foo', run_side_effect=exceptions.AppCrashException(msg='crash!')),
        test_stories.DummyStory('bar')])

    self.assertEqual([call[0] for call in root_mock.mock_calls], [
        'state.DumpStateUponStoryRunFailure',
        # This tear down happens because of the app crash.
        'state.TearDownState',
        # This one happens since state must be re-created to check whether
        # later stories should be skipped or unexpectedly skipped. Then
        # state is torn down normally at the end of the runs.
        'state.TearDownState'
    ])

    test_results = self.ReadTestResults()
    self.assertEqual(len(test_results), 2)
    # First story unexpectedly failed with AppCrashException.
    self.assertEqual(test_results[0]['status'], 'FAIL')
    self.assertFalse(test_results[0]['expected'])
    # Second story unexpectedly skipped due to exception during tear down.
    self.assertEqual(test_results[1]['status'], 'SKIP')
    self.assertFalse(test_results[1]['expected'])

  def testPagesetRepeat(self):
    self.options.pageset_repeat = 2
    self.RunStories(['story1', 'story2'])
    test_results = self.ReadTestResults()
    self.assertEqual(['benchmark/story1', 'benchmark/story2'] * 2,
                     [test['testPath'] for test in test_results])
    self.assertEqual(['PASS', 'PASS', 'PASS', 'PASS'],
                     [test['status'] for test in test_results])

  def _testMaxFailuresOptionIsRespectedAndOverridable(
      self, num_failing_stories, runner_max_failures, options_max_failures,
      expected_num_failures, expected_num_skips):
    if options_max_failures:
      self.options.max_failures = options_max_failures
    self.RunStories([
        test_stories.DummyStory(
            'failing_%d' % i, run_side_effect=Exception('boom!'))
        for i in range(num_failing_stories)
    ], max_failures=runner_max_failures)
    test_results = self.ReadTestResults()
    self.assertEqual(len(test_results),
                     expected_num_failures + expected_num_skips)
    for i, test in enumerate(test_results):
      expected_status = 'FAIL' if i < expected_num_failures else 'SKIP'
      self.assertEqual(test['status'], expected_status)

  def testMaxFailuresNotSpecified(self):
    self._testMaxFailuresOptionIsRespectedAndOverridable(
        num_failing_stories=5, runner_max_failures=None,
        options_max_failures=None, expected_num_failures=5,
        expected_num_skips=0)

  def testMaxFailuresSpecifiedToRun(self):
    # Runs up to max_failures+1 failing tests before stopping, since
    # every tests after max_failures failures have been encountered
    # may all be passing.
    self._testMaxFailuresOptionIsRespectedAndOverridable(
        num_failing_stories=5, runner_max_failures=3,
        options_max_failures=None, expected_num_failures=4,
        expected_num_skips=1)

  def testMaxFailuresOption(self):
    # Runs up to max_failures+1 failing tests before stopping, since
    # every tests after max_failures failures have been encountered
    # may all be passing.
    self._testMaxFailuresOptionIsRespectedAndOverridable(
        num_failing_stories=5, runner_max_failures=3,
        options_max_failures=1, expected_num_failures=2,
        expected_num_skips=3)


class UpdateAndCheckArchivesTest(unittest.TestCase):
  """Tests for the private _UpdateAndCheckArchives."""
  def setUp(self):
    mock.patch.object(archive_info.WprArchiveInfo,
                      'DownloadArchivesIfNeeded').start()
    self._mock_story_filter = mock.Mock()
    self._mock_story_filter.ShouldSkip.return_value = False

  def tearDown(self):
    mock.patch.stopall()

  def testMissingArchiveDataFile(self):
    story_set = test_stories.DummyStorySet(['story'])
    with self.assertRaises(story_runner.ArchiveError):
      story_runner._UpdateAndCheckArchives(
          story_set.archive_data_file, story_set.wpr_archive_info,
          story_set.stories, self._mock_story_filter)


  def testMissingArchiveDataFileWithSkippedStory(self):
    story_set = test_stories.DummyStorySet(['story'])
    self._mock_story_filter.ShouldSkip.return_value = True
    success = story_runner._UpdateAndCheckArchives(
        story_set.archive_data_file, story_set.wpr_archive_info,
        story_set.stories, self._mock_story_filter)
    self.assertTrue(success)

  def testArchiveDataFileDoesNotExist(self):
    story_set = test_stories.DummyStorySet(
        ['story'], archive_data_file='does_not_exist.json')
    with self.assertRaises(story_runner.ArchiveError):
      story_runner._UpdateAndCheckArchives(
          story_set.archive_data_file, story_set.wpr_archive_info,
          story_set.stories, self._mock_story_filter)

  def testUpdateAndCheckArchivesSuccess(self):
    # This test file has a recording for a 'http://www.testurl.com' story only.
    archive_data_file = os.path.join(
        util.GetUnittestDataDir(), 'archive_files', 'test.json')
    story_set = test_stories.DummyStorySet(
        ['http://www.testurl.com'], archive_data_file=archive_data_file)
    success = story_runner._UpdateAndCheckArchives(
        story_set.archive_data_file, story_set.wpr_archive_info,
        story_set.stories, self._mock_story_filter)
    self.assertTrue(success)

  def testArchiveWithMissingStory(self):
    # This test file has a recording for a 'http://www.testurl.com' story only.
    archive_data_file = os.path.join(
        util.GetUnittestDataDir(), 'archive_files', 'test.json')
    story_set = test_stories.DummyStorySet(
        ['http://www.testurl.com', 'http://www.google.com'],
        archive_data_file=archive_data_file)
    with self.assertRaises(story_runner.ArchiveError):
      story_runner._UpdateAndCheckArchives(
          story_set.archive_data_file, story_set.wpr_archive_info,
          story_set.stories, self._mock_story_filter)

  def testArchiveWithMissingWprFile(self):
    # This test file claims to have recordings for both
    # 'http://www.testurl.com' and 'http://www.google.com'; but the file with
    # the wpr recording for the later story is actually missing.
    archive_data_file = os.path.join(
        util.GetUnittestDataDir(), 'archive_files',
        'test_missing_wpr_file.json')
    story_set = test_stories.DummyStorySet(
        ['http://www.testurl.com', 'http://www.google.com'],
        archive_data_file=archive_data_file)
    with self.assertRaises(story_runner.ArchiveError):
      story_runner._UpdateAndCheckArchives(
          story_set.archive_data_file, story_set.wpr_archive_info,
          story_set.stories, self._mock_story_filter)


class RunStoryAndProcessErrorIfNeededTest(unittest.TestCase):
  """Tests for the private _RunStoryAndProcessErrorIfNeeded.

  All these tests:
  - Use mocks for all objects, including stories. No real browser is involved.
  - Call story_runner._RunStoryAndProcessErrorIfNeeded as entry point.
  """
  def setUp(self):
    self.finder_options = options_for_unittests.GetCopy()
    self.finder_options.periodic_screenshot_frequency_ms = None

  def _CreateErrorProcessingMock(self, method_exceptions=None,
                                 legacy_test=False):
    if legacy_test:
      test_class = legacy_page_test.LegacyPageTest
    else:
      test_class = story_test.StoryTest

    root_mock = mock.NonCallableMock(
        story=mock.NonCallableMagicMock(story_module.Story),
        results=mock.NonCallableMagicMock(page_test_results.PageTestResults),
        test=mock.NonCallableMagicMock(test_class),
        state=mock.NonCallableMagicMock(
            story_module.SharedState,
            CanRunStory=mock.Mock(return_value=True)))

    if method_exceptions:
      root_mock.configure_mock(**{
          path + '.side_effect': exception
          for path, exception in six.iteritems(method_exceptions)})

    return root_mock

  def testRunStoryAndProcessErrorIfNeeded_success(self):
    root_mock = self._CreateErrorProcessingMock()

    story_runner._RunStoryAndProcessErrorIfNeeded(
        root_mock.story, root_mock.results, root_mock.state, root_mock.test,
        self.finder_options)

    self.assertEqual(root_mock.method_calls, [
        mock.call.results.CreateArtifact('logs.txt'),
        mock.call.test.WillRunStory(root_mock.state.platform, root_mock.story),
        mock.call.state.WillRunStory(root_mock.story),
        mock.call.state.CanRunStory(root_mock.story),
        mock.call.state.RunStory(root_mock.results),
        mock.call.test.Measure(root_mock.state.platform, root_mock.results),
        mock.call.test.DidRunStory(root_mock.state.platform, root_mock.results),
        mock.call.state.DidRunStory(root_mock.results),
    ])

  def testRunStoryAndProcessErrorIfNeeded_successLegacy(self):
    root_mock = self._CreateErrorProcessingMock(legacy_test=True)

    story_runner._RunStoryAndProcessErrorIfNeeded(
        root_mock.story, root_mock.results, root_mock.state, root_mock.test,
        self.finder_options)

    self.assertEqual(root_mock.method_calls, [
        mock.call.results.CreateArtifact('logs.txt'),
        mock.call.state.WillRunStory(root_mock.story),
        mock.call.state.CanRunStory(root_mock.story),
        mock.call.state.RunStory(root_mock.results),
        mock.call.test.DidRunPage(root_mock.state.platform),
        mock.call.state.DidRunStory(root_mock.results),
    ])

  def testRunStoryAndProcessErrorIfNeeded_tryTimeout(self):
    root_mock = self._CreateErrorProcessingMock(method_exceptions={
        'state.WillRunStory': exceptions.TimeoutException('foo')
    })

    story_runner._RunStoryAndProcessErrorIfNeeded(
        root_mock.story, root_mock.results, root_mock.state, root_mock.test,
        self.finder_options)

    self.assertEqual(root_mock.method_calls, [
        mock.call.results.CreateArtifact('logs.txt'),
        mock.call.test.WillRunStory(root_mock.state.platform, root_mock.story),
        mock.call.state.WillRunStory(root_mock.story),
        mock.call.state.DumpStateUponStoryRunFailure(root_mock.results),
        mock.call.results.Fail(
            'Exception raised running %s' % root_mock.story.name),
        mock.call.test.DidRunStory(root_mock.state.platform, root_mock.results),
        mock.call.state.DidRunStory(root_mock.results),
    ])

  def testRunStoryAndProcessErrorIfNeeded_tryAppCrash(self):
    tmp = tempfile.NamedTemporaryFile(delete=False)
    tmp.close()
    temp_file_path = tmp.name
    fake_app = fakes.FakeApp()
    fake_app.recent_minidump_path = temp_file_path
    try:
      app_crash_exception = exceptions.AppCrashException(fake_app, msg='foo')
      root_mock = self._CreateErrorProcessingMock(method_exceptions={
          'state.WillRunStory': app_crash_exception
      })

      with self.assertRaises(exceptions.AppCrashException):
        story_runner._RunStoryAndProcessErrorIfNeeded(
            root_mock.story, root_mock.results, root_mock.state, root_mock.test,
            self.finder_options)

      self.assertListEqual(root_mock.method_calls, [
          mock.call.results.CreateArtifact('logs.txt'),
          mock.call.test.WillRunStory(
              root_mock.state.platform, root_mock.story),
          mock.call.state.WillRunStory(root_mock.story),
          mock.call.state.DumpStateUponStoryRunFailure(root_mock.results),
          mock.call.results.Fail(
              'Exception raised running %s' % root_mock.story.name),
          mock.call.test.DidRunStory(
              root_mock.state.platform, root_mock.results),
          mock.call.state.DidRunStory(root_mock.results),
      ])
    finally:
      os.remove(temp_file_path)

  def testRunStoryAndProcessErrorIfNeeded_tryError(self):
    root_mock = self._CreateErrorProcessingMock(method_exceptions={
        'state.CanRunStory': exceptions.Error('foo')
    })

    with self.assertRaisesRegex(exceptions.Error, 'foo'):
      story_runner._RunStoryAndProcessErrorIfNeeded(
          root_mock.story, root_mock.results, root_mock.state, root_mock.test,
          self.finder_options)

    self.assertEqual(root_mock.method_calls, [
        mock.call.results.CreateArtifact('logs.txt'),
        mock.call.test.WillRunStory(root_mock.state.platform, root_mock.story),
        mock.call.state.WillRunStory(root_mock.story),
        mock.call.state.CanRunStory(root_mock.story),
        mock.call.state.DumpStateUponStoryRunFailure(root_mock.results),
        mock.call.results.Fail(
            'Exception raised running %s' % root_mock.story.name),
        mock.call.test.DidRunStory(root_mock.state.platform, root_mock.results),
        mock.call.state.DidRunStory(root_mock.results),
    ])

  def testRunStoryAndProcessErrorIfNeeded_tryUnsupportedAction(self):
    root_mock = self._CreateErrorProcessingMock(method_exceptions={
        'state.RunStory': page_action.PageActionNotSupported('foo')
    })

    story_runner._RunStoryAndProcessErrorIfNeeded(
        root_mock.story, root_mock.results, root_mock.state, root_mock.test,
        self.finder_options)
    self.assertEqual(root_mock.method_calls, [
        mock.call.results.CreateArtifact('logs.txt'),
        mock.call.test.WillRunStory(root_mock.state.platform, root_mock.story),
        mock.call.state.WillRunStory(root_mock.story),
        mock.call.state.CanRunStory(root_mock.story),
        mock.call.state.RunStory(root_mock.results),
        mock.call.results.Skip('Unsupported page action: foo'),
        mock.call.test.DidRunStory(root_mock.state.platform, root_mock.results),
        mock.call.state.DidRunStory(root_mock.results),
    ])

  def testRunStoryAndProcessErrorIfNeeded_tryUnhandlable(self):
    root_mock = self._CreateErrorProcessingMock(method_exceptions={
        'test.WillRunStory': Exception('foo')
    })

    with self.assertRaisesRegex(Exception, 'foo'):
      story_runner._RunStoryAndProcessErrorIfNeeded(
          root_mock.story, root_mock.results, root_mock.state, root_mock.test,
          self.finder_options)

    self.assertEqual(root_mock.method_calls, [
        mock.call.results.CreateArtifact('logs.txt'),
        mock.call.test.WillRunStory(root_mock.state.platform, root_mock.story),
        mock.call.state.DumpStateUponStoryRunFailure(root_mock.results),
        mock.call.results.Fail(
            'Exception raised running %s' % root_mock.story.name),
        mock.call.test.DidRunStory(root_mock.state.platform, root_mock.results),
        mock.call.state.DidRunStory(root_mock.results),
    ])

  def testRunStoryAndProcessErrorIfNeeded_finallyException(self):
    exc = Exception('bar')
    root_mock = self._CreateErrorProcessingMock(method_exceptions={
        'state.DidRunStory': exc,
    })

    with self.assertRaisesRegex(Exception, 'bar'):
      story_runner._RunStoryAndProcessErrorIfNeeded(
          root_mock.story, root_mock.results, root_mock.state, root_mock.test,
          self.finder_options)

    self.assertEqual(root_mock.method_calls, [
        mock.call.results.CreateArtifact('logs.txt'),
        mock.call.test.WillRunStory(root_mock.state.platform, root_mock.story),
        mock.call.state.WillRunStory(root_mock.story),
        mock.call.state.CanRunStory(root_mock.story),
        mock.call.state.RunStory(root_mock.results),
        mock.call.test.Measure(root_mock.state.platform, root_mock.results),
        mock.call.test.DidRunStory(root_mock.state.platform, root_mock.results),
        mock.call.state.DidRunStory(root_mock.results),
        mock.call.state.DumpStateUponStoryRunFailure(root_mock.results),
    ])

  def testRunStoryAndProcessErrorIfNeeded_tryTimeout_finallyException(self):
    root_mock = self._CreateErrorProcessingMock(method_exceptions={
        'state.RunStory': exceptions.TimeoutException('foo'),
        'state.DidRunStory': Exception('bar')
    })

    story_runner._RunStoryAndProcessErrorIfNeeded(
        root_mock.story, root_mock.results, root_mock.state, root_mock.test,
        self.finder_options)

    self.assertEqual(root_mock.method_calls, [
        mock.call.results.CreateArtifact('logs.txt'),
        mock.call.test.WillRunStory(root_mock.state.platform, root_mock.story),
        mock.call.state.WillRunStory(root_mock.story),
        mock.call.state.CanRunStory(root_mock.story),
        mock.call.state.RunStory(root_mock.results),
        mock.call.state.DumpStateUponStoryRunFailure(root_mock.results),
        mock.call.results.Fail(
            'Exception raised running %s' % root_mock.story.name),
        mock.call.test.DidRunStory(root_mock.state.platform, root_mock.results),
        mock.call.state.DidRunStory(root_mock.results),
    ])

  def testRunStoryAndProcessErrorIfNeeded_tryError_finallyException(self):
    root_mock = self._CreateErrorProcessingMock(method_exceptions={
        'state.WillRunStory': exceptions.Error('foo'),
        'test.DidRunStory': Exception('bar')
    })

    with self.assertRaisesRegex(exceptions.Error, 'foo'):
      story_runner._RunStoryAndProcessErrorIfNeeded(
          root_mock.story, root_mock.results, root_mock.state, root_mock.test,
          self.finder_options)

    self.assertEqual(root_mock.method_calls, [
        mock.call.results.CreateArtifact('logs.txt'),
        mock.call.test.WillRunStory(root_mock.state.platform, root_mock.story),
        mock.call.state.WillRunStory(root_mock.story),
        mock.call.state.DumpStateUponStoryRunFailure(root_mock.results),
        mock.call.results.Fail(
            'Exception raised running %s' % root_mock.story.name),
        mock.call.test.DidRunStory(root_mock.state.platform, root_mock.results),
    ])

  def testRunStoryAndProcessErrorIfNeeded_tryUnsupportedAction_finallyException(
      self):
    root_mock = self._CreateErrorProcessingMock(method_exceptions={
        'test.WillRunStory': page_action.PageActionNotSupported('foo'),
        'state.DidRunStory': Exception('bar')
    })

    story_runner._RunStoryAndProcessErrorIfNeeded(
        root_mock.story, root_mock.results, root_mock.state, root_mock.test,
        self.finder_options)

    self.assertEqual(root_mock.method_calls, [
        mock.call.results.CreateArtifact('logs.txt'),
        mock.call.test.WillRunStory(root_mock.state.platform, root_mock.story),
        mock.call.results.Skip('Unsupported page action: foo'),
        mock.call.test.DidRunStory(root_mock.state.platform, root_mock.results),
        mock.call.state.DidRunStory(root_mock.results),
    ])

  def testRunStoryAndProcessErrorIfNeeded_tryUnhandlable_finallyException(self):
    root_mock = self._CreateErrorProcessingMock(method_exceptions={
        'test.Measure': Exception('foo'),
        'test.DidRunStory': Exception('bar')
    })

    with self.assertRaisesRegex(Exception, 'foo'):
      story_runner._RunStoryAndProcessErrorIfNeeded(
          root_mock.story, root_mock.results, root_mock.state, root_mock.test,
          self.finder_options)

    self.assertEqual(root_mock.method_calls, [
        mock.call.results.CreateArtifact('logs.txt'),
        mock.call.test.WillRunStory(root_mock.state.platform, root_mock.story),
        mock.call.state.WillRunStory(root_mock.story),
        mock.call.state.CanRunStory(root_mock.story),
        mock.call.state.RunStory(root_mock.results),
        mock.call.test.Measure(root_mock.state.platform, root_mock.results),
        mock.call.state.DumpStateUponStoryRunFailure(root_mock.results),
        mock.call.results.Fail(
            'Exception raised running %s' % root_mock.story.name),
        mock.call.test.DidRunStory(root_mock.state.platform, root_mock.results),
    ])


class FakeBenchmark(benchmark.Benchmark):
  test = test_stories.DummyStoryTest
  NAME = 'fake_benchmark'

  def __init__(self, stories=None, **kwargs):
    """A customizable fake_benchmark.

    Args:
      stories: Optional sequence of either story names or objects. Instances
        of DummyStory are useful here. If omitted the benchmark will contain
        a single DummyStory.
      other kwargs are passed to the test_stories.DummyStorySet constructor.
    """
    super().__init__()
    self._story_set = test_stories.DummyStorySet(
        stories if stories is not None else ['story'], **kwargs)

  @classmethod
  def Name(cls):
    return cls.NAME

  def CreateStorySet(self, _):
    return self._story_set


class FakeStoryFilter():
  def __init__(self, stories_to_filter_out=None, stories_to_skip=None):
    self._stories_to_filter = stories_to_filter_out or []
    self._stories_to_skip = stories_to_skip or []
    assert isinstance(self._stories_to_filter, list)
    assert isinstance(self._stories_to_skip, list)

  def FilterStories(self, story_set):
    return [story for story in story_set
            if story.name not in self._stories_to_filter]

  def ShouldSkip(self, story, should_log=False):
    del should_log  # unused
    return 'fake_reason' if story.name in self._stories_to_skip else ''


def ReadDiagnostics(test_result):
  artifact = test_result['outputArtifacts'][page_test_results.DIAGNOSTICS_NAME]
  with open(artifact['filePath']) as f:
    return json.load(f)['diagnostics']


class RunBenchmarkTest(unittest.TestCase):
  """Tests that run fake benchmarks, no real browser is involved.

  All these tests:
  - Use a FakeBenchmark instance.
  - Call GetFakeBrowserOptions to get options for a fake browser.
  - Call story_runner.RunBenchmark as entry point.
  """
  def setUp(self):
    self.output_dir = tempfile.mkdtemp()

  def tearDown(self):
    shutil.rmtree(self.output_dir)

  def GetFakeBrowserOptions(self, overrides=None):
    return options_for_unittests.GetRunOptions(
        output_dir=self.output_dir,
        fake_browser=True, overrides=overrides)

  def ReadTestResults(self):
    return results_options.ReadTestResults(
        os.path.join(self.output_dir, 'artifacts'))

  def testDisabledBenchmarkViaCanRunOnPlatform(self):
    fake_benchmark = FakeBenchmark()
    fake_benchmark.SUPPORTED_PLATFORMS = []
    options = self.GetFakeBrowserOptions()
    story_runner.RunBenchmark(fake_benchmark, options)
    test_results = self.ReadTestResults()
    self.assertFalse(test_results)  # No tests ran at all.

  def testSkippedWithStoryFilter(self):
    fake_benchmark = FakeBenchmark(stories=['fake_story'])
    options = self.GetFakeBrowserOptions()
    fake_story_filter = FakeStoryFilter(stories_to_skip=['fake_story'])
    with mock.patch(
        'telemetry.story.story_filter.StoryFilterFactory.BuildStoryFilter',
        return_value=fake_story_filter):
      story_runner.RunBenchmark(fake_benchmark, options)
    test_results = self.ReadTestResults()
    self.assertTrue(test_results)  # Some tests ran, but all skipped.
    self.assertTrue(all(t['status'] == 'SKIP' for t in test_results))

  def testOneStorySkippedOneNot(self):
    fake_story_filter = FakeStoryFilter(stories_to_skip=['story1'])
    fake_benchmark = FakeBenchmark(stories=['story1', 'story2'])
    options = self.GetFakeBrowserOptions()
    with mock.patch(
        'telemetry.story.story_filter.StoryFilterFactory.BuildStoryFilter',
        return_value=fake_story_filter):
      story_runner.RunBenchmark(fake_benchmark, options)
    test_results = self.ReadTestResults()
    status = [t['status'] for t in test_results]
    self.assertEqual(len(status), 2)
    self.assertIn('SKIP', status)
    self.assertIn('PASS', status)

  def testOneStoryFilteredOneNot(self):
    fake_story_filter = FakeStoryFilter(stories_to_filter_out=['story1'])
    fake_benchmark = FakeBenchmark(stories=['story1', 'story2'])
    options = self.GetFakeBrowserOptions()
    with mock.patch(
        'telemetry.story.story_filter.StoryFilterFactory.BuildStoryFilter',
        return_value=fake_story_filter):
      story_runner.RunBenchmark(fake_benchmark, options)
    test_results = self.ReadTestResults()
    self.assertEqual(len(test_results), 1)
    self.assertEqual(test_results[0]['status'], 'PASS')
    self.assertTrue(test_results[0]['testPath'].endswith('/story2'))

  def testValidateBenchmarkName(self):
    class FakeBenchmarkWithBadName(FakeBenchmark):
      NAME = 'bad/benchmark (name)'

    fake_benchmark = FakeBenchmarkWithBadName()
    options = self.GetFakeBrowserOptions()
    return_code = story_runner.RunBenchmark(fake_benchmark, options)
    self.assertEqual(return_code, 2)
    self.assertIn('Invalid benchmark name', sys.stderr.getvalue())

  def testWithOwnerInfo(self):

    @benchmark.Owner(emails=['alice@chromium.org', 'bob@chromium.org'],
                     component='fooBar',
                     documentation_url='https://example.com/')
    class FakeBenchmarkWithOwner(FakeBenchmark):
      pass

    fake_benchmark = FakeBenchmarkWithOwner()
    options = self.GetFakeBrowserOptions()
    story_runner.RunBenchmark(fake_benchmark, options)
    test_results = self.ReadTestResults()
    diagnostics = ReadDiagnostics(test_results[0])
    self.assertEqual(diagnostics['owners'],
                     ['alice@chromium.org', 'bob@chromium.org'])
    self.assertEqual(diagnostics['bugComponents'], ['fooBar'])
    self.assertEqual(diagnostics['documentationLinks'],
                     [['Benchmark documentation link', 'https://example.com/']])

  def testWithOwnerInfoButNoUrl(self):

    @benchmark.Owner(emails=['alice@chromium.org'])
    class FakeBenchmarkWithOwner(FakeBenchmark):
      pass

    fake_benchmark = FakeBenchmarkWithOwner()
    options = self.GetFakeBrowserOptions()
    story_runner.RunBenchmark(fake_benchmark, options)
    test_results = self.ReadTestResults()
    diagnostics = ReadDiagnostics(test_results[0])
    self.assertEqual(diagnostics['owners'], ['alice@chromium.org'])
    self.assertNotIn('documentationLinks', diagnostics)

  def testDeviceInfo(self):
    fake_benchmark = FakeBenchmark(stories=['fake_story'])
    options = self.GetFakeBrowserOptions()
    options.fake_possible_browser = fakes.FakePossibleBrowser(
        arch_name='abc', os_name='win', os_version_name='win10')
    story_runner.RunBenchmark(fake_benchmark, options)
    test_results = self.ReadTestResults()
    diagnostics = ReadDiagnostics(test_results[0])
    self.assertEqual(diagnostics['architectures'], ['abc'])
    self.assertEqual(diagnostics['osNames'], ['win'])
    self.assertEqual(diagnostics['osVersions'], ['win10'])

  def testReturnCodeDisabledStory(self):
    fake_benchmark = FakeBenchmark(stories=['fake_story'])
    fake_story_filter = FakeStoryFilter(stories_to_skip=['fake_story'])
    options = self.GetFakeBrowserOptions()
    with mock.patch(
        'telemetry.story.story_filter.StoryFilterFactory.BuildStoryFilter',
        return_value=fake_story_filter):
      return_code = story_runner.RunBenchmark(fake_benchmark, options)
    self.assertEqual(return_code, exit_codes.ALL_TESTS_SKIPPED)

  def testReturnCodeSuccessfulRun(self):
    fake_benchmark = FakeBenchmark()
    options = self.GetFakeBrowserOptions()
    return_code = story_runner.RunBenchmark(fake_benchmark, options)
    self.assertEqual(return_code, exit_codes.SUCCESS)

  def testReturnCodeCaughtException(self):
    fake_benchmark = FakeBenchmark(stories=[
        test_stories.DummyStory(
            'story', run_side_effect=exceptions.AppCrashException())])
    options = self.GetFakeBrowserOptions()
    return_code = story_runner.RunBenchmark(fake_benchmark, options)
    self.assertEqual(return_code, exit_codes.TEST_FAILURE)

  def testReturnCodeUnhandleableError(self):
    fake_benchmark = FakeBenchmark(stories=[
        test_stories.DummyStory(
            'story', run_side_effect=MemoryError('Unhandleable'))])
    options = self.GetFakeBrowserOptions()
    return_code = story_runner.RunBenchmark(fake_benchmark, options)
    self.assertEqual(return_code, exit_codes.FATAL_ERROR)

  def testRunStoryWithMissingArchiveFile(self):
    fake_benchmark = FakeBenchmark(archive_data_file='data/does-not-exist.json')
    options = self.GetFakeBrowserOptions()
    return_code = story_runner.RunBenchmark(fake_benchmark, options)
    self.assertEqual(return_code, 2)  # Benchmark was interrupted.
    self.assertIn('ArchiveError', sys.stderr.getvalue())

  def testDownloadMinimalServingDirs(self):
    fake_benchmark = FakeBenchmark(stories=[
        test_stories.DummyStory(
            'story_foo', serving_dir='/files/foo', tags=['foo']),
        test_stories.DummyStory(
            'story_bar', serving_dir='/files/bar', tags=['bar']),
    ], cloud_bucket=cloud_storage.PUBLIC_BUCKET)
    options = self.GetFakeBrowserOptions(overrides={'story_tag_filter': 'foo'})
    with mock.patch(
        'py_utils.cloud_storage.GetFilesInDirectoryIfChanged') as get_files:
      story_runner.RunBenchmark(fake_benchmark, options)

    # Foo is the only included story serving dir.
    self.assertEqual(get_files.call_count, 1)
    get_files.assert_called_once_with('/files/foo', cloud_storage.PUBLIC_BUCKET)

  def testAbridged(self):
    options = self.GetFakeBrowserOptions()
    options.run_abridged_story_set = True
    story_filter.StoryFilterFactory.ProcessCommandLineArgs(
        parser=None, args=options)
    fake_benchmark = FakeBenchmark(stories=[
        test_stories.DummyStory('story1', tags=['important']),
        test_stories.DummyStory('story2', tags=['other']),
    ], abridging_tag='important')
    story_runner.RunBenchmark(fake_benchmark, options)
    test_results = self.ReadTestResults()
    self.assertEqual(len(test_results), 1)
    self.assertTrue(test_results[0]['testPath'].endswith('/story1'))

  def testFullRun(self):
    options = self.GetFakeBrowserOptions()
    story_filter.StoryFilterFactory.ProcessCommandLineArgs(
        parser=None, args=options)
    fake_benchmark = FakeBenchmark(stories=[
        test_stories.DummyStory('story1', tags=['important']),
        test_stories.DummyStory('story2', tags=['other']),
    ], abridging_tag='important')
    story_runner.RunBenchmark(fake_benchmark, options)
    test_results = self.ReadTestResults()
    self.assertEqual(len(test_results), 2)

  def testStoryFlag(self):
    options = self.GetFakeBrowserOptions()
    args = fakes.FakeParsedArgsForStoryFilter(stories=['story1', 'story3'])
    story_filter.StoryFilterFactory.ProcessCommandLineArgs(
        parser=None, args=args)
    fake_benchmark = FakeBenchmark(stories=['story1', 'story2', 'story3'])
    story_runner.RunBenchmark(fake_benchmark, options)
    test_results = self.ReadTestResults()
    self.assertEqual(len(test_results), 2)
    self.assertTrue(test_results[0]['testPath'].endswith('/story1'))
    self.assertTrue(test_results[1]['testPath'].endswith('/story3'))

  def testArtifactLogsContainHandleableException(self):
    def failed_run():
      logging.warning('This will fail gracefully')
      raise exceptions.TimeoutException('karma!')

    fake_benchmark = FakeBenchmark(stories=[
        test_stories.DummyStory('story1', run_side_effect=failed_run),
        test_stories.DummyStory('story2')
    ])

    options = self.GetFakeBrowserOptions()
    return_code = story_runner.RunBenchmark(fake_benchmark, options)
    self.assertEqual(return_code, exit_codes.TEST_FAILURE)
    test_results = self.ReadTestResults()
    self.assertEqual(len(test_results), 2)

    # First story failed.
    self.assertEqual(test_results[0]['testPath'], 'fake_benchmark/story1')
    self.assertEqual(test_results[0]['status'], 'FAIL')
    self.assertIn('logs.txt', test_results[0]['outputArtifacts'])

    with open(test_results[0]['outputArtifacts']['logs.txt']['filePath']) as f:
      test_log = f.read()

    # Ensure that the log contains warning messages and python stack.
    self.assertIn('Handleable error', test_log)
    self.assertIn('This will fail gracefully', test_log)
    self.assertIn("raise exceptions.TimeoutException('karma!')", test_log)

    # Second story ran fine.
    self.assertEqual(test_results[1]['testPath'], 'fake_benchmark/story2')
    self.assertEqual(test_results[1]['status'], 'PASS')

  def testArtifactLogsContainUnhandleableException(self):
    def failed_run():
      logging.warning('This will fail badly')
      raise MemoryError('this is a fatal exception')

    fake_benchmark = FakeBenchmark(stories=[
        test_stories.DummyStory('story1', run_side_effect=failed_run),
        test_stories.DummyStory('story2')
    ])

    options = self.GetFakeBrowserOptions()
    return_code = story_runner.RunBenchmark(fake_benchmark, options)
    self.assertEqual(return_code, exit_codes.FATAL_ERROR)
    test_results = self.ReadTestResults()
    self.assertEqual(len(test_results), 2)

    # First story failed.
    self.assertEqual(test_results[0]['testPath'], 'fake_benchmark/story1')
    self.assertEqual(test_results[0]['status'], 'FAIL')
    self.assertIn('logs.txt', test_results[0]['outputArtifacts'])

    with open(test_results[0]['outputArtifacts']['logs.txt']['filePath']) as f:
      test_log = f.read()

    # Ensure that the log contains warning messages and python stack.
    self.assertIn('Unhandleable error', test_log)
    self.assertIn('This will fail badly', test_log)
    self.assertIn("raise MemoryError('this is a fatal exception')", test_log)

    # Second story was skipped.
    self.assertEqual(test_results[1]['testPath'], 'fake_benchmark/story2')
    self.assertEqual(test_results[1]['status'], 'SKIP')

  def testUnexpectedSkipsWithFiltering(self):
    # We prepare side effects for 50 stories, the first 30 run fine, the
    # remaining 20 fail with a fatal error.
    fatal_error = MemoryError('this is an unexpected exception')
    side_effects = [None] * 30 + [fatal_error] * 20

    fake_benchmark = FakeBenchmark(stories=(
        test_stories.DummyStory('story_%i' % i, run_side_effect=effect)
        for i, effect in enumerate(side_effects)))

    # Set the filtering to only run from story_10 --> story_40
    options = self.GetFakeBrowserOptions({
        'story_shard_begin_index': 10,
        'story_shard_end_index': 41})
    return_code = story_runner.RunBenchmark(fake_benchmark, options)
    self.assertEqual(exit_codes.FATAL_ERROR, return_code)

    # The results should contain entries of story 10 --> story 40. Of those
    # entries, story 31's actual result is 'FAIL' and
    # stories from 31 to 40 will shows 'SKIP'.
    test_results = self.ReadTestResults()
    self.assertEqual(len(test_results), 31)

    expected = []
    expected.extend(('story_%i' % i, 'PASS') for i in range(10, 30))
    expected.append(('story_30', 'FAIL'))
    expected.extend(('story_%i' % i, 'SKIP') for i in range(31, 41))

    for (story, status), result in zip(expected, test_results):
      self.assertEqual(result['testPath'], 'fake_benchmark/%s' % story)
      self.assertEqual(result['status'], status)


  def testRangeIndexSingles(self):
    fake_benchmark = FakeBenchmark(stories=(
        test_stories.DummyStory('story_%i' % i) for i in range(100)))
    options = self.GetFakeBrowserOptions({
        'story_shard_indexes': "2,50,90"})
    story_runner.RunBenchmark(fake_benchmark, options)
    test_results = self.ReadTestResults()
    self.assertEqual(len(test_results), 3)


  def testRangeIndexRanges(self):
    fake_benchmark = FakeBenchmark(stories=(
        test_stories.DummyStory('story_%i' % i) for i in range(100)))
    options = self.GetFakeBrowserOptions({
        'story_shard_indexes': "-10, 20-30, 90-"})
    story_runner.RunBenchmark(fake_benchmark, options)
    test_results = self.ReadTestResults()
    self.assertEqual(len(test_results), 30)
