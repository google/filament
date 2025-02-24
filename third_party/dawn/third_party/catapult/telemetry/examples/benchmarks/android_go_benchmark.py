# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import contextlib
import logging

from telemetry.core import android_platform
from telemetry.internal.browser import browser_finder
from telemetry.timeline import chrome_trace_category_filter
from telemetry.util import wpr_modes
from telemetry.web_perf import timeline_based_measurement
from telemetry import benchmark
from telemetry import story as story_module

from devil.android.sdk import intent


class SharedAndroidStoryState(story_module.SharedState):

  def __init__(self, test, finder_options, story_set, possible_browser):
    """
    Args:
      test: (unused)
      finder_options: A finder_options object
      story_set: (unused)
    """
    super().__init__(
        test, finder_options, story_set, possible_browser)
    self._finder_options = finder_options
    if not self._possible_browser:
      self._possible_browser = browser_finder.FindBrowser(self._finder_options)
    self._current_story = None

    # This is an Android-only shared state.
    assert isinstance(self.platform, android_platform.AndroidPlatform)
    self._finder_options.browser_options.browser_user_agent_type = 'mobile'

    # TODO: This will always use live sites. Should use options to configure
    # network_controller properly. See e.g.: https://goo.gl/nAsyFr
    self.platform.network_controller.Open(wpr_modes.WPR_OFF)
    self.platform.Initialize()

  @property
  def platform(self):
    return self._possible_browser.platform

  def TearDownState(self):
    self.platform.network_controller.Close()

  def LaunchBrowser(self, url):
    # Clear caches before starting browser.
    self.platform.FlushDnsCache()
    self._possible_browser.FlushOsPageCaches()
    # TODO: Android Go stories could, e.g., use the customtabs helper app to
    # start Chrome as a custom tab.
    self.platform.StartActivity(intent.Intent(
        package=self._possible_browser.settings.package,
        activity=self._possible_browser.GetActivityForCurrentSdk(),
        action=self._possible_browser.GetActionForCurrentSdk(),
        data=url),
                                blocking=True)

  @contextlib.contextmanager
  def FindBrowser(self):
    """Find and manage the lifetime of a browser.

    The browser is closed when exiting the context managed code, and the
    browser state is dumped in case of errors during the story execution.
    """
    browser = self._possible_browser.FindExistingBrowser()
    try:
      yield browser
    except Exception as exc:
      logging.critical(
          '%s raised during story run. Dumping current browser state to help'
          ' diagnose this issue.', type(exc).__name__)
      browser.DumpStateUponFailure()
      raise
    finally:
      browser.Close()

  def WillRunStory(self, story):
    # TODO: Should start replay to use WPR recordings.
    # See e.g.: https://goo.gl/UJuu8a
    self._possible_browser.SetUpEnvironment(
        self._finder_options.browser_options)
    self._current_story = story

  def RunStory(self, _):
    self._current_story.Run(self)

  def DidRunStory(self, _):
    self._current_story = None
    self._possible_browser.CleanUpEnvironment()

  def DumpStateUponStoryRunFailure(self, results):
    del results
    # Note: Dumping state of objects upon errors, e.g. of the browser, is
    # handled individually by the context managers that handle their lifetime.

  def CanRunStory(self, _):
    return True


class AndroidGoFooStory(story_module.Story):
  """An example story that restarts the browser a few times."""
  URL = 'https://en.wikipedia.org/wiki/Main_Page'

  def __init__(self):
    super().__init__(
        SharedAndroidStoryState, name='go:story:foo')

  def Run(self, shared_state):
    for _ in range(3):
      shared_state.LaunchBrowser(self.URL)
      with shared_state.FindBrowser() as browser:
        action_runner = browser.foreground_tab.action_runner
        action_runner.tab.WaitForDocumentReadyStateToBeComplete()
        action_runner.RepeatableBrowserDrivenScroll(repeat_count=2)


class AndroidGoBarStory(story_module.Story):
  def __init__(self):
    super().__init__(
        SharedAndroidStoryState, name='go:story:bar')

  def Run(self, shared_state):
    shared_state.LaunchBrowser('http://www.bbc.co.uk/news')
    with shared_state.FindBrowser() as browser:
      action_runner = browser.foreground_tab.action_runner
      action_runner.tab.WaitForDocumentReadyStateToBeComplete()
      action_runner.RepeatableBrowserDrivenScroll(repeat_count=2)


class AndroidGoStories(story_module.StorySet):
  def __init__(self):
    super().__init__()
    self.AddStory(AndroidGoFooStory())
    self.AddStory(AndroidGoBarStory())


class AndroidGoBenchmark(benchmark.Benchmark):
  def CreateCoreTimelineBasedMeasurementOptions(self):
    cat_filter = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        filter_string='rail,toplevel')

    options = timeline_based_measurement.Options(cat_filter)
    options.config.enable_chrome_trace = True
    options.SetTimelineBasedMetrics([
        'clockSyncLatencyMetric',
        'tracingMetric',
    ])
    return options

  def CreateStorySet(self, options):
    return AndroidGoStories()

  @classmethod
  def Name(cls):
    return 'android_go.example'
