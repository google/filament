# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
from unittest import mock

from telemetry import decorators
from telemetry.internal.browser import browser_finder
from telemetry.testing import options_for_unittests
from telemetry.testing import tab_test_case
from telemetry.timeline import tracing_config
from telemetry.util import trace_processor


class TracingControllerTest(tab_test_case.TabTestCase):
  """Tests that start tracing when a browser tab is already active."""

  def setUp(self):
    super().setUp()
    self.config = tracing_config.TracingConfig()
    self.config.enable_chrome_trace = True

  @property
  def tracing_controller(self):
    return self._tab.browser.platform.tracing_controller

  def AddTimelineMarker(self, title):
    self._tab.AddTimelineMarker(title)

  @decorators.Isolated
  def testExceptionDuringStopTracingIsCaught(self):
    self.tracing_controller.StartTracing(self.config)
    self.assertTrue(self.tracing_controller.is_tracing_running)

    # Inject exception while trying to stop Chrome tracing. It should be
    # caught and buffered until the trace builder is cleaned up.
    with mock.patch.object(
        self._tab._inspector_backend._devtools_client,
        'StopChromeTracing',
        side_effect=Exception('Intentional Tracing Exception')):
      trace_builder = self.tracing_controller.StopTracing()

    # Tracing is stopped even if there was an exception.
    self.assertFalse(self.tracing_controller.is_tracing_running)

    # Cleaning up the builder raises the exception.
    with self.assertRaisesRegex(Exception, 'Intentional Tracing Exception'):
      trace_builder.CleanUpTraceData()

  @decorators.Isolated
  def testGotTrace(self):
    self.tracing_controller.StartTracing(self.config)
    self.assertTrue(self.tracing_controller.is_tracing_running)

    self.AddTimelineMarker('trace-event')

    trace_data = self.tracing_controller.StopTracing()
    self.assertFalse(self.tracing_controller.is_tracing_running)

    markers = trace_processor.ExtractTimelineMarkers(trace_data)
    self.assertIn('trace-event', markers)

  @decorators.Isolated
  def testGotClockSyncMarkers(self):
    self.tracing_controller.StartTracing(self.config)
    self.assertTrue(self.tracing_controller.is_tracing_running)
    trace_data = self.tracing_controller.StopTracing()
    self.assertFalse(self.tracing_controller.is_tracing_running)

    complete_sync_ids = trace_processor.ExtractCompleteSyncIds(trace_data)
    self.assertEqual(len(complete_sync_ids), 1)

  @decorators.Isolated
  def testStartAndStopTraceMultipleTimes(self):
    self.tracing_controller.StartTracing(self.config)
    self.assertTrue(self.tracing_controller.is_tracing_running)

    # Calling StartTracing again does nothing.
    self.assertFalse(self.tracing_controller.StartTracing(self.config))
    self.assertTrue(self.tracing_controller.is_tracing_running)

    self.AddTimelineMarker('trace-event')

    trace_data = self.tracing_controller.StopTracing()
    self.assertFalse(self.tracing_controller.is_tracing_running)

    markers = trace_processor.ExtractTimelineMarkers(trace_data)
    self.assertIn('trace-event', markers)

    # Calling StopTracing again will raise an exception.
    with self.assertRaises(Exception):
      self.tracing_controller.StopTracing()

  @decorators.Isolated
  @decorators.Disabled('mac')
  def testFlushTracing(self):
    self.tracing_controller.StartTracing(self.config)
    self.assertTrue(self.tracing_controller.is_tracing_running)

    self.AddTimelineMarker('before-flush')

    self.tracing_controller.FlushTracing()
    self.assertTrue(self.tracing_controller.is_tracing_running)

    self.AddTimelineMarker('after-flush')

    trace_data = self.tracing_controller.StopTracing()
    self.assertFalse(self.tracing_controller.is_tracing_running)

    # Both markers before and after flushing are found.
    markers = trace_processor.ExtractTimelineMarkers(trace_data)
    self.assertIn('before-flush', markers)
    self.assertIn('after-flush', markers)

  @decorators.Isolated
  @decorators.Disabled('win')  # https://crbug.com/957831
  def testFlushTracingDiscardCurrent(self):
    self.tracing_controller.StartTracing(self.config)
    self.assertTrue(self.tracing_controller.is_tracing_running)

    self.AddTimelineMarker('before-flush')

    self.tracing_controller.FlushTracing(discard_current=True)
    self.assertTrue(self.tracing_controller.is_tracing_running)

    self.AddTimelineMarker('after-flush')

    trace_data = self.tracing_controller.StopTracing()
    self.assertFalse(self.tracing_controller.is_tracing_running)

    # The marker after flushing should be found, but not the one before.
    markers = trace_processor.ExtractTimelineMarkers(trace_data)
    self.assertIn('after-flush', markers)
    self.assertNotIn('before-flush', markers)


class StartupTracingTest(unittest.TestCase):
  """Tests that start tracing before the browser is created."""

  def setUp(self):
    finder_options = options_for_unittests.GetCopy()
    self.possible_browser = browser_finder.FindBrowser(finder_options)
    if not self.possible_browser:
      raise Exception('No browser found, cannot continue test.')
    self.browser_options = finder_options.browser_options
    self.config = tracing_config.TracingConfig()
    self.config.enable_chrome_trace = True

  def tearDown(self):
    if self.possible_browser and self.tracing_controller.is_tracing_running:
      self.tracing_controller.StopTracing()

  @property
  def tracing_controller(self):
    return self.possible_browser.platform.tracing_controller

  def StopTracingAndGetTimelineMarkers(self):
    self.assertTrue(self.tracing_controller.is_tracing_running)
    trace_data = self.tracing_controller.StopTracing()
    self.assertFalse(self.tracing_controller.is_tracing_running)
    return trace_processor.ExtractTimelineMarkers(trace_data)

  @decorators.Isolated
  @decorators.Disabled('chromeos')  # https://crbug.com/920454
  @decorators.Disabled('win')  # https://crbug.com/1220402
  def testStopTracingWhileBrowserIsRunning(self):
    self.tracing_controller.StartTracing(self.config)
    with self.possible_browser.BrowserSession(self.browser_options) as browser:
      browser.tabs[0].Navigate('about:blank')
      browser.tabs[0].WaitForDocumentReadyStateToBeInteractiveOrBetter()
      browser.tabs[0].AddTimelineMarker('trace-event')
      markers = self.StopTracingAndGetTimelineMarkers()
    self.assertIn('trace-event', markers)

  @decorators.Isolated
  @decorators.Disabled('chromeos')  # https://crbug.com/920454
  @decorators.Disabled('win')  # https://crbug.com/957831
  def testCloseBrowserBeforeTracingIsStopped(self):
    self.tracing_controller.StartTracing(self.config)
    with self.possible_browser.BrowserSession(self.browser_options) as browser:
      browser.tabs[0].Navigate('about:blank')
      browser.tabs[0].WaitForDocumentReadyStateToBeInteractiveOrBetter()
      browser.tabs[0].AddTimelineMarker('trace-event')
    markers = self.StopTracingAndGetTimelineMarkers()
    self.assertIn('trace-event', markers)

  @decorators.Isolated
  @decorators.Disabled('chromeos')  # https://crbug.com/920454
  @decorators.Disabled('win')  # https://crbug.com/957831
  def testRestartBrowserWhileTracing(self):
    expected_markers = ['trace-event-%i' % i for i in range(4)]
    self.tracing_controller.StartTracing(self.config)
    try:
      self.possible_browser.SetUpEnvironment(self.browser_options)
      for marker in expected_markers:
        with self.possible_browser.Create() as browser:
          browser.tabs[0].Navigate('about:blank')
          browser.tabs[0].WaitForDocumentReadyStateToBeInteractiveOrBetter()
          browser.tabs[0].AddTimelineMarker(marker)
    finally:
      self.possible_browser.CleanUpEnvironment()
    markers = self.StopTracingAndGetTimelineMarkers()
    for marker in expected_markers:
      self.assertIn(marker, markers)
