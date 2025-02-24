# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

class TracingController():

  def __init__(self, tracing_controller_backend):
    """Provides control of the tracing systems supported by Telemetry."""
    self._tracing_controller_backend = tracing_controller_backend

  def RecordBenchmarkMetadata(self, results):
    """Write benchmark metadata into the trace being currently recorded."""
    self._tracing_controller_backend.RecordBenchmarkMetadata(results)

  def StartTracing(self, tracing_config, timeout=20):
    """Starts tracing.

    tracing config contains both tracing options and category filters.

    trace_options specifies which tracing systems to activate. Category filter
    allows fine-tuning of the data that are collected by the selected tracing
    systems.

    Some tracers are process-specific, e.g. chrome tracing, but are not
    guaranteed to be supported. In order to support tracing of these kinds of
    tracers, Start will succeed *always*, even if the tracing systems you have
    requested are not supported.

    If you absolutely require a particular tracer to exist, then check
    for its support after you have started the process in question. Or, have
    your code fail gracefully when the data you require is not present in the
    resulting trace.
    """
    self._tracing_controller_backend.StartTracing(tracing_config, timeout)

  def StopTracing(self):
    """Stops tracing and returns a TraceDataBuilder object
    """
    return self._tracing_controller_backend.StopTracing()

  def FlushTracing(self, discard_current=False):
    """Flush tracing buffer and continue tracing.

    Args:
      discard_current: optional bool, if True the current tracing data will
        be discarded before tracing continues.
    """
    self._tracing_controller_backend.FlushTracing(
        discard_current=discard_current)

  @property
  def is_tracing_running(self):
    return self._tracing_controller_backend.is_tracing_running

  def ClearStateIfNeeded(self):
    """Clear tracing state if needed."""
    self._tracing_controller_backend.ClearStateIfNeeded()
