# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import unittest

from telemetry.internal.results import page_test_results
from telemetry.internal.platform.tracing_agent import telemetry_tracing_agent
from telemetry.testing import test_stories
from tracing.trace_data import trace_data

from py_trace_event import trace_event


def GetEventNames(trace):
  return (e['name'] for e in trace['traceEvents'])


@unittest.skipUnless(trace_event.is_tracing_controllable(),
                     'py_trace_event is not supported')
class TelemetryTracingAgentTest(unittest.TestCase):
  def setUp(self):
    platform = None  # Does not actually need one.
    self.config = None  # Does not actually need one.
    self.agent = telemetry_tracing_agent.TelemetryTracingAgent(
        platform, self.config)

  def tearDown(self):
    if self.agent.is_tracing:
      self.agent.StopAgentTracing()

  def testAddTraceEvent(self):
    self.agent.StartAgentTracing(self.config, timeout=10)
    with trace_event.trace('test-marker'):
      pass
    self.agent.StopAgentTracing()
    with trace_data.TraceDataBuilder() as builder:
      self.agent.CollectAgentTraceData(builder)
      trace = builder.AsData().GetTraceFor(trace_data.TELEMETRY_PART)
    self.assertIn('test-marker', GetEventNames(trace))

  def testRecordClockSync(self):
    self.agent.StartAgentTracing(self.config, timeout=10)
    telemetry_tracing_agent.RecordIssuerClockSyncMarker('1234', issue_ts=0)
    self.agent.StopAgentTracing()
    with trace_data.TraceDataBuilder() as builder:
      self.agent.CollectAgentTraceData(builder)
      trace = builder.AsData().GetTraceFor(trace_data.TELEMETRY_PART)
    self.assertIn('clock_sync', GetEventNames(trace))

  def testWriteBenchmarkMetadata(self):
    story = test_stories.DummyStory('story')
    with page_test_results.PageTestResults(
        benchmark_name='benchmark_name') as results:
      with results.CreateStoryRun(story):
        self.agent.StartAgentTracing(self.config, timeout=10)
        telemetry_tracing_agent.RecordBenchmarkMetadata(results)
        self.agent.StopAgentTracing()
        with trace_data.TraceDataBuilder() as builder:
          self.agent.CollectAgentTraceData(builder)
          trace = builder.AsData().GetTraceFor(trace_data.TELEMETRY_PART)
    benchmarks = trace['metadata']['telemetry']['benchmarks']
    self.assertEqual(len(benchmarks), 1)
    self.assertEqual(benchmarks[0], 'benchmark_name')
