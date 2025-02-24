# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest

from telemetry.timeline import chrome_trace_category_filter
from telemetry.timeline import chrome_trace_config


class ChromeTraceConfigTests(unittest.TestCase):
  def testDefault(self):
    config = chrome_trace_config.ChromeTraceConfig()

    # Trace config for startup tracing.
    self.assertEqual({'record_mode': 'record-continuously'},
                     config.GetChromeTraceConfigForStartupTracing())

    # Trace config for DevTools (modern API).
    self.assertEqual({'recordMode': 'recordContinuously'},
                     config.GetChromeTraceConfigForDevTools())

  def testBasic(self):
    category_filter = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        'x,-y,disabled-by-default-z,DELAY(7;foo)')
    config = chrome_trace_config.ChromeTraceConfig()
    config.SetCategoryFilter(category_filter)
    config.record_mode = chrome_trace_config.RECORD_UNTIL_FULL

    # Trace config for startup tracing.
    self.assertEqual(
        {
            'excluded_categories': ['y'],
            'included_categories': sorted(['x', 'disabled-by-default-z']),
            'record_mode': 'record-until-full',
            'synthetic_delays': ['DELAY(7;foo)']
        }, config.GetChromeTraceConfigForStartupTracing())

    # Trace config for DevTools (modern API).
    self.assertEqual(
        {
            'excludedCategories': ['y'],
            'includedCategories': sorted(['x', 'disabled-by-default-z']),
            'recordMode': 'recordUntilFull',
            'syntheticDelays': ['DELAY(7;foo)']
        }, config.GetChromeTraceConfigForDevTools())

    # Test correct modification of config after enabling systrace.
    config.SetEnableSystrace()
    # Test enable systrace with trace config for startup tracing.
    self.assertEqual(
        {
            'excluded_categories': ['y'],
            'included_categories': sorted(['x', 'disabled-by-default-z']),
            'record_mode': 'record-until-full',
            'synthetic_delays': ['DELAY(7;foo)'],
            'enable_systrace': True
        }, config.GetChromeTraceConfigForStartupTracing())

    # And test again with modern API.
    self.assertEqual(
        {
            'excludedCategories': ['y'],
            'includedCategories': sorted(['x', 'disabled-by-default-z']),
            'recordMode': 'recordUntilFull',
            'syntheticDelays': ['DELAY(7;foo)'],
            'enableSystrace': True
        }, config.GetChromeTraceConfigForDevTools())


  def testMemoryDumpConfigFormat(self):
    config = chrome_trace_config.ChromeTraceConfig()
    config.record_mode = chrome_trace_config.ECHO_TO_CONSOLE
    dump_config = chrome_trace_config.MemoryDumpConfig()
    config.SetMemoryDumpConfig(dump_config)

    # Trace config for startup tracing.
    self.assertEqual(
        {
            'memory_dump_config': {
                'triggers': []
            },
            'record_mode': 'trace-to-console'
        }, config.GetChromeTraceConfigForStartupTracing())

    # Trace config for DevTools (modern API).
    self.assertEqual(
        {
            'memoryDumpConfig': {
                'triggers': []
            },
            'recordMode': 'traceToConsole'
        }, config.GetChromeTraceConfigForDevTools())

    dump_config.AddTrigger('light', 250)
    dump_config.AddTrigger('detailed', 2000)

    # Trace config for startup tracing.
    self.assertEqual(
        {
            'memory_dump_config': {
                'triggers': [{
                    'mode': 'light',
                    'periodic_interval_ms': 250
                }, {
                    'mode': 'detailed',
                    'periodic_interval_ms': 2000
                }]
            },
            'record_mode': 'trace-to-console'
        }, config.GetChromeTraceConfigForStartupTracing())

    # Trace config for DevTools (modern API).
    self.assertEqual(
        {
            'memoryDumpConfig': {
                'triggers': [{
                    'mode': 'light',
                    'periodicIntervalMs': 250
                }, {
                    'mode': 'detailed',
                    'periodicIntervalMs': 2000
                }]
            },
            'recordMode': 'traceToConsole'
        }, config.GetChromeTraceConfigForDevTools())

  def testUMAHistograms(self):
    config = chrome_trace_config.ChromeTraceConfig()
    config.EnableUMAHistograms('Event.Latency.ScrollUpdate.Touch.Metric1')
    self.assertEqual(
        {
            'histogramNames': ['Event.Latency.ScrollUpdate.Touch.Metric1'],
            'recordMode': 'recordContinuously'
        }, config.GetChromeTraceConfigForDevTools())

    config.EnableUMAHistograms('Event.Latency.ScrollUpdate.Touch.Metric2')
    self.assertEqual(
        {
            'histogramNames': [
                'Event.Latency.ScrollUpdate.Touch.Metric1',
                'Event.Latency.ScrollUpdate.Touch.Metric2'
            ],
            'recordMode':
            'recordContinuously'
        }, config.GetChromeTraceConfigForDevTools())

    config.EnableUMAHistograms('AnotherMetric', 'LastMetric')
    self.assertEqual(
        {
            'histogramNames': [
                'Event.Latency.ScrollUpdate.Touch.Metric1',
                'Event.Latency.ScrollUpdate.Touch.Metric2', 'AnotherMetric',
                'LastMetric'
            ],
            'recordMode':
            'recordContinuously'
        }, config.GetChromeTraceConfigForDevTools())

  def testTraceBufferSize(self):
    config = chrome_trace_config.ChromeTraceConfig()
    config.SetTraceBufferSizeInKb(42)
    self.assertEqual(
        {
            'recordMode': 'recordContinuously',
            'traceBufferSizeInKb': 42
        }, config.GetChromeTraceConfigForDevTools())
