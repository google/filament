# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.internal.platform import tracing_agent
from tracing.trace_data import trace_data


class DisplayTracingAgent(tracing_agent.TracingAgent):
  @classmethod
  def IsSupported(cls, platform_backend):
    return platform_backend.IsDisplayTracingSupported()

  def StartAgentTracing(self, config, timeout):
    del timeout  # unused
    if config.enable_platform_display_trace:
      self._platform_backend.StartDisplayTracing()
      return True
    return False

  def StopAgentTracing(self):
    # TODO: Split collection and stopping.
    pass

  def CollectAgentTraceData(self, trace_data_builder, timeout=None):
    # TODO: Move stopping to StopAgentTracing.
    del timeout
    surface_flinger_trace_data = self._platform_backend.StopDisplayTracing()
    trace_data_builder.AddTraceFor(
        trace_data.CHROME_TRACE_PART, surface_flinger_trace_data)
