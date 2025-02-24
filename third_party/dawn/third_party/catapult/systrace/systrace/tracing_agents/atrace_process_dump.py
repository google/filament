# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Tracing agent that captures periodic per-process memory dumps and other
# useful information from ProcFS like utime, stime, OOM stats, etc.

import json
import logging
# TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
# pylint: disable=deprecated-module
import optparse
import py_utils

from devil.android import device_utils
from devil.android.device_errors import AdbShellCommandFailedError
from py_trace_event import trace_time as trace_time_module
from systrace import tracing_agents
from systrace import trace_result

TRACE_HEADER = 'ATRACE_PROCESS_DUMP'
TRACE_RESULT_NAME = 'atraceProcessDump'

HELPER_COMMAND = '/data/local/tmp/atrace_helper'
HELPER_STOP_COMMAND = 'kill -TERM `pidof atrace_helper`'
HELPER_DUMP_JSON = '/data/local/tmp/procdump.json'


class AtraceProcessDumpAgent(tracing_agents.TracingAgent):
  def __init__(self):
    super().__init__()
    self._device = None
    self._dump = None
    self._clock_sync_markers = {}

  @py_utils.Timeout(tracing_agents.START_STOP_TIMEOUT)
  def StartAgentTracing(self, config, timeout=None):
    self._device = device_utils.DeviceUtils(config.device_serial_number)
    cmd = [HELPER_COMMAND, '-b', '-g',
        '-t', str(config.dump_interval_ms),
        '-o', HELPER_DUMP_JSON]
    if config.full_dump_config:
      cmd += ['-m', config.full_dump_config]
    if config.enable_mmaps:
      cmd += ['-s']
    self._device.RunShellCommand(cmd, check_return=True, as_root=True)
    return True

  @py_utils.Timeout(tracing_agents.START_STOP_TIMEOUT)
  def StopAgentTracing(self, timeout=None):
    self._device.RunShellCommand(
        HELPER_STOP_COMMAND,
        shell=True, check_return=True, as_root=True)
    try:
      self._device.RunShellCommand(['test', '-f', HELPER_DUMP_JSON],
          check_return=True, as_root=True)
      self._dump = self._device.ReadFile(HELPER_DUMP_JSON, force_pull=True)
      self._device.RunShellCommand(['rm', HELPER_DUMP_JSON],
          check_return=True, as_root=True)
    except AdbShellCommandFailedError:
      logging.error(
        'AtraceProcessDumpAgent failed to pull data. Check device storage.')
      return False
    return True

  @py_utils.Timeout(tracing_agents.GET_RESULTS_TIMEOUT)
  def GetResults(self, timeout=None):
    result = TRACE_HEADER + '\n' + self._dump
    cs = json.dumps(self._clock_sync_markers)
    result = TRACE_HEADER + \
        '\n{\"clock_sync_markers\":' + cs + ',\n\"dump\":' + self._dump + '}'
    return trace_result.TraceResult(TRACE_RESULT_NAME, result)

  def SupportsExplicitClockSync(self):
    return True

  def RecordClockSyncMarker(self, sync_id, did_record_sync_marker_callback):
    with self._device.adb.PersistentShell(self._device.serial) as shell:
      ts_in_controller_domain = trace_time_module.Now()
      output = shell.RunCommand(HELPER_COMMAND + ' --echo-ts', close=True)
      ts_in_agent_domain = int(output[0][0])
      self._clock_sync_markers[sync_id] = ts_in_agent_domain
      did_record_sync_marker_callback(ts_in_controller_domain, sync_id)


class AtraceProcessDumpConfig(tracing_agents.TracingConfig):
  def __init__(self, enabled, device_serial_number,
               dump_interval_ms, full_dump_config, enable_mmaps):
    tracing_agents.TracingConfig.__init__(self)
    self.enabled = enabled
    self.device_serial_number = device_serial_number
    self.dump_interval_ms = dump_interval_ms
    self.full_dump_config = full_dump_config
    self.enable_mmaps = enable_mmaps


def add_options(parser):
  # TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
  # pylint: disable=deprecated-module
  options = optparse.OptionGroup(parser, 'Atrace process dump options')
  options.add_option('--process-dump', dest='process_dump_enable',
                     default=False, action='store_true',
                     help='Capture periodic per-process memory dumps.')
  options.add_option('--process-dump-interval', dest='process_dump_interval_ms',
                     default=5000,
                     help='Interval between memory dumps in milliseconds.')
  options.add_option('--process-dump-full', dest='process_dump_full_config',
                     default=None,
                     help='Capture full memory dumps for some processes.\n' \
                          'Value: all, apps or comma-separated process names.')
  options.add_option('--process-dump-mmaps', dest='process_dump_mmaps',
                     default=False, action='store_true',
                     help='Capture VM regions and memory-mapped files.\n' \
                          'It increases dump size dramatically, hence only ' \
                          'has effect if --process-dump-full is a whitelist.')
  return options


def get_config(options):
  can_enable = (options.target == 'android') and (not options.from_file)
  return AtraceProcessDumpConfig(
    enabled=(options.process_dump_enable and can_enable),
    device_serial_number=options.device_serial_number,
    dump_interval_ms=options.process_dump_interval_ms,
    full_dump_config=options.process_dump_full_config,
    enable_mmaps=options.process_dump_mmaps
  )


def try_create_agent(config):
  if config.enabled:
    return AtraceProcessDumpAgent()
  return None
