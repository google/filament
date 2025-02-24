# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Tracing agent that captures friendly process and thread data - names, pids and
# tids and names, etc to enrich display in the trace viewer. Captures snapshots
# of the output of 'ps' on the device at intervals.

import logging
import py_utils

from devil.android import device_utils
from devil.android.device_errors import AdbShellCommandFailedError
from systrace import tracing_agents
from systrace import trace_result

# Leftmost output columns match those used on legacy devices.
# Get thread names separately as there may be spaces that breaks col
# splitting.
# TODO(benm): Refactor device_utils.GetPids to get threads and use that here.
PS_COMMAND_PROC = "ps -A -o USER,PID,PPID,VSIZE,RSS,WCHAN,ADDR=PC,S,NAME,COMM" \
    "&& ps -AT -o USER,PID,TID,CMD"

# Fallback for old devices.
PS_COMMAND_PROC_LEGACY = "ps && ps -t"

# identify this as trace of thread / process state
TRACE_HEADER = 'PROCESS DUMP\n'

def try_create_agent(config):
  if config.target != 'android':
    return None
  if config.from_file is not None:
    return None
  if config.process_dump_enable:
    # Since AtraceProcessDumpAgent was enabled it's unnecessary to collect ps
    # data because each process memory dump updates information about processes
    # and their threads. It's more complete data than two ps snapshots for an
    # entire trace. However, that agent isn't enabled by default.
    return None
  return AndroidProcessDataAgent()

def get_config(options):
  return options

class AndroidProcessDataAgent(tracing_agents.TracingAgent):
  def __init__(self):
    super().__init__()
    self._trace_data = ""
    self._device = None

  def __repr__(self):
    return 'android_process_data'

  @py_utils.Timeout(tracing_agents.START_STOP_TIMEOUT)
  def StartAgentTracing(self, config, timeout=None):
    self._device = device_utils.DeviceUtils(config.device_serial_number)
    self._trace_data += self._get_process_snapshot()
    return True

  @py_utils.Timeout(tracing_agents.START_STOP_TIMEOUT)
  def StopAgentTracing(self, timeout=None):
    self._trace_data += self._get_process_snapshot()
    return True

  @py_utils.Timeout(tracing_agents.GET_RESULTS_TIMEOUT)
  def GetResults(self, timeout=None):
    result = TRACE_HEADER + self._trace_data
    return trace_result.TraceResult('androidProcessDump', result)

  def SupportsExplicitClockSync(self):
    return False

  def RecordClockSyncMarker(self, sync_id, did_record_sync_marker_callback):
    pass

  def _get_process_snapshot(self):
    use_legacy = False
    try:
      dump = self._device.RunShellCommand( \
          PS_COMMAND_PROC, check_return=True, as_root=True, shell=True)
    except AdbShellCommandFailedError:
      use_legacy = True

    # Check length of 2 as we execute two commands, which in case of failure
    # on old devices output 1 line each.
    if use_legacy or len(dump) == 2:
      logging.debug('Couldn\'t parse ps dump, trying legacy method ...')
      dump = self._device.RunShellCommand( \
          PS_COMMAND_PROC_LEGACY, check_return=True, as_root=True, shell=True)
      if len(dump) == 2:
        logging.error('Unable to extract process data!')
        return ""

    return '\n'.join(dump) + '\n'
