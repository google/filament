# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import posixpath

from telemetry.internal.platform import tracing_agent
from telemetry.internal.util import binary_manager

from devil.android import device_signal
from devil.android import device_temp_file
from devil.android.sdk import version_codes
import py_utils
from tracing.trace_data import trace_data


ANDROID_TRACES_DIR = '/data/misc/perfetto-traces'
ANDROID_TMP_DIR = '/data/local/tmp'
TRACED = 'traced'
TRACED_PROBES = 'traced_probes'
PERFETTO = 'perfetto'
STOP_TIMEOUT = 60

class PerfettoTracingAgent(tracing_agent.TracingAgent):
  def __init__(self, platform_backend, config):
    super().__init__(platform_backend, config)
    self._device = platform_backend.device
    self._trace_config_temp_file = None
    self._trace_output_temp_file = None
    self._perfetto_pid = None
    self._perfetto_path = PERFETTO

    if (self._device.build_version_sdk < version_codes.PIE or
        config.force_sideload_perfetto):
      logging.info('Sideloading perfetto binaries to the device.')

      if not self._device.PathExists(ANDROID_TRACES_DIR, as_root=True):
        self._device.RunShellCommand(['mkdir', '-p', ANDROID_TRACES_DIR])

      # Stop system perfetto service if there is one.
      self._device.RunShellCommand(['setprop', 'ctl.stop', TRACED])
      self._device.RunShellCommand(['setprop', 'ctl.stop', TRACED_PROBES])
      for service in [TRACED, TRACED_PROBES]:
        self._device.KillAll(service, exact=True, quiet=True,
                             signum=device_signal.SIGTERM, timeout=STOP_TIMEOUT,
                             as_root=True)

      self._PushFilesAndStartService(platform_backend.GetArchName())

    processes = set(p.name for p in self._device.ListProcesses())
    assert TRACED in processes
    assert TRACED_PROBES in processes
    logging.info('Perfetto tracing agent is set up.')

  def _PushFilesAndStartService(self, arch_name):
    perfetto_device_path = posixpath.join(ANDROID_TMP_DIR, PERFETTO)
    traced_device_path = posixpath.join(ANDROID_TMP_DIR, TRACED)
    traced_probes_device_path = posixpath.join(ANDROID_TMP_DIR, TRACED_PROBES)
    perfetto_local_path = binary_manager.FetchPath(
        'perfetto_monolithic_perfetto', 'android', arch_name)
    traced_local_path = binary_manager.FetchPath(
        'perfetto_monolithic_traced', 'android', arch_name)
    traced_probes_local_path = binary_manager.FetchPath(
        'perfetto_monolithic_traced_probes', 'android', arch_name)
    self._device.PushChangedFiles([
        (perfetto_local_path, perfetto_device_path),
        (traced_local_path, traced_device_path),
        (traced_probes_local_path, traced_probes_device_path),
    ])
    in_background = '</dev/null >/dev/null 2>&1 &'
    self._device.RunShellCommand(traced_device_path + in_background,
                                 shell=True)
    self._device.RunShellCommand(traced_probes_device_path + in_background,
                                 shell=True)
    self._perfetto_path = perfetto_device_path

  @classmethod
  def IsSupported(cls, platform_backend):
    return platform_backend.GetOSName() == 'android'

  def _TempFile(self, **kwargs):
    return device_temp_file.DeviceTempFile(self._device.adb, **kwargs)

  def StartAgentTracing(self, config, timeout):
    del timeout  # Unused.
    self._trace_config_temp_file = self._TempFile(suffix='.txt',
                                                  dir=ANDROID_TMP_DIR)
    self._trace_output_temp_file = self._TempFile(suffix='.pftrace',
                                                  dir=ANDROID_TRACES_DIR)
    text_config = config.system_trace_config.GetTextConfig()
    self._device.WriteFile(self._trace_config_temp_file.name, text_config)
    start_perfetto = (
        'cat %s | %s --background --config - --txt --out %s' % (
            self._trace_config_temp_file.name,
            self._perfetto_path,
            self._trace_output_temp_file.name,
        )
    )
    stdout = self._device.RunShellCommand(start_perfetto, shell=True)
    self._perfetto_pid = int(stdout[0])
    logging.info('Started perfetto with pid %s.', self._perfetto_pid)
    return True

  def StopAgentTracing(self):
    self._device.RunShellCommand(['kill', str(self._perfetto_pid)])

    def PerfettoStopped():
      for p in self._device.ListProcesses(PERFETTO):
        if p.pid == self._perfetto_pid:
          return False
      return True
    py_utils.WaitFor(PerfettoStopped, timeout=STOP_TIMEOUT)
    logging.info('Stopped Perfetto system tracing.')

    # Shut down the sideloaded service and restart the system one.
    for service in [TRACED, TRACED_PROBES]:
      self._device.KillAll(service, exact=True, blocking=True,
                           signum=device_signal.SIGTERM, timeout=STOP_TIMEOUT,
                           as_root=True)
    self._device.RunShellCommand(['setprop', 'ctl.start', TRACED])
    self._device.RunShellCommand(['setprop', 'ctl.start', TRACED_PROBES])

  def CollectAgentTraceData(self, trace_data_builder, timeout=300):
    with trace_data_builder.OpenTraceHandleFor(
        trace_data.CHROME_TRACE_PART, suffix='.pb') as handle:
      pass

    self._device.PullFile(
        self._trace_output_temp_file.name, handle.name, timeout=timeout)
    self._trace_config_temp_file.close()
    self._trace_output_temp_file.close()
    self._trace_config_temp_file = None
    self._trace_output_temp_file = None
    self._perfetto_pid = None
    logging.info('Collected trace from Perfetto system tracing.')
