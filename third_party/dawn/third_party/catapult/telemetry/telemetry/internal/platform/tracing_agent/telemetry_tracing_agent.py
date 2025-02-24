# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import sys
import tempfile

from telemetry.internal.platform import tracing_agent
from tracing.trace_data import trace_data

from py_trace_event import trace_event
from py_trace_event import trace_time

def IsAgentEnabled():
  """Returns True if the agent is currently enabled and tracing."""
  return trace_event.trace_is_enabled()


def RecordBenchmarkMetadata(results):
  """Record benchmark metadata if tracing is enabled."""
  if IsAgentEnabled():
    trace_event.trace_add_benchmark_metadata(
        benchmark_name=results.benchmark_name,
        benchmark_description=results.benchmark_description,
        benchmark_start_time_us=results.benchmark_start_us,
        label=results.label,
        story_run_time_us=results.current_story_run.start_us,
        story_name=results.current_story.name,
        story_tags=results.current_story.GetStoryTagsList(),
        story_run_index=results.current_story_run.index,
    )
  else:
    logging.warning(
        'Telemetry tracing agent is not enabled. Discarding Telemetry info.')


def RecordIssuerClockSyncMarker(sync_id, issue_ts):
  """Record clock sync event.

  Args:
    sync_id: Unique id for sync event.
    issue_ts: timestamp before issuing clock sync to agent.
  """
  trace_event.clock_sync(sync_id, issue_ts=issue_ts)


def _UsePerfettoSync(platform_backend, config):
  is_android = (platform_backend is not None
                and platform_backend.GetOSName() == 'android')
  if config is not None and config.enable_experimental_system_tracing:
    all_agents_support_proto = True
  else:
    all_agents_support_proto = config is not None and not (
        config.enable_atrace_trace or
        config.enable_cpu_trace or
        config.enable_platform_display_trace)
  return is_android and all_agents_support_proto


class TelemetryTracingAgent(tracing_agent.TracingAgent):
  """Tracing agent to collect data about Telemetry (and Python) execution.

  Is implemented as a thin wrapper around py_trace_event.trace_event. Clients
  can record events using the trace_event.trace() decorator or the
  trace_event.TracedMetaClass to trace all methods of a class.

  Also works as the clock sync recorder, against which other tracing agents
  can issue clock sync events. And is responsible for recording telemetry
  metadata with information e.g. about the benchmark that produced a trace.
  """
  def __init__(self, platform_backend, config):
    super().__init__(platform_backend, config)
    self._trace_file = None

    # When tracing Chrome on Android, Telemetry and the browser run on
    # different devices. The trace_set_clock_snapshot() function provides
    # a clock snapshot for the perfetto trace processor to synchronize
    # host and device clocks.
    if _UsePerfettoSync(platform_backend, config):
      telemetry_ts = trace_time.Now()
      device_uptime = platform_backend.device.RunShellCommand(
          ['cat', '/proc/uptime'], single_line=True)
      boottime_ts = float(device_uptime.split(' ')[0]) * 1e6
      trace_event.trace_set_clock_snapshot(
          telemetry_ts=telemetry_ts,
          boottime_ts=boottime_ts,
      )

  @classmethod
  def IsSupported(cls, platform_backend):
    del platform_backend  # Unused.
    return trace_event.is_tracing_controllable()

  @property
  def is_tracing(self):
    return IsAgentEnabled()

  def StartAgentTracing(self, config, timeout):
    del timeout  # Unused.
    assert not self.is_tracing, 'Telemetry tracing already running'

    # Keep Telemetry trace format in accordance with Chrome trace format.
    if config and config.chrome_trace_config.trace_format == 'proto':
      trace_format = trace_event.PROTOBUF
      suffix = '.pb'
    else:
      trace_format = trace_event.JSON_WITH_METADATA
      suffix = '.json'

    if sys.version_info.major < 3 or trace_format == trace_event.PROTOBUF:
      temp_file_mode = 'w+b'
    else:
      temp_file_mode = 'w+'

    # Create a temporary file and pass the opened file-like object to
    # trace_event.trace_enable(); the file will be closed on trace_disable(),
    # and later passed to a trace data builder in CollectAgentTraceData().
    self._trace_file = tempfile.NamedTemporaryFile(
        mode=temp_file_mode, delete=False, suffix=suffix)
    trace_event.trace_enable(self._trace_file, format=trace_format)
    assert self.is_tracing, 'Failed to start Telemetry tracing'
    return True

  def StopAgentTracing(self):
    assert self.is_tracing, 'Telemetry tracing is not running'
    trace_event.trace_disable()
    assert not self.is_tracing, 'Failed to stop Telemetry tracing'

  def CollectAgentTraceData(self, trace_data_builder, timeout=None):
    assert not self.is_tracing, 'Must stop tracing before collection'
    assert self._trace_file.closed, 'Trace file was not properly closed'
    # Ownership of the file, and responsibility to manage its lifetime, is now
    # transferred over to the trace data builder.
    trace_data_builder.AddTraceFileFor(
        trace_data.TELEMETRY_PART, self._trace_file.name)
    self._trace_file = None
