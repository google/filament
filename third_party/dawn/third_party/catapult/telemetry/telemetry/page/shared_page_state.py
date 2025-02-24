# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import os
import shutil
from six.moves import input # pylint: disable=redefined-builtin

from telemetry.core import exceptions
from telemetry.core import platform as platform_module
from telemetry.internal.backends.chrome import gpu_compositing_checker
from telemetry.internal.browser import browser_info as browser_info_module
from telemetry.internal.browser import browser_interval_profiling_controller
from telemetry.internal.platform import android_device
from telemetry.page import cache_temperature
from telemetry.page import legacy_page_test
from telemetry.page import traffic_setting
from telemetry import story as story_module
from telemetry.util import screenshot


class SharedPageState(story_module.SharedState):
  """
  This class contains all specific logic necessary to run a Chrome browser
  benchmark.
  """

  _device_type = None

  def __init__(self, test, finder_options, story_set, possible_browser):
    super().__init__(
        test, finder_options, story_set, possible_browser)
    self._page_test = None
    if issubclass(type(test), legacy_page_test.LegacyPageTest):
      # We only need a page_test for legacy measurements that involve running
      # some commands before/after starting the browser or navigating to a page.
      # This is not needed for newer timeline (tracing) based benchmarks which
      # just collect a trace, then measurements are done after the fact by
      # analysing the trace itself.
      self._page_test = test
      self._page_test_results = None

    if (self._device_type == 'desktop' and
        platform_module.GetHostPlatform().GetOSName() == 'chromeos'):
      self._device_type = 'chromeos'
    if (possible_browser.browser_type == 'web-engine-shell' or
        possible_browser.browser_type == 'fuchsia-chrome' or
        possible_browser.browser_type == 'cast-streaming-shell'):
      self._device_type = None

    browser_options = finder_options.browser_options
    browser_options.browser_user_agent_type = self._device_type

    if self._page_test:
      self._page_test.CustomizeBrowserOptions(browser_options)

    self._browser = None
    self._extra_browser_args = None
    self._finder_options = finder_options

    self._first_browser = True
    self._current_page = None
    self._current_tab = None

    if self._page_test:
      self._page_test.SetOptions(self._finder_options)

    self._extra_wpr_args = browser_options.extra_wpr_args
    if (hasattr(finder_options, 'use_local_wpr') and
        finder_options.use_local_wpr):
      self._extra_wpr_args.append('--use-local-wpr')
    if (hasattr(finder_options, 'disable_fuzzy_url_matching') and
        finder_options.disable_fuzzy_url_matching):
      self._extra_wpr_args.append('--disable-fuzzy-url-matching')

    profiling_mod = browser_interval_profiling_controller
    self._interval_profiling_controller = (
        profiling_mod.BrowserIntervalProfilingController(
            possible_browser=self._possible_browser,
            process_name=finder_options.interval_profiling_target,
            periods=finder_options.interval_profiling_periods,
            frequency=finder_options.interval_profiling_frequency,
            profiler_options=finder_options.interval_profiler_options))

    self.platform.SetPerformanceMode(finder_options.performance_mode)
    self._perf_mode_set = (finder_options.performance_mode !=
                           android_device.KEEP_PERFORMANCE_MODE)
    self.platform.network_controller.Open(self.wpr_mode)
    self.platform.Initialize()
    self._video_recording_enabled = (self._finder_options.capture_screen_video
                                     and self.platform.CanRecordVideo())

  @property
  def interval_profiling_controller(self):
    return self._interval_profiling_controller

  @property
  def browser(self):
    return self._browser

  def DumpStateUponStoryRunFailure(self, results):
    # Dump browser standard output and log.
    if self._browser:
      self._browser.DumpStateUponFailure()
    else:
      logging.warning('Cannot dump browser state: No browser.')

    # Capture a screenshot
    if self._finder_options.browser_options.take_screenshot_for_failed_page:
      fh = screenshot.TryCaptureScreenShot(self.platform, self._current_tab)
      if fh is not None:
        with results.CaptureArtifact('screenshot.png') as path:
          shutil.move(fh.GetAbsPath(), path)
    else:
      logging.warning('Taking screenshots upon failures disabled.')

  def DidRunStory(self, results):
    self._AllowInteractionForStage('after-run-story')
    try:
      if not self.ShouldReuseBrowserForAllStoryRuns():
        self._StopBrowser()
      elif self._current_tab:
        # We might hang while trying to close the connection, and need to
        # guarantee the page will get cleaned up to avoid future tests failing
        # in weird ways.
        try:
          if self._current_tab.IsAlive():
            self._current_tab.CloseConnections()
        except Exception as exc: # pylint: disable=broad-except
          logging.warning(
              '%s raised while closing tab connections; tab will be closed.',
              type(exc).__name__)
          self._current_tab.Close()
      self._interval_profiling_controller.GetResults(
          self._current_page.file_safe_name, results)
    finally:
      self._current_page = None
      self._current_tab = None
      if self._video_recording_enabled:
        with results.CaptureArtifact('recording.mp4') as video_path:
          self.platform.StopVideoRecording(video_path)

  def ShouldReuseBrowserForAllStoryRuns(self):
    """Whether a single browser instance should be reused to run all stories.

    This should return False in most situations in order to help maitain
    independence between measurements taken on different story runs.

    Previously, we only reused the browser for ChromeOS,
    where bringing up the browser for each story is expensive.
    However, this is causing some tests to break.
    """
    return False

  @property
  def platform(self):
    return self._possible_browser.platform

  def _AllowInteractionForStage(self, stage):
    if self._finder_options.pause == stage:
      input('Pausing for interaction at %s... Press Enter to continue.' %
            stage)

  def _StartBrowser(self, page):
    assert self._browser is None
    self._AllowInteractionForStage('before-start-browser')

    if self._page_test:
      self._page_test.WillStartBrowser(self.platform)
    # Create a deep copy of browser_options so that we can add page-level
    # arguments and url to it without polluting the run for the next page.
    browser_options = self._finder_options.browser_options.Copy()
    self._extra_browser_args = page.extra_browser_args
    browser_options.AppendExtraBrowserArgs(page.extra_browser_args)
    self._possible_browser.SetUpEnvironment(browser_options)

    # Clear caches before starting browser.
    self.platform.FlushDnsCache()
    if browser_options.flush_os_page_caches_on_start:
      self._possible_browser.FlushOsPageCaches()

    self._browser = self._possible_browser.Create()
    if self._page_test:
      self._page_test.DidStartBrowser(self.browser)

    if browser_options.assert_gpu_compositing:
      gpu_compositing_checker.AssertGpuCompositingEnabled(
          self._browser.GetSystemInfo())

    if self._first_browser:
      self._first_browser = False
      # Cut back on mostly redundant logs length per crbug.com/943650.
      self._finder_options.browser_options.trim_logs = True
    self._AllowInteractionForStage('after-start-browser')

  def WillRunStory(self, story):
    reusing_browser = self.browser is not None
    page = story
    # Make sure we don't have accidentally diverging browser args.
    if reusing_browser and self._extra_browser_args != page._extra_browser_args:
      self._StopBrowser()
      reusing_browser = False

    if not self.platform.tracing_controller.is_tracing_running:
      # For TimelineBasedMeasurement benchmarks, tracing has already started.
      # For PageTest benchmarks, tracing has not yet started. We need to make
      # sure no tracing state is left before starting the browser for PageTest
      # benchmarks.
      self.platform.tracing_controller.ClearStateIfNeeded()

    self._current_page = page

    archive_path = page.story_set.WprFilePathForStory(
        page, self.platform.GetOSName())
    # TODO(crbug.com/1029785): Ideally we should just let the network
    # controller raise an exception when the archive_path is not found.
    if archive_path is not None and not os.path.isfile(archive_path):
      logging.warning('WPR archive missing: %s', archive_path)
      archive_path = None
    self.platform.network_controller.StartReplay(
        archive_path, page.make_javascript_deterministic, self._extra_wpr_args)

    if self._video_recording_enabled:
      self.platform.StartVideoRecording()
    if reusing_browser:
      assert self._extra_browser_args == page._extra_browser_args
    else:
      self._StartBrowser(page)

    if self.browser.supports_tab_control:
      if reusing_browser:
        # Try to close all previous tabs to maintain some independence between
        # individual story runs. Note that the final tab.Close(keep_one=True)
        # will create a fresh new tab before the last one is closed.
        while len(self.browser.tabs) > 1:
          self.browser.tabs[-1].Close()
        self.browser.tabs[-1].Close(keep_one=True)
      else:
        # Create a tab if there's none.
        if len(self.browser.tabs) == 0:
          self.browser.tabs.New()

      # Must wait for tab to commit otherwise it can commit after the next
      # navigation has begun and RenderFrameHostManager::DidNavigateMainFrame()
      # will cancel the next navigation because it's pending. This manifests as
      # the first navigation in a PageSet freezing indefinitely because the
      # navigation was silently canceled when |self.browser.tabs[0]| was
      # committed.
      self.browser.tabs[0].WaitForDocumentReadyStateToBeComplete()

    # Reset traffic shaping to speed up cache temperature setup.
    self.platform.network_controller.UpdateTrafficSettings(0, 0, 0)
    cache_temperature.EnsurePageCacheTemperature(
        self._current_page, self.browser)
    if self._current_page.traffic_setting != traffic_setting.NONE:
      s = traffic_setting.NETWORK_CONFIGS[self._current_page.traffic_setting]
      self.platform.network_controller.UpdateTrafficSettings(
          round_trip_latency_ms=s.round_trip_latency_ms,
          download_bandwidth_kbps=s.download_bandwidth_kbps,
          upload_bandwidth_kbps=s.upload_bandwidth_kbps)

    self._AllowInteractionForStage('before-run-story')

  def CanRunStory(self, story):
    return self.CanRunOnBrowser(browser_info_module.BrowserInfo(self.browser),
                                story)

  def CanRunOnBrowser(self, browser_info, page):
    """Override this to return whether the browser brought up by this state
    instance is suitable for running the given page.

    Args:
      browser_info: an instance of telemetry.core.browser_info.BrowserInfo
      page: an instance of telemetry.page.Page
    """
    del browser_info, page  # unused
    return True

  def _GetCurrentTab(self):
    try:
      return self.browser.tabs[0]
    # The tab may have gone away in some case, so we create a new tab and retry
    # (See crbug.com/496280)
    except exceptions.DevtoolsTargetCrashException as e:
      logging.error('Tab may have crashed: %s' % str(e))
      self.browser.tabs.New()
      # See below in WillRunStory for why this waiting is needed.
      self.browser.tabs[0].WaitForDocumentReadyStateToBeComplete()
      return self.browser.tabs[0]

  def _PreparePage(self):
    self._current_tab = self._GetCurrentTab()
    if self._current_page.is_file:
      self.platform.SetHTTPServerDirectories(
          self._current_page.story_set.serving_dirs
          | {self._current_page.serving_dir},
          self._current_page.story_set.request_handler_class)

  @property
  def current_page(self):
    return self._current_page

  @property
  def current_tab(self):
    return self._current_tab

  def NavigateToPage(self, action_runner, page):
    # Method called by page.Run(), lives in shared_state to avoid exposing
    # references to the legacy self._page_test object.
    if self._page_test:
      self._page_test.WillNavigateToPage(page, action_runner.tab)
    with self.interval_profiling_controller.SamplePeriod(
        'navigation', action_runner):
      page.RunNavigateSteps(action_runner)
    if self._page_test:
      self._page_test.DidNavigateToPage(page, action_runner.tab)

  def RunPageInteractions(self, action_runner, page):
    # The purpose is similar to NavigateToPage.
    with self.interval_profiling_controller.SamplePeriod(
        'interactions', action_runner):
      page.RunPageInteractions(action_runner)
      if self._page_test:
        self._page_test.ValidateAndMeasurePage(
            page, action_runner.tab, self._page_test_results)

  def RunStory(self, results):
    self._PreparePage()
    self._page_test_results = results
    self._current_page.Run(self)
    self._page_test_results = None

  def TearDownState(self):
    self._StopBrowser()
    self.platform.StopAllLocalServers()
    self.platform.network_controller.Close()
    if self._perf_mode_set:
      self.platform.SetPerformanceMode(android_device.NORMAL_PERFORMANCE_MODE)

  def _StopBrowser(self):
    if self._browser:
      self._browser.Close()
      self._browser = None
    if self._possible_browser:
      self._possible_browser.CleanUpEnvironment()


class SharedMobilePageState(SharedPageState):
  _device_type = 'mobile'


class SharedDesktopPageState(SharedPageState):
  _device_type = 'desktop'


class SharedTabletPageState(SharedPageState):
  _device_type = 'tablet'


class Shared10InchTabletPageState(SharedPageState):
  _device_type = 'tablet_10_inch'
