# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import contextlib

from telemetry import decorators
from telemetry import page as page_module
from telemetry import story
from telemetry.internal.testing.test_page_sets import example_domain
from telemetry.page import cache_temperature
from telemetry.testing import browser_test_case
from telemetry.timeline import tracing_config
from telemetry.util import trace_processor


_TEST_URL = example_domain.HTTP_EXAMPLE


class CacheTemperatureTests(browser_test_case.BrowserTestCase):
  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    self.markers = None

  def setUp(self):
    super().setUp()
    self._browser.platform.network_controller.StartReplay(
        example_domain.FetchExampleDomainArchive())

  def tearDown(self):
    super().tearDown()
    self._browser.platform.network_controller.StopReplay()

  @contextlib.contextmanager
  def CaptureTraceMarkers(self):
    tracing_controller = self._browser.platform.tracing_controller
    options = tracing_config.TracingConfig()
    options.enable_chrome_trace = True
    tracing_controller.StartTracing(options)
    try:
      yield
    finally:
      trace_data = tracing_controller.StopTracing()
      self.markers = trace_processor.ExtractTimelineMarkers(trace_data)

  @decorators.Enabled('has tabs')
  @decorators.Disabled('chromeos')  # crbug.com/840033
  @decorators.Disabled('win')  # crbug.com/840033
  def testEnsureAny(self):
    with self.CaptureTraceMarkers():
      story_set = story.StorySet()
      page = page_module.Page(
          _TEST_URL, page_set=story_set,
          cache_temperature=cache_temperature.ANY, name=_TEST_URL)
      cache_temperature.EnsurePageCacheTemperature(page, self._browser)

    self.assertNotIn('telemetry.internal.ensure_diskcache.start', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.warm.start', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.warm.end', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.hot.start', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.hot.end', self.markers)

  @decorators.Enabled('has tabs')
  @decorators.Disabled('chromeos')  # crbug.com/840033
  @decorators.Disabled('win')  # crbug.com/840033
  def testEnsureCold(self):
    with self.CaptureTraceMarkers():
      story_set = story.StorySet()
      page = page_module.Page(
          _TEST_URL, page_set=story_set,
          cache_temperature=cache_temperature.COLD, name=_TEST_URL)
      cache_temperature.EnsurePageCacheTemperature(page, self._browser)

    self.assertIn('telemetry.internal.ensure_diskcache.start', self.markers)
    self.assertIn('telemetry.internal.ensure_diskcache.end', self.markers)

  @decorators.Disabled('reference')
  @decorators.Enabled('has tabs')
  @decorators.Disabled('chromeos')  # crbug.com/840033
  @decorators.Disabled('win')  # crbug.com/840033
  def testEnsureWarmAfterColdRun(self):
    with self.CaptureTraceMarkers():
      story_set = story.StorySet()
      page = page_module.Page(
          _TEST_URL, page_set=story_set,
          cache_temperature=cache_temperature.COLD, name=_TEST_URL)
      cache_temperature.EnsurePageCacheTemperature(page, self._browser)

      previous_page = page
      page = page_module.Page(
          _TEST_URL, page_set=story_set,
          cache_temperature=cache_temperature.WARM, name=_TEST_URL)
      cache_temperature.EnsurePageCacheTemperature(
          page, self._browser, previous_page)

    self.assertNotIn('telemetry.internal.warm_cache.warm.start', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.warm.end', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.hot.start', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.hot.end', self.markers)

  @decorators.Disabled('reference')
  @decorators.Enabled('has tabs')
  @decorators.Disabled('chromeos')  # crbug.com/840033
  @decorators.Disabled('win')  # crbug.com/840033
  def testEnsureWarmFromScratch(self):
    with self.CaptureTraceMarkers():
      story_set = story.StorySet()
      page = page_module.Page(
          _TEST_URL, page_set=story_set,
          cache_temperature=cache_temperature.WARM, name=_TEST_URL)
      cache_temperature.EnsurePageCacheTemperature(page, self._browser)

    self.assertIn('telemetry.internal.warm_cache.warm.start', self.markers)
    self.assertIn('telemetry.internal.warm_cache.warm.end', self.markers)

  @decorators.Disabled('reference')
  @decorators.Enabled('has tabs')
  @decorators.Disabled('all')  # crbug.com/840033 crbug.com/960552
  def testEnsureHotAfterColdAndWarmRun(self):
    with self.CaptureTraceMarkers():
      story_set = story.StorySet()
      page = page_module.Page(
          _TEST_URL, page_set=story_set,
          cache_temperature=cache_temperature.COLD, name=_TEST_URL)
      cache_temperature.EnsurePageCacheTemperature(page, self._browser)

      previous_page = page
      page = page_module.Page(
          _TEST_URL, page_set=story_set,
          cache_temperature=cache_temperature.WARM, name=_TEST_URL)
      cache_temperature.EnsurePageCacheTemperature(
          page, self._browser, previous_page)

      previous_page = page
      page = page_module.Page(
          _TEST_URL, page_set=story_set,
          cache_temperature=cache_temperature.HOT, name=_TEST_URL)
      cache_temperature.EnsurePageCacheTemperature(
          page, self._browser, previous_page)

    self.assertNotIn('telemetry.internal.warm_cache.warm.start', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.warm.end', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.hot.start', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.hot.end', self.markers)

  @decorators.Disabled('reference')
  @decorators.Disabled('chromeos')  # crbug.com/840033
  @decorators.Disabled('win')  # crbug.com/840033
  def testEnsureHotAfterColdRun(self):
    with self.CaptureTraceMarkers():
      story_set = story.StorySet()
      page = page_module.Page(
          _TEST_URL, page_set=story_set,
          cache_temperature=cache_temperature.COLD, name=_TEST_URL)
      cache_temperature.EnsurePageCacheTemperature(page, self._browser)

      previous_page = page
      page = page_module.Page(
          _TEST_URL, page_set=story_set,
          cache_temperature=cache_temperature.HOT, name=_TEST_URL)
      cache_temperature.EnsurePageCacheTemperature(
          page, self._browser, previous_page)

    # After navigation for another origin url, traces in previous origin page
    # does not appear in |markers|, so we can not check this:
    # self.assertIn('telemetry.internal.warm_cache.hot.start', markers)
    # TODO: Ensure all traces are in |markers|
    self.assertIn('telemetry.internal.warm_cache.hot.end', self.markers)

  @decorators.Disabled('reference')
  @decorators.Enabled('has tabs')
  @decorators.Disabled('chromeos')  # crbug.com/840033
  @decorators.Disabled('win')  # crbug.com/840033
  def testEnsureHotFromScratch(self):
    with self.CaptureTraceMarkers():
      story_set = story.StorySet()
      page = page_module.Page(
          _TEST_URL, page_set=story_set,
          cache_temperature=cache_temperature.HOT, name=_TEST_URL)
      cache_temperature.EnsurePageCacheTemperature(page, self._browser)

    self.assertIn('telemetry.internal.warm_cache.warm.start', self.markers)
    self.assertIn('telemetry.internal.warm_cache.warm.end', self.markers)
    self.assertIn('telemetry.internal.warm_cache.hot.start', self.markers)
    self.assertIn('telemetry.internal.warm_cache.hot.end', self.markers)

  @decorators.Disabled('reference')
  @decorators.Enabled('has tabs')
  @decorators.Disabled('chromeos')  # crbug.com/840033
  @decorators.Disabled('win')       # crbug.com/840033
  @decorators.Disabled('linux')     # crbug.com/1394993
  @decorators.Disabled('mac')       # crbug.com/1394993
  def testEnsureWarmBrowser(self):
    with self.CaptureTraceMarkers():
      story_set = story.StorySet()
      page = page_module.Page(
          _TEST_URL, page_set=story_set,
          cache_temperature=cache_temperature.WARM_BROWSER,
          name=_TEST_URL)
      cache_temperature.EnsurePageCacheTemperature(
          page, self._browser)

    # Browser cache warming happens in a different tab so markers shouldn't
    # appear.
    self.assertNotIn('telemetry.internal.warm_cache.warm.start', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.warm.end', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.hot.start', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.hot.end', self.markers)

  @decorators.Disabled('reference')
  @decorators.Enabled('has tabs')
  @decorators.Disabled('chromeos')  # crbug.com/840033
  @decorators.Disabled('win')  # crbug.com/840033
  @decorators.Disabled('mac') # crbug.com/1394632
  @decorators.Disabled('linux') # crbug.com/1394632
  def testEnsureHotBrowser(self):
    with self.CaptureTraceMarkers():
      story_set = story.StorySet()
      page = page_module.Page(
          _TEST_URL, page_set=story_set,
          cache_temperature=cache_temperature.HOT_BROWSER,
          name=_TEST_URL)
      cache_temperature.EnsurePageCacheTemperature(
          page, self._browser)

    # Browser cache warming happens in a different tab so markers shouldn't
    # appear.
    self.assertNotIn('telemetry.internal.warm_cache.warm.start', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.warm.end', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.hot.start', self.markers)
    self.assertNotIn('telemetry.internal.warm_cache.hot.end', self.markers)
