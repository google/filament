# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Tracing agent that captures cgroup information from /dev/cpuset on
# an Android device.

import stat
import py_utils

from devil.android import device_utils
from systrace import tracing_agents
from systrace import trace_result

# identify this as trace of cgroup state
# for now fake it as trace as no importer supports it
TRACE_HEADER = '# tracer: \nCGROUP DUMP\n'

def add_options(parser): # pylint: disable=unused-argument
  return None

def try_create_agent(config):
  if config.target != 'android':
    return None
  if not config.atrace_categories:
    return None
  # 'sched' contains cgroup events
  if 'sched' not in config.atrace_categories:
    return None
  if config.from_file is not None:
    return None
  return AndroidCgroupAgent()

def get_config(options):
  return options

def parse_proc_cgroups(cgroups, subsys):
  for line in cgroups.split('\n'):
    if line.startswith(subsys):
      return line.split()[1]
  return '-1'

class AndroidCgroupAgent(tracing_agents.TracingAgent):
  def __init__(self):
    super().__init__()
    self._config = None
    self._device_utils = None
    self._trace_data = ""

  def __repr__(self):
    return 'cgroup_data'

  @py_utils.Timeout(tracing_agents.START_STOP_TIMEOUT)
  def StartAgentTracing(self, config, timeout=None):
    self._config = config
    self._device_utils = device_utils.DeviceUtils(
        self._config.device_serial_number)

    if not self._device_utils.HasRoot():
      return False

    self._trace_data += self._get_cgroup_info()
    return True

  @py_utils.Timeout(tracing_agents.START_STOP_TIMEOUT)
  def StopAgentTracing(self, timeout=None):
    return True

  @py_utils.Timeout(tracing_agents.GET_RESULTS_TIMEOUT)
  def GetResults(self, timeout=None):
    result = TRACE_HEADER + self._trace_data
    return trace_result.TraceResult('cgroupDump', result)

  def SupportsExplicitClockSync(self):
    return False

  def RecordClockSyncMarker(self, sync_id, did_record_sync_marker_callback):
    pass

  def _get_cgroup_info(self):
    data = []
    CGROUP_SUBSYS = 'cpuset'
    CGROUP_ROOT = '/dev/cpuset/'

    cgroups = self._device_utils.ReadFile('/proc/cgroups')
    header = '# cgroup task attachment\n'

    root_id = parse_proc_cgroups(cgroups, CGROUP_SUBSYS)

    for cgrp in self._device_utils.StatDirectory(CGROUP_ROOT):
      if not stat.S_ISDIR(cgrp['st_mode']):
        continue
      tasks_file = CGROUP_ROOT + cgrp['filename'] + '/tasks'
      tasks = self._device_utils.ReadFile(tasks_file).split('\n')
      cgrp_info = '/%s (root=%s) : ' % (cgrp['filename'], root_id)
      data.append(cgrp_info  + ' '.join(tasks))
    return cgroups + header + '\n'.join(data) + '\n'
