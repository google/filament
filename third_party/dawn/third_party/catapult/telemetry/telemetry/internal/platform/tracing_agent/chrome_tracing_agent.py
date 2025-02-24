# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import sys
import traceback

from py_trace_event import trace_time

from telemetry.core import exceptions
from telemetry.internal.platform import tracing_agent
from telemetry.internal.platform.tracing_agent import (
    chrome_tracing_devtools_manager)


class ChromeTracingStartedError(exceptions.Error):
  pass


class ChromeTracingStoppedError(exceptions.Error):
  pass


class ChromeClockSyncError(exceptions.Error):
  pass


class ChromeTracingAgent(tracing_agent.TracingAgent):
  def __init__(self, platform_backend, config):
    super().__init__(platform_backend, config)
    self._trace_config = None
    self._trace_config_file = None
    self._previously_responsive_devtools = []

  @property
  def trace_config(self):
    # Trace config is also used to check if Chrome tracing is running or not.
    return self._trace_config

  @property
  def trace_config_file(self):
    return self._trace_config_file

  @classmethod
  def IsSupported(cls, platform_backend):
    raise NotImplementedError

  def _GetTransferMode(self):
    return None

  def _StartStartupTracing(self, config):
    raise NotImplementedError

  def _StartDevToolsTracing(self, config, timeout):
    devtools_clients = (
        chrome_tracing_devtools_manager
        .GetActiveDevToolsClients(self._platform_backend))
    if not devtools_clients:
      return False
    for client in devtools_clients:
      if not client.has_tracing_client:
        continue
      if client.is_tracing_running:
        raise ChromeTracingStartedError(
            'Tracing is already running on devtools at port %s on platform'
            'backend %s.' % (client.remote_port, self._platform_backend))
      client.StartChromeTracing(config, transfer_mode=self._GetTransferMode(),
                                timeout=timeout)
    return True

  def StartAgentTracing(self, config, timeout):
    if not config.enable_chrome_trace:
      return False

    if self._trace_config:
      raise ChromeTracingStartedError(
          'Tracing is already running on platform backend %s.'
          % self._platform_backend)

    if (config.enable_android_graphics_memtrack and
        self._platform_backend.GetOSName() == 'android'):
      self._platform_backend.SetGraphicsMemoryTrackingEnabled(True)

    # Chrome tracing Agent needs to start tracing for chrome browsers that are
    # not yet started, and for the ones that already are. For the former, we
    # first setup the trace_config_file, which allows browsers that starts after
    # this point to use it for enabling tracing upon browser startup. For the
    # latter, we invoke start tracing command through devtools for browsers that
    # are already started and tracked by chrome_tracing_devtools_manager.
    started_startup_tracing = self._StartStartupTracing(config)

    started_devtools_tracing = self._StartDevToolsTracing(config, timeout)
    if started_startup_tracing or started_devtools_tracing:
      self._trace_config = config
      return True
    return False

  def SupportsExplicitClockSync(self):
    return True

  def RecordClockSyncMarker(self, sync_id,
                            record_controller_clocksync_marker_callback):
    devtools_clients = (chrome_tracing_devtools_manager
                        .GetActiveDevToolsClients(self._platform_backend))
    if not devtools_clients:
      logging.info('No devtools clients for issuing clock sync.')
      return False

    has_clock_synced = False
    for client in devtools_clients:
      if not client.has_tracing_client:
        continue
      try:
        timestamp = trace_time.Now()
        client.RecordChromeClockSyncMarker(sync_id)
        # We only need one successful clock sync.
        has_clock_synced = True
        break
      except Exception: # pylint: disable=broad-except
        logging.exception('Failed to record clock sync marker with sync_id=%r '
                          'via DevTools client %r:', sync_id, client)
    if not has_clock_synced:
      raise ChromeClockSyncError(
          'Failed to issue clock sync to devtools client')
    record_controller_clocksync_marker_callback(sync_id, timestamp)
    return True

  def StopAgentTracing(self):
    if not self._trace_config:
      raise ChromeTracingStoppedError(
          'Tracing is not running on platform backend %s.'
          % self._platform_backend)
    self._RemoveTraceConfigFile()

    # We get all DevTools clients including the stale ones, so that we get an
    # exception if there is a stale client. This is because we will potentially
    # lose data if there is a stale client.
    # TODO(crbug.com/1029812): Check if this actually works. It looks like the
    # call to GetActiveDevToolsClients in RecordClockSyncMarker would have
    # wiped out the stale clients anyway.
    devtools_clients = (chrome_tracing_devtools_manager
                        .GetDevToolsClients(self._platform_backend))
    raised_exception_messages = []
    assert len(self._previously_responsive_devtools) == 0
    for client in devtools_clients:
      try:
        client.StopChromeTracing()
        self._previously_responsive_devtools.append(client)

      except Exception: # pylint: disable=broad-except
        raised_exception_messages.append(
            """Error when trying to stop Chrome tracing
            on devtools at port %s:\n%s"""
            % (client.remote_port,
               ''.join(traceback.format_exception(*sys.exc_info()))))

    if (self._trace_config.enable_android_graphics_memtrack and
        self._platform_backend.GetOSName() == 'android'):
      self._platform_backend.SetGraphicsMemoryTrackingEnabled(False)

    self._trace_config = None
    if raised_exception_messages:
      raise ChromeTracingStoppedError(
          'Exceptions raised when trying to stop Chrome devtool tracing:\n' +
          '\n'.join(raised_exception_messages))

  def _RemoveTraceConfigFile(self):
    raise NotImplementedError

  def CollectAgentTraceData(self, trace_data_builder, timeout=None):
    raised_exception_messages = []
    for client in self._previously_responsive_devtools:
      try:
        client.CollectChromeTracingData(trace_data_builder)
      except Exception: # pylint: disable=broad-except
        raised_exception_messages.append(
            'Error when collecting Chrome tracing on devtools at port %s:\n%s' %
            (client.remote_port,
             ''.join(traceback.format_exception(*sys.exc_info()))))
    self._previously_responsive_devtools = []

    if raised_exception_messages:
      raise ChromeTracingStoppedError(
          'Exceptions raised when trying to collect Chrome devtool tracing:\n' +
          '\n'.join(raised_exception_messages))

  def SupportsFlushingAgentTracing(self):
    return True

  def FlushAgentTracing(self, config, timeout, trace_data_builder):
    if not self._trace_config:
      raise ChromeTracingStoppedError(
          'Tracing is not running on platform backend %s.'
          % self._platform_backend)

    for backend in self._IterFirstTabBackends():
      backend.EvaluateJavaScript("console.time('flush-tracing');")

    self.StopAgentTracing()
    self.CollectAgentTraceData(trace_data_builder)
    self.StartAgentTracing(config, timeout)

    for backend in self._IterFirstTabBackends():
      backend.EvaluateJavaScript("console.timeEnd('flush-tracing');")

  def _IterFirstTabBackends(self):
    for client in chrome_tracing_devtools_manager.GetDevToolsClients(
        self._platform_backend):
      backend = client.FirstTabBackend()
      if backend is not None:
        yield backend
