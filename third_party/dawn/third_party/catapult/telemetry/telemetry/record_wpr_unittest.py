# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import contextlib
import os
import sys

from py_utils import tempfile_ext

from telemetry import benchmark
from telemetry import story
from telemetry.core import util
from telemetry import decorators
from telemetry.page import page as page_module
from telemetry.page import legacy_page_test
from telemetry.project_config import ProjectConfig
from telemetry import record_wpr
from telemetry.testing import options_for_unittests
from telemetry.testing import tab_test_case
from telemetry.util import wpr_modes


class MockPage(page_module.Page):
  def __init__(self, story_set, url):
    super().__init__(url=url,
                                   page_set=story_set,
                                   base_dir=util.GetUnittestDataDir(),
                                   name=url)
    self.func_calls = []

  def RunPageInteractions(self, _):
    self.func_calls.append('RunPageInteractions')

  def RunSmoothness(self, _):
    self.func_calls.append('RunSmoothness')


class MockStorySet(story.StorySet):
  def __init__(self, url=''):
    if url == '':
      return
    super().__init__(
        archive_data_file='data/archive_files/test.json')
    self.AddStory(MockPage(self, url))


class MockPageTest(legacy_page_test.LegacyPageTest):
  def __init__(self):
    super().__init__()
    self._action_name_to_run = "RunPageInteractions"
    self.func_calls = []

  def CustomizeBrowserOptions(self, options):
    self.func_calls.append('CustomizeBrowserOptions')

  def WillNavigateToPage(self, page, tab):
    self.func_calls.append('WillNavigateToPage')

  def DidNavigateToPage(self, page, tab):
    self.func_calls.append('DidNavigateToPage')

  def ValidateAndMeasurePage(self, page, tab, results):
    self.func_calls.append('ValidateAndMeasurePage')

  def WillStartBrowser(self, platform):
    self.func_calls.append('WillStartBrowser')

  def DidStartBrowser(self, browser):
    self.func_calls.append('DidStartBrowser')


class MockBenchmark(benchmark.Benchmark):
  test = MockPageTest

  def __init__(self):
    super().__init__()
    self.mock_story_set = None

  @classmethod
  def AddBenchmarkCommandLineArgs(cls, parser):
    parser.add_argument('--mock-benchmark-url')

  def CreateStorySet(self, options):
    kwargs = {}
    if hasattr(options, 'mock_benchmark_url') and options.mock_benchmark_url:
      kwargs['url'] = options.mock_benchmark_url
    self.mock_story_set = MockStorySet(**kwargs)
    return self.mock_story_set

  def SetExtraBrowserOptions(self, options):
    options.AppendExtraBrowserArgs('--extra-option')


class MockTimelineBasedMeasurementBenchmark(benchmark.Benchmark):

  def __init__(self):
    super().__init__()
    self.mock_story_set = None

  @classmethod
  def AddBenchmarkCommandLineArgs(cls, parser):
    parser.add_argument('--mock-benchmark-url')

  def CreateStorySet(self, options):
    kwargs = {}
    if options.mock_benchmark_url:
      kwargs['url'] = options.mock_benchmark_url
    self.mock_story_set = MockStorySet(**kwargs)
    return self.mock_story_set


def _SuccessfulStories(results):
  return sorted(set(run.story for run in results.IterStoryRuns() if run.ok))


def _SkippedStories(results):
  return sorted(set(run.story
                    for run in results.IterStoryRuns() if run.skipped))


class RecordWprUnitTests(tab_test_case.TabTestCase):

  _base_dir = util.GetUnittestDataDir()
  _test_data_dir = os.path.join(util.GetUnittestDataDir(), 'page_tests')

  @classmethod
  def setUpClass(cls):
    sys.path.extend([cls._base_dir, cls._test_data_dir])
    super(RecordWprUnitTests, cls).setUpClass()
    cls._url = cls.UrlOfUnittestFile('blank.html')
    cls._test_options = options_for_unittests.GetCopy()

  @contextlib.contextmanager
  def SkippingWith(self, expectations):
    with tempfile_ext.NamedTemporaryFile(prefix='expectations_',
                                         suffix='.config',
                                         mode='w+') as expectations_out:
      expectations_out.write('# results: [ Skip ]\n')
      for issue, abenchmark, astory in expectations:
        expectations_out.write('{} {}/{} [ Skip ]\n'.format(
            issue, abenchmark, astory))
      expectations_out.close()
      yield expectations_out.name

  # When the RecorderPageTest is created from a Benchmark, the benchmark will
  # have a PageTest, specified by its test attribute.
  def testRunPage_OnlyRunBenchmarkAction(self):
    page_test = MockBenchmark().test()
    record_page_test = record_wpr.RecorderPageTest(page_test)
    page = MockPage(story_set=MockStorySet(url=self._url), url=self._url)
    record_page_test.ValidateAndMeasurePage(page, self._tab, results=None)

  def testRunPage_CallBenchmarksPageTestsFunctions(self):
    page_test = MockBenchmark().test()
    record_page_test = record_wpr.RecorderPageTest(page_test)
    page = MockPage(story_set=MockStorySet(url=self._url), url=self._url)
    record_page_test.ValidateAndMeasurePage(page, self._tab, results=None)
    self.assertEqual(1, len(page_test.func_calls))
    self.assertEqual('ValidateAndMeasurePage', page_test.func_calls[0])

  def GetBrowserDeviceFlags(self):
    flags = ['--browser', self._browser.browser_type,
             '--remote', self._test_options.remote,
             '--device', self._device]
    if self._browser.browser_type == 'exact':
      flags += ['--browser-executable', self._test_options.browser_executable]
    if self._test_options.chrome_root:
      flags += ['--chrome-root', self._test_options.chrome_root]
    return flags

  @decorators.Disabled('chromeos') # crbug.com/404868.
  def testWprRecorderWithPageSet(self):
    flags = self.GetBrowserDeviceFlags()
    mock_story_set = MockStorySet(url=self._url)
    with record_wpr.WprRecorder(ProjectConfig(self._test_data_dir),
                                mock_story_set, flags) as wpr_recorder:
      with wpr_recorder.CreateResults() as results:
        wpr_recorder.Record(results)
        self.assertEqual(
            sorted(mock_story_set.stories), _SuccessfulStories(results))

  def testWprRecorderWithBenchmark(self):
    flags = self.GetBrowserDeviceFlags()
    flags.extend(['--mock-benchmark-url', self._url])
    mock_benchmark = MockBenchmark()
    with record_wpr.WprRecorder(ProjectConfig(self._test_data_dir),
                                mock_benchmark, flags) as wpr_recorder:
      with wpr_recorder.CreateResults() as results:
        wpr_recorder.Record(results)
        self.assertEqual(
            sorted(mock_benchmark.mock_story_set.stories),
            _SuccessfulStories(results))

  def testWprRecorderWithTimelineBasedMeasurementBenchmark(self):
    flags = self.GetBrowserDeviceFlags()
    flags.extend(['--mock-benchmark-url', self._url])
    mock_benchmark = MockTimelineBasedMeasurementBenchmark()
    with record_wpr.WprRecorder(ProjectConfig(self._test_data_dir),
                                mock_benchmark, flags) as wpr_recorder:
      with wpr_recorder.CreateResults() as results:
        wpr_recorder.Record(results)
        self.assertEqual(
            sorted(mock_benchmark.mock_story_set.stories),
            _SuccessfulStories(results))

  def testPageSetBaseDirFlag(self):
    flags = self.GetBrowserDeviceFlags()
    flags.extend(['--page-set-base-dir', self._test_data_dir,
                  '--mock-benchmark-url', self._url])
    mock_benchmark = MockBenchmark()
    with record_wpr.WprRecorder(ProjectConfig('non-existent-dummy-dir'),
                                mock_benchmark, flags) as wpr_recorder:
      with wpr_recorder.CreateResults() as results:
        wpr_recorder.Record(results)
        self.assertEqual(
            sorted(mock_benchmark.mock_story_set.stories),
            _SuccessfulStories(results))

  def testCommandLineFlags(self):
    flags = [
        '--pageset-repeat', '2',
        '--mock-benchmark-url', self._url,
        '--upload',
    ]
    with record_wpr.WprRecorder(ProjectConfig(self._test_data_dir),
                                MockBenchmark(), flags) as wpr_recorder:
      # page_runner command-line args
      self.assertEqual(2, wpr_recorder.options.pageset_repeat)
      # benchmark command-line args
      self.assertEqual(self._url, wpr_recorder.options.mock_benchmark_url)
      # record_wpr command-line arg to upload to cloud-storage.
      self.assertTrue(wpr_recorder.options.upload)
      # --extra-option added from Benchmark.SetExtraBrowserOptions()
      self.assertTrue('--extra-option' in
                      wpr_recorder.options.browser_options.extra_browser_args)
      # invalid command-line args
      self.assertFalse(hasattr(wpr_recorder.options, 'not_a_real_option'))

  def testCommandLineFlagParsingSkipped(self):
    flags = [
        '--pageset-repeat', '2',
        '--mock-benchmark-url', self._url,
        '--upload',
    ]
    with record_wpr.WprRecorder(ProjectConfig(self._test_data_dir),
                                MockBenchmark(), flags, False) as wpr_recorder:
      self.assertFalse(hasattr(wpr_recorder.options, 'mock_benchmark_url'))

  def testRecordingEnabled(self):
    flags = ['--mock-benchmark-url', self._url]
    with record_wpr.WprRecorder(ProjectConfig(self._test_data_dir),
                                MockBenchmark(), flags) as wpr_recorder:
      self.assertEqual(wpr_modes.WPR_RECORD,
                       wpr_recorder.options.browser_options.wpr_mode)

  # When the RecorderPageTest CustomizeBrowserOptions/WillStartBrowser/
  # DidStartBrowser function is called, it forwards the call to the PageTest
  def testRecorderPageTest_BrowserMethods(self):
    flags = ['--mock-benchmark-url', self._url]
    page_test = MockBenchmark().test()
    record_page_test = record_wpr.RecorderPageTest(page_test)
    with record_wpr.WprRecorder(ProjectConfig(self._test_data_dir),
                                MockBenchmark(), flags) as wpr_recorder:
      record_page_test.CustomizeBrowserOptions(wpr_recorder.options)
      record_page_test.WillStartBrowser(self._tab.browser.platform)
      record_page_test.DidStartBrowser(self._tab.browser)
      self.assertTrue('CustomizeBrowserOptions' in page_test.func_calls)
      self.assertTrue('WillStartBrowser' in page_test.func_calls)
      self.assertTrue('DidStartBrowser' in page_test.func_calls)

  def testUseLiveSitesUnsupported(self):
    flags = ['--use-live-sites']
    with self.assertRaises(SystemExit):
      record_wpr.WprRecorder(ProjectConfig(self._test_data_dir),
                             MockBenchmark(), flags)

  def testWprRecorderWithBenchmarkAndExpectations(self):
    flags = self.GetBrowserDeviceFlags()
    flags.extend(['--mock-benchmark-url', self._url])
    mock_benchmark = MockBenchmark()
    page = MockPage(story_set=MockStorySet(url=self._url), url=self._url)
    skips = [('crbug.com/1237031', mock_benchmark.Name(), page.name)]
    with self.SkippingWith(expectations=skips) as expectations_file:
      config = ProjectConfig(self._test_data_dir,
                             expectations_files=[expectations_file])
      with record_wpr.WprRecorder(config, mock_benchmark,
                                  flags) as wpr_recorder:
        with wpr_recorder.CreateResults() as results:
          wpr_recorder.Record(results)
          self.assertEqual([], _SuccessfulStories(results))
          self.assertEqual(
              sorted(mock_benchmark.mock_story_set.stories),
              _SkippedStories(results))

  def testWprRecorderWithBenchmarkAndEmptyExpectations(self):
    flags = self.GetBrowserDeviceFlags()
    flags.extend(['--mock-benchmark-url', self._url])
    mock_benchmark = MockBenchmark()
    with self.SkippingWith(expectations=[]) as expectations_file:
      config = ProjectConfig(self._test_data_dir,
                             expectations_files=[expectations_file])
      with record_wpr.WprRecorder(config, mock_benchmark,
                                  flags) as wpr_recorder:
        with wpr_recorder.CreateResults() as results:
          wpr_recorder.Record(results)
          self.assertEqual([], _SkippedStories(results))
          self.assertEqual(
              sorted(mock_benchmark.mock_story_set.stories),
              _SuccessfulStories(results))
