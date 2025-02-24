# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import six

from systrace.tracing_agents import atrace_agent
from telemetry.core import exceptions
from telemetry.internal.platform import tracing_agent
from tracing.trace_data import trace_data

from devil.android.sdk import version_codes


class AtraceTracingAgent(tracing_agent.TracingAgent):
  def __init__(self, platform_backend, config):
    super().__init__(platform_backend, config)
    self._device = platform_backend.device
    self._categories = None
    self._atrace_agent = atrace_agent.AtraceAgent(
        platform_backend.device.build_version_sdk,
        platform_backend.device.tracing_path)
    self._config = None

  @classmethod
  def IsSupported(cls, platform_backend):
    return (platform_backend.GetOSName() == 'android' and
            platform_backend.device.build_version_sdk >
            version_codes.JELLY_BEAN_MR1)

  def StartAgentTracing(self, config, timeout):
    if not config.enable_atrace_trace:
      return False

    app_name = (','.join(config.atrace_config.app_name) if isinstance(
        config.atrace_config.app_name, list) else config.atrace_config.app_name)
    self._config = atrace_agent.AtraceConfig(
        config.atrace_config.categories,
        trace_buf_size=None, kfuncs=None, app_name=app_name,
        compress_trace_data=True, from_file=True,
        device_serial_number=str(self._device), trace_time=None,
        target='android')
    return self._atrace_agent.StartAgentTracing(self._config, timeout)

  def StopAgentTracing(self):
    self._atrace_agent.StopAgentTracing()

  def SupportsExplicitClockSync(self):
    return self._atrace_agent.SupportsExplicitClockSync()

  def RecordClockSyncMarker(self, sync_id,
                            record_controller_clocksync_marker_callback):
    return self._atrace_agent.RecordClockSyncMarker(
        sync_id, lambda t, sid: record_controller_clocksync_marker_callback(
            sid, t))

  def CollectAgentTraceData(self, trace_data_builder, timeout=None):
    results = self._atrace_agent.GetResults(timeout)
    if results is False:
      raise exceptions.AtraceTracingError(
          'Timed out retrieving the atrace tracing data from device %s.'
          % self._device)
    trace_data_builder.AddTraceFor(
        trace_data.ATRACE_PART, six.ensure_str(results.raw_data),
        allow_unstructured=True)
