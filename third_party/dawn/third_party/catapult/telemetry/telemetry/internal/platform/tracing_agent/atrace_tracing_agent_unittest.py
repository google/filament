# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
from unittest import mock

from systrace.tracing_agents import atrace_agent as systrace_atrace_agent
from telemetry.core import exceptions
from telemetry.internal.platform.tracing_agent import atrace_tracing_agent
from tracing.trace_data import trace_data


class AtraceTracingAgentTest(unittest.TestCase):
  def setUp(self):
    self._mock_platform_backend = mock.NonCallableMagicMock()
    self._mock_config = mock.NonCallableMagicMock()

  @mock.patch.object(systrace_atrace_agent.AtraceAgent, 'GetResults')
  def testCollectAgentTraceDataEmptyTrace(self, mock_get_results):
    # Make GetResults() in the mock systrace atrace agent return an empty
    # trace.
    empty_atrace_output = '# tracer: nop'
    mock_get_results.return_value.raw_data = empty_atrace_output
    atrace_agent = atrace_tracing_agent.AtraceTracingAgent(
        self._mock_platform_backend, self._mock_config)

    mock_trace_builder = mock.NonCallableMagicMock(spec=['AddTraceFor'])
    atrace_agent.CollectAgentTraceData(mock_trace_builder)
    mock_trace_builder.AddTraceFor.assert_called_once_with(
        trace_data.ATRACE_PART, empty_atrace_output, allow_unstructured=True)

  @mock.patch.object(systrace_atrace_agent.AtraceAgent, 'GetResults')
  def testCollectAgentTraceDataTimeout(self, mock_get_results):
    # Make GetResults() in the mock systrace atrace agent return false to
    # simulate a timeout happening inside the Systrace Atrace agent.
    mock_get_results.return_value = False
    atrace_agent = atrace_tracing_agent.AtraceTracingAgent(
        self._mock_platform_backend, self._mock_config)

    mock_trace_builder = mock.NonCallableMagicMock(spec=['AddTraceFor'])
    with self.assertRaises(exceptions.AtraceTracingError):
      atrace_agent.CollectAgentTraceData(mock_trace_builder)
    mock_trace_builder.AddTraceFor.assert_not_called()
