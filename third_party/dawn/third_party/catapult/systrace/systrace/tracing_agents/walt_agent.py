# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
# pylint: disable=deprecated-module
import optparse
import threading

import py_utils

from devil.android import device_utils
from systrace import trace_result
from systrace import tracing_agents
from py_trace_event import trace_time as trace_time_module

TRACE_FILE_PATH = \
    '/sdcard/Android/data/org.chromium.latency.walt/files/trace.txt'

CLOCK_DOMAIN_MARKER = '# clock_type=LINUX_CLOCK_MONOTONIC\n'


def try_create_agent(options):
  if options.is_walt_enabled:
    return WaltAgent()
  return None


class WaltConfig(tracing_agents.TracingConfig):
  def __init__(self, device_serial_number, is_walt_enabled):
    tracing_agents.TracingConfig.__init__(self)
    self.device_serial_number = device_serial_number
    self.is_walt_enabled = is_walt_enabled


def add_options(parser):
  # TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
  # pylint: disable=deprecated-module
  options = optparse.OptionGroup(parser, 'WALT trace options')
  options.add_option('--walt', dest='is_walt_enabled', default=False,
                    action='store_true', help='Use the WALT tracing agent. '
                    'WALT is a device for measuring latency of physical '
                    'sensors on phones and computers. '
                    'See https://github.com/google/walt')
  return options


def get_config(options):
  return WaltConfig(options.device_serial_number, options.is_walt_enabled)


class WaltAgent(tracing_agents.TracingAgent):
  """
  This tracing agent requires the WALT app to be installed on the Android phone,
  and requires the WALT device to be attached to the phone. WALT is a device
  for measuring latency of physical sensors and outputs on phones and
  computers. For more information, visit https://github.com/google/walt
  """
  def __init__(self):
    super().__init__()
    self._trace_contents = None
    self._config = None
    self._device_utils = None
    self._clock_sync_marker = None
    self._collection_thread = None

  def __repr__(self):
    return 'WaltAgent'

  @py_utils.Timeout(tracing_agents.START_STOP_TIMEOUT)
  def StartAgentTracing(self, config, timeout=None):
    del timeout  # unused
    self._config = config
    self._device_utils = device_utils.DeviceUtils(
        self._config.device_serial_number)
    if self._device_utils.PathExists(TRACE_FILE_PATH):
      # clear old trace events so they are not included in the current trace
      self._device_utils.WriteFile(TRACE_FILE_PATH, '')
    return True

  @py_utils.Timeout(tracing_agents.START_STOP_TIMEOUT)
  def StopAgentTracing(self, timeout=None):
    """Stops tracing and starts collecting results.

    To synchronously retrieve the results after calling this function,
    call GetResults().
    """
    del timeout  # unused
    self._collection_thread = threading.Thread(
        target=self._collect_trace_data)
    self._collection_thread.start()
    return True

  def _collect_trace_data(self):
    self._trace_contents = self._device_utils.ReadFile(TRACE_FILE_PATH)

  def SupportsExplicitClockSync(self):
    return True

  # TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
  # pylint: disable=arguments-differ
  def RecordClockSyncMarker(self, sync_id, did_record_clock_sync_callback):
    cmd = 'cat /proc/timer_list | grep now'
    t1 = trace_time_module.Now()
    command_result = self._device_utils.RunShellCommand(cmd, shell=True)
    nsec = command_result[0].split()[2]
    self._clock_sync_marker = format_clock_sync_marker(sync_id, nsec)
    did_record_clock_sync_callback(t1, sync_id)

  @py_utils.Timeout(tracing_agents.GET_RESULTS_TIMEOUT)
  def GetResults(self, timeout=None):
    del timeout  # unused
    self._collection_thread.join()
    self._collection_thread = None
    return trace_result.TraceResult('waltTrace', self._get_trace_result())

  def _get_trace_result(self):
    result = '# tracer: \n' + CLOCK_DOMAIN_MARKER + self._trace_contents
    if self._clock_sync_marker is not None:
      result += self._clock_sync_marker
    return result


def format_clock_sync_marker(sync_id, nanosec_time):
  return ('<0>-0  (-----) [001] ...1  ' + str(float(nanosec_time) / 1e9)
          + ': tracing_mark_write: trace_event_clock_sync: name='
          + sync_id + '\n')
