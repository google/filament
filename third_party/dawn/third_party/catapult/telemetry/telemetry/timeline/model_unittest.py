# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest

from telemetry import decorators
from telemetry.testing import tab_test_case
from telemetry.timeline import model as timeline_model
from telemetry.timeline import tracing_config
from tracing.trace_data import trace_data


class TimelineModelUnittest(unittest.TestCase):
  def testEmptyImport(self):
    timeline_model.TimelineModel(trace_data.CreateFromRawChromeEvents([]))

  def testBrowserProcess(self):
    trace = trace_data.CreateFromRawChromeEvents([
        {
            "name": "process_name",
            "args": {"name": "Browser"},
            "pid": 5,
            "ph": "M"
        }, {
            "name": "thread_name",
            "args": {"name": "CrBrowserMain"},
            "pid": 5,
            "tid": 32578,
            "ph": "M"
        }])
    model = timeline_model.TimelineModel(trace)
    self.assertEqual(5, model.browser_process.pid)


class TimelineModelIntegrationTests(tab_test_case.TabTestCase):
  def setUp(self):
    super().setUp()
    self.tracing_controller = self._browser.platform.tracing_controller
    self.config = tracing_config.TracingConfig()
    self.config.chrome_trace_config.SetLowOverheadFilter()
    self.config.enable_chrome_trace = True

  def testGetTrace(self):
    self.tracing_controller.StartTracing(self.config)
    self.tabs[0].AddTimelineMarker('trace-event')
    trace = self.tracing_controller.StopTracing()
    model = timeline_model.TimelineModel(trace)
    markers = model.FindTimelineMarkers('trace-event')
    self.assertEqual(len(markers), 1)

  def testGetFirstRendererThread_singleTab(self):
    self.assertEqual(len(self.tabs), 1)  # We have a single tab/page.
    self.tracing_controller.StartTracing(self.config)
    self.tabs[0].AddTimelineMarker('single-tab-marker')
    trace = self.tracing_controller.StopTracing()
    model = timeline_model.TimelineModel(trace)

    # Check that we can find the marker injected into the trace.
    renderer_thread = model.GetFirstRendererThread(self.tabs[0].id)
    markers = list(renderer_thread.IterTimelineMarkers('single-tab-marker'))
    self.assertEqual(len(markers), 1)

  @decorators.Enabled('has tabs')
  def testGetFirstRendererThread_multipleTabs(self):
    # Make sure a couple of tabs exist.
    first_tab = self.tabs[0]
    second_tab = self.tabs.New()
    second_tab.Navigate('about:blank')
    second_tab.WaitForDocumentReadyStateToBeInteractiveOrBetter()

    self.tracing_controller.StartTracing(self.config)
    first_tab.AddTimelineMarker('background-tab')
    second_tab.AddTimelineMarker('foreground-tab')
    trace = self.tracing_controller.StopTracing()
    model = timeline_model.TimelineModel(trace)

    # Check that we can find the marker injected into the foreground tab.
    renderer_thread = model.GetFirstRendererThread(second_tab.id)
    markers = list(renderer_thread.IterTimelineMarkers([
        'foreground-tab', 'background-tab']))
    self.assertEqual(len(markers), 1)
    self.assertEqual(markers[0].name, 'foreground-tab')

    # Check that trying to find the background tab rases an error.
    with self.assertRaises(AssertionError):
      model.GetFirstRendererThread(first_tab.id)
