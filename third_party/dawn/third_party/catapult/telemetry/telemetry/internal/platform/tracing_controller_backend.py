# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import contextlib
import gc
import logging
import os
import sys
import traceback
import uuid

from py_trace_event import trace_event
from telemetry.core import exceptions
from telemetry.internal.platform.tracing_agent import atrace_tracing_agent
from telemetry.internal.platform.tracing_agent import chrome_report_events_tracing_agent
from telemetry.internal.platform.tracing_agent import chrome_return_as_stream_tracing_agent
from telemetry.internal.platform.tracing_agent import chrome_tracing_agent
from telemetry.internal.platform.tracing_agent import cpu_tracing_agent
from telemetry.internal.platform.tracing_agent import display_tracing_agent
from telemetry.internal.platform.tracing_agent import perfetto_tracing_agent
from telemetry.internal.platform.tracing_agent import telemetry_tracing_agent
from telemetry.timeline import tracing_config
from tracing.trace_data import trace_data


# Note: TelemetryTracingAgent should be first so that we can record debug
# trace events when the other agents start/stop.
_TRACING_AGENT_CLASSES = (
    telemetry_tracing_agent.TelemetryTracingAgent,
    chrome_report_events_tracing_agent.ChromeReportEventsTracingAgent,
    chrome_return_as_stream_tracing_agent.ChromeReturnAsStreamTracingAgent,
    atrace_tracing_agent.AtraceTracingAgent,
    cpu_tracing_agent.CpuTracingAgent,
    display_tracing_agent.DisplayTracingAgent
)

_EXPERIMENTAL_TRACING_AGENTS = (
    telemetry_tracing_agent.TelemetryTracingAgent,
    perfetto_tracing_agent.PerfettoTracingAgent
)


def _GenerateClockSyncId():
  return str(uuid.uuid4())


@contextlib.contextmanager
def _DisableGarbageCollection():
  try:
    gc.disable()
    yield
  finally:
    gc.enable()


class _TraceDataDiscarder():
  """A do-nothing data builder that just discards trace data.

  TODO(crbug.com/928278): This should be moved as a "discarding mode" in
  TraceDataBuilder itself.
  """
  def OpenTraceHandleFor(self, part, suffix):
    del part, suffix  # Unused.
    return open(os.devnull, 'wb')

  def AddTraceFor(self, part, data, allow_unstructured=False):
    assert not allow_unstructured
    del part  # Unused.
    del data  # Unused.

  def RecordTraceDataException(self):
    logging.info('Ignoring exception while flushing to TraceDataDiscarder:\n%s',
                 ''.join(traceback.format_exception(*sys.exc_info())))

  def IterTraceParts(self):
    yield '_TraceDataDiscarder', 'discarded'


class _TracingState():

  def __init__(self, config, timeout):
    self._builder = trace_data.TraceDataBuilder()
    self._config = config
    self._timeout = timeout

  @property
  def builder(self):
    return self._builder

  @property
  def config(self):
    return self._config

  @property
  def timeout(self):
    return self._timeout


class TracingControllerBackend():
  def __init__(self, platform_backend):
    self._platform_backend = platform_backend
    self._current_state = None
    self._active_agents_instances = []
    self._is_tracing_controllable = True

  def RecordBenchmarkMetadata(self, results):
    """Write benchmark metadata into the trace being currently recorded."""
    telemetry_tracing_agent.RecordBenchmarkMetadata(results)

  def StartTracing(self, config, timeout):
    if self.is_tracing_running:
      return False

    assert isinstance(config, tracing_config.TracingConfig)
    assert len(self._active_agents_instances) == 0

    self._current_state = _TracingState(config, timeout)

    if config.enable_experimental_system_tracing:
      agent_classes = _EXPERIMENTAL_TRACING_AGENTS
    else:
      agent_classes = _TRACING_AGENT_CLASSES

    for agent_class in agent_classes:
      if agent_class.IsSupported(self._platform_backend):
        agent = agent_class(self._platform_backend, config)
        if agent.StartAgentTracing(config, timeout):
          self._active_agents_instances.append(agent)

    return True

  def StopTracing(self):
    assert self.is_tracing_running, 'Can only stop tracing when tracing is on.'
    # Ideally, we would never get into a state where we're trying to issue a
    # clock sync marker when we can't. However, it's unclear if we can actually
    # ensure that we never get into that state. Since this seems to occur during
    # browser shutdown after a failed test, it seems like it should be safe to
    # continue stopping tracing even if this fails. See crbug.com/1320873 as an
    # example case of when this can happen.
    try:
      self._IssueClockSyncMarker()
    except exceptions.Error as e:
      logging.error(
          'Failed to issue clock sync marker during tracing shutdown: %s', e)
    builder = self._current_state.builder

    for agent in reversed(self._active_agents_instances):
      try:
        agent.StopAgentTracing()
      except Exception: # pylint: disable=broad-except
        builder.RecordTraceDataException()

    for agent in self._active_agents_instances:
      try:
        agent.CollectAgentTraceData(builder)
      except Exception: # pylint: disable=broad-except
        builder.RecordTraceDataException()

    self._active_agents_instances = []
    self._current_state = None

    return builder.Freeze()

  def FlushTracing(self, discard_current=False):
    assert self.is_tracing_running, 'Can only flush tracing when tracing is on.'
    # Similar to what we do in StopTracing, but if we fail to flush, then we
    # should turn off tracing since we're in a bad state.
    try:
      self._IssueClockSyncMarker()
    except exceptions.Error as e:
      logging.error(
          'Failed to issue clock sync marker during tracing flush: %s', e)
      logging.error('Forcibly stopping tracing due to above error.')
      self.StopTracing()
      return

    # See: https://github.com/PyCQA/pylint/issues/710
    if discard_current:
      trace_builder = _TraceDataDiscarder()
    else:
      trace_builder = self._current_state.builder

    for agent in self._active_agents_instances:
      try:
        if agent.SupportsFlushingAgentTracing():
          agent.FlushAgentTracing(self._current_state.config,
                                  self._current_state.timeout,
                                  trace_builder)
      except Exception: # pylint: disable=broad-except
        trace_builder.RecordTraceDataException()

  def _IssueClockSyncMarker(self):
    if not telemetry_tracing_agent.IsAgentEnabled():
      return

    with _DisableGarbageCollection():
      for agent in self._active_agents_instances:
        if agent.SupportsExplicitClockSync():
          sync_id = _GenerateClockSyncId()
          with trace_event.trace('RecordClockSyncMarker',
                                 agent=str(agent.__class__.__name__),
                                 sync_id=sync_id):
            agent.RecordClockSyncMarker(
                sync_id, telemetry_tracing_agent.RecordIssuerClockSyncMarker)

  @property
  def is_tracing_running(self):
    return self._current_state is not None

  @property
  def is_chrome_tracing_running(self):
    return self._GetActiveChromeTracingAgent() is not None

  @property
  def current_state(self):
    return self._current_state

  def _GetActiveChromeTracingAgent(self):
    if not self.is_tracing_running:
      return None
    if not self._current_state.config.enable_chrome_trace:
      return None
    for agent in self._active_agents_instances:
      if isinstance(agent, chrome_tracing_agent.ChromeTracingAgent):
        return agent
    return None

  def GetChromeTraceConfig(self):
    agent = self._GetActiveChromeTracingAgent()
    if agent:
      return agent.trace_config
    return None

  def GetChromeTraceConfigFile(self):
    agent = self._GetActiveChromeTracingAgent()
    if agent:
      return agent.trace_config_file
    return None

  def ClearStateIfNeeded(self):
    chrome_return_as_stream_tracing_agent.ClearStarupTracingStateIfNeeded(
        self._platform_backend)
