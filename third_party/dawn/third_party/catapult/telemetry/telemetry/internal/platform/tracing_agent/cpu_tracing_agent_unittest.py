# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import sys
import time
import unittest

from telemetry import decorators
from telemetry.internal.platform.tracing_agent import cpu_tracing_agent
from telemetry.internal.platform import tracing_agent
from telemetry.internal.platform import linux_platform_backend
from telemetry.internal.platform import mac_platform_backend
from telemetry.internal.platform import win_platform_backend
from telemetry.timeline import tracing_config
from tracing.trace_data import trace_data


SNAPSHOT_KEYS = ['pid', 'ppid', 'name', 'pCpu', 'pMem']
TRACE_EVENT_KEYS = ['name', 'tid', 'pid', 'ph', 'args', 'local', 'id', 'ts']


class FakeAndroidPlatformBackend():
  def __init__(self):
    self.device = 'fake_device'

  def GetOSName(self):
    return 'android'


class CpuTracingAgentTest(unittest.TestCase):

  def setUp(self):
    self._config = tracing_config.TracingConfig()
    self._config.enable_cpu_trace = True
    if sys.platform.startswith('win'):
      self._desktop_backend = win_platform_backend.WinPlatformBackend()
    elif sys.platform.startswith('darwin'):
      self._desktop_backend = mac_platform_backend.MacPlatformBackend()
    else:
      self._desktop_backend = linux_platform_backend.LinuxPlatformBackend()
    self._agent = cpu_tracing_agent.CpuTracingAgent(self._desktop_backend,
                                                    self._config)

  @decorators.Enabled('linux', 'mac', 'win')
  def testInit(self):
    self.assertTrue(isinstance(self._agent,
                               tracing_agent.TracingAgent))
    self.assertFalse(self._agent._snapshots)
    self.assertFalse(self._agent._snapshot_ongoing)

  @decorators.Enabled('linux', 'mac', 'win')
  def testIsSupported(self):
    self.assertTrue(cpu_tracing_agent.CpuTracingAgent.IsSupported(
        self._desktop_backend))
    self.assertFalse(cpu_tracing_agent.CpuTracingAgent.IsSupported(
        FakeAndroidPlatformBackend()))

  # Flaky on Win (crbug.com/803210).
  @decorators.Enabled('linux', 'mac')
  def testStartAgentTracing(self):
    self.assertFalse(self._agent._snapshot_ongoing)
    self.assertFalse(self._agent._snapshots)
    self.assertTrue(self._agent.StartAgentTracing(self._config, 0))
    self.assertTrue(self._agent._snapshot_ongoing)
    time.sleep(2)
    self.assertTrue(self._agent._snapshots)
    self._agent.StopAgentTracing()

  # Flaky on Win (crbug.com/803210).
  @decorators.Enabled('linux', 'mac')
  def testStartAgentTracingNotEnabled(self):
    self._config.enable_cpu_trace = False
    self.assertFalse(self._agent._snapshot_ongoing)
    self.assertFalse(self._agent.StartAgentTracing(self._config, 0))
    self.assertFalse(self._agent._snapshot_ongoing)
    self.assertFalse(self._agent._snapshots)
    time.sleep(2)
    self.assertFalse(self._agent._snapshots)

  # Flaky on Win (crbug.com/803210).
  @decorators.Enabled('linux', 'mac')
  def testStopAgentTracingBeforeStart(self):
    self.assertRaises(AssertionError, self._agent.StopAgentTracing)

  # Flaky on Win (crbug.com/803210).
  @decorators.Enabled('linux', 'mac')
  def testStopAgentTracing(self):
    self._agent.StartAgentTracing(self._config, 0)
    self._agent.StopAgentTracing()
    self.assertFalse(self._agent._snapshot_ongoing)

  # Flaky on Win (crbug.com/803210).
  @decorators.Enabled('linux', 'mac')
  def testCollectAgentTraceDataBeforeStop(self):
    self._agent.StartAgentTracing(self._config, 0)
    with self.assertRaises(AssertionError):
      with trace_data.TraceDataBuilder() as builder:
        self._agent.CollectAgentTraceData(builder)
    self._agent.StopAgentTracing()

  # Flaky on Win (crbug.com/803210).
  @decorators.Enabled('linux', 'mac')
  def testCollectAgentTraceData(self):
    self._agent.StartAgentTracing(self._config, 0)
    self._agent.StopAgentTracing()
    with trace_data.TraceDataBuilder() as builder:
      self._agent.CollectAgentTraceData(builder)
      self.assertFalse(self._agent._snapshot_ongoing)
      data = builder.AsData()
    self.assertTrue(data.HasTracesFor(trace_data.CPU_TRACE_DATA))

  # Flaky on Win (crbug.com/803210).
  @decorators.Enabled('linux', 'mac')
  def testCollectAgentTraceDataFormat(self):
    self._agent.StartAgentTracing(self._config, 0)
    time.sleep(2)
    self._agent.StopAgentTracing()
    with trace_data.TraceDataBuilder() as builder:
      self._agent.CollectAgentTraceData(builder)
      data = builder.AsData().GetTraceFor(
          trace_data.CPU_TRACE_DATA)['traceEvents']

    self.assertEqual(set(data[0].keys()), set(TRACE_EVENT_KEYS))
    self.assertEqual(set(data[0]['args']['snapshot'].keys()), {'processes'})
    self.assertTrue(data[0]['args']['snapshot']['processes'])
    self.assertEqual(set(data[0]['args']['snapshot']['processes'][0].keys()),
                     set(SNAPSHOT_KEYS))

  # Flaky on Win (crbug.com/803210).
  @decorators.Enabled('linux', 'mac')
  def testContainsRealProcesses(self):
    self._agent.StartAgentTracing(self._config, 0)
    time.sleep(2)
    self._agent.StopAgentTracing()
    with trace_data.TraceDataBuilder() as builder:
      self._agent.CollectAgentTraceData(builder)
      data = builder.AsData().GetTraceFor(
          trace_data.CPU_TRACE_DATA)['traceEvents']

    for snapshot in data:
      found_unittest_process = False
      processes = snapshot['args']['snapshot']['processes']
      for process in processes:
        if 'run_tests' in process['name']:
          found_unittest_process = True

      self.assertTrue(found_unittest_process)

  # Flaky on Win (crbug.com/803210).
  @decorators.Enabled('linux', 'mac')
  def testTraceSpecifiesTelemetryClockDomain(self):
    self._agent.StartAgentTracing(self._config, 0)
    self._agent.StopAgentTracing()
    with trace_data.TraceDataBuilder() as builder:
      self._agent.CollectAgentTraceData(builder)
      cpu_trace = builder.AsData().GetTraceFor(trace_data.CPU_TRACE_DATA)

    self.assertEqual(cpu_trace['metadata']['clock-domain'], 'TELEMETRY')

  # Flaky on Win (crbug.com/803210).
  @decorators.Disabled('all')
  def testWindowsCanHandleProcessesWithSpaces(self):
    proc_collector = cpu_tracing_agent.WindowsProcessCollector()
    proc_collector.Init()
    proc = proc_collector._ParseProcessString(
        '0 1 Multi Word Process 50 75')
    self.assertEqual(proc['ppid'], 0)
    self.assertEqual(proc['pid'], 1)
    self.assertEqual(proc['name'], 'Multi Word Process')
    self.assertEqual(proc['pCpu'], 50)
