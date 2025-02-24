# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import platform
import unittest
from unittest import mock

from telemetry.internal.platform.tracing_agent import chrome_tracing_agent
from telemetry.internal.platform.tracing_agent import (
    chrome_tracing_devtools_manager)
from telemetry.timeline import tracing_config

_CHROME_TRACING_AGENT_IMPORT_PATH = 'telemetry.internal.platform.tracing_agent'\
                                    '.chrome_tracing_agent.ChromeTracingAgent'


class FakeTracingControllerBackend():
  def __init__(self):
    self.is_tracing_running = False


class FakePlatformBackend():
  def __init__(self):
    self.tracing_controller_backend = FakeTracingControllerBackend()

  def GetOSName(self):
    raise NotImplementedError


class FakeDesktopPlatformBackend(FakePlatformBackend):
  def GetOSName(self):
    system = platform.system()
    if system == 'Linux':
      return 'linux'
    if system == 'Darwin':
      return 'mac'
    if system == 'Windows':
      return 'win'
    return 'unknown'


class FakeContextMap():
  def __init__(self, contexts):
    self.contexts = contexts


class FakeDevtoolsClient():
  def __init__(self, remote_port, platform_backend):
    self.has_tracing_client = True
    self.is_alive = True
    self.is_tracing_running = False
    self.remote_port = remote_port
    self.will_raise_exception_in_stop_tracing = False
    self.will_raise_exception_in_clock_sync = False
    self.collected = False
    self.chrome_branch_number = 9001
    self.platform_backend = platform_backend

  def IsAlive(self):
    return self.is_alive

  def StartChromeTracing(self, trace_options, transfer_mode=None, timeout=20):
    del trace_options, transfer_mode, timeout  # unused
    self.is_tracing_running = True

  def StopChromeTracing(self):
    self.is_tracing_running = False
    if self.will_raise_exception_in_stop_tracing:
      raise Exception

  def CollectChromeTracingData(self, trace_data_builder, timeout=30):
    del trace_data_builder  # unused
    del timeout # unused
    self.collected = True

  def GetUpdatedInspectableContexts(self):
    return FakeContextMap([])

  def RecordChromeClockSyncMarker(self, sync_id):
    del sync_id # unused
    if self.will_raise_exception_in_clock_sync:
      raise Exception

  def GetChromeBranchNumber(self):
    return self.chrome_branch_number

  def FirstTabBackend(self):
    return None


class ChromeTracingAgentTest(unittest.TestCase):
  def setUp(self):
    self.platform1 = FakeDesktopPlatformBackend()
    self.platform2 = FakeDesktopPlatformBackend()
    self.platform3 = FakeDesktopPlatformBackend()

  def StartTracing(self, platform_backend, enable_chrome_trace=True,
                   throw_exception=False):
    config = tracing_config.TracingConfig()
    config.enable_chrome_trace = enable_chrome_trace
    config.chrome_trace_config.category_filter.AddIncludedCategory('foo')
    agent = chrome_tracing_agent.ChromeTracingAgent(platform_backend, config)

    if throw_exception:
      agent._trace_config = True
    agent._platform_backend.tracing_controller_backend.is_tracing_running = True
    agent._test_config = config
    agent._transfer_mode = 'ReturnAsStream'
    with mock.patch(_CHROME_TRACING_AGENT_IMPORT_PATH + '._StartStartupTracing',
                    return_value=True):
      agent.StartAgentTracing(config, 10)
    return agent

  def FlushTracing(self, agent):
    with mock.patch(_CHROME_TRACING_AGENT_IMPORT_PATH + '._StartStartupTracing',
                    return_value=True):
      with mock.patch(
          _CHROME_TRACING_AGENT_IMPORT_PATH + '._RemoveTraceConfigFile'):
        agent.FlushAgentTracing(agent._test_config, 10, None)

  def StopTracing(self, agent):
    agent._platform_backend.tracing_controller_backend.is_tracing_running = (
        False)
    agent.RecordClockSyncMarker('123', lambda *unused: True)
    with mock.patch(
        _CHROME_TRACING_AGENT_IMPORT_PATH + '._RemoveTraceConfigFile'):
      agent.StopAgentTracing()
    agent.CollectAgentTraceData(None)

  def testRegisterDevtoolsClient(self):
    chrome_tracing_devtools_manager.RegisterDevToolsClient(
        FakeDevtoolsClient(1, self.platform1))
    chrome_tracing_devtools_manager.RegisterDevToolsClient(
        FakeDevtoolsClient(2, self.platform1))
    chrome_tracing_devtools_manager.RegisterDevToolsClient(
        FakeDevtoolsClient(3, self.platform1))

    tracing_agent_of_platform1 = self.StartTracing(self.platform1)

    chrome_tracing_devtools_manager.RegisterDevToolsClient(
        FakeDevtoolsClient(4, self.platform1))
    chrome_tracing_devtools_manager.RegisterDevToolsClient(
        FakeDevtoolsClient(5, self.platform2))

    self.StopTracing(tracing_agent_of_platform1)
    chrome_tracing_devtools_manager.RegisterDevToolsClient(
        FakeDevtoolsClient(6, self.platform1))

  def testStartAndStopTracing(self):
    devtool1 = FakeDevtoolsClient(1, self.platform1)
    devtool2 = FakeDevtoolsClient(2, self.platform1)
    devtool3 = FakeDevtoolsClient(3, self.platform1)
    devtool4 = FakeDevtoolsClient(2, self.platform2)
    # Register devtools 1, 2, 3 on platform1 and devtool 4 on platform 2
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool1)
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool2)
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool3)
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool4)
    devtool2.is_alive = False

    tracing_agent1 = self.StartTracing(self.platform1)
    with self.assertRaises(chrome_tracing_agent.ChromeTracingStartedError):
      self.StartTracing(self.platform1)

    self.assertTrue(devtool1.is_tracing_running)
    self.assertFalse(devtool2.is_tracing_running)
    self.assertTrue(devtool3.is_tracing_running)
    # Devtool 4 shouldn't have tracing started although it has the same remote
    # port as devtool 2
    self.assertFalse(devtool4.is_tracing_running)


    self.assertFalse(devtool1.collected)
    self.StopTracing(tracing_agent1)
    self.assertTrue(devtool1.collected)
    self.assertFalse(devtool1.is_tracing_running)
    self.assertFalse(devtool2.is_tracing_running)
    self.assertFalse(devtool3.is_tracing_running)
    self.assertFalse(devtool4.is_tracing_running)

    # Test that it should be ok to start & stop tracing on platform1 again.
    tracing_agent1 = self.StartTracing(self.platform1)
    self.StopTracing(tracing_agent1)

    tracing_agent2 = self.StartTracing(self.platform2)
    self.assertTrue(devtool4.is_tracing_running)
    self.assertFalse(devtool4.collected)
    self.StopTracing(tracing_agent2)
    self.assertFalse(devtool4.is_tracing_running)
    self.assertTrue(devtool4.collected)

  def testFlushTracing(self):
    devtool1 = FakeDevtoolsClient(1, self.platform1)
    devtool2 = FakeDevtoolsClient(2, self.platform1)
    devtool3 = FakeDevtoolsClient(3, self.platform1)
    devtool4 = FakeDevtoolsClient(2, self.platform2)
    # Register devtools 1, 2, 3 on platform1 and devtool 4 on platform 2
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool1)
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool2)
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool3)
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool4)
    devtool2.is_alive = False

    tracing_agent1 = self.StartTracing(self.platform1)

    self.assertTrue(devtool1.is_tracing_running)
    self.assertFalse(devtool2.is_tracing_running)
    self.assertTrue(devtool3.is_tracing_running)
    # Devtool 4 shouldn't have tracing started although it has the same remote
    # port as devtool 2.
    self.assertFalse(devtool4.is_tracing_running)

    for _ in range(5):
      self.FlushTracing(tracing_agent1)
      self.assertTrue(devtool1.is_tracing_running)
      self.assertFalse(devtool2.is_tracing_running)
      self.assertTrue(devtool3.is_tracing_running)
      self.assertFalse(devtool4.is_tracing_running)

    self.StopTracing(tracing_agent1)
    self.assertFalse(devtool1.is_tracing_running)
    self.assertFalse(devtool2.is_tracing_running)
    self.assertFalse(devtool3.is_tracing_running)
    self.assertFalse(devtool4.is_tracing_running)

    # Test that it is ok to start, flush & stop tracing on platform1 again.
    tracing_agent1 = self.StartTracing(self.platform1)
    self.FlushTracing(tracing_agent1)
    self.StopTracing(tracing_agent1)

    tracing_agent2 = self.StartTracing(self.platform2)
    self.assertTrue(devtool4.is_tracing_running)
    self.FlushTracing(tracing_agent2)
    self.assertTrue(devtool4.is_tracing_running)
    self.StopTracing(tracing_agent2)
    self.assertFalse(devtool4.is_tracing_running)

  def testExceptionRaisedInStopTracing(self):
    devtool1 = FakeDevtoolsClient(1, self.platform1)
    devtool2 = FakeDevtoolsClient(2, self.platform1)
    # Register devtools 1, 2 on platform 1
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool1)
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool2)
    tracing_agent1 = self.StartTracing(self.platform1)

    self.assertTrue(devtool1.is_tracing_running)
    self.assertTrue(devtool2.is_tracing_running)

    devtool1.will_raise_exception_in_stop_tracing = True
    with self.assertRaises(chrome_tracing_agent.ChromeTracingStoppedError):
      self.StopTracing(tracing_agent1)
    # Tracing is stopped on both devtools clients even if there is exception.
    self.assertIsNone(tracing_agent1.trace_config)
    self.assertFalse(devtool1.is_tracing_running)
    self.assertFalse(devtool2.is_tracing_running)

    devtool1.is_alive = False
    devtool2.is_alive = False
    # Register devtools 3 on platform 1 should not raise any exception.
    devtool3 = FakeDevtoolsClient(3, self.platform1)
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool3)

    # Start & Stop tracing on platform 1 should work just fine.
    tracing_agent2 = self.StartTracing(self.platform1)
    self.StopTracing(tracing_agent2)

  def testExceptionRaisedInStartTracing(self):
    devtool1 = FakeDevtoolsClient(1, self.platform1)
    devtool2 = FakeDevtoolsClient(2, self.platform1)
    # Register devtools 1, 2 on platform 1
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool1)
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool2)

    with self.assertRaises(chrome_tracing_agent.ChromeTracingStartedError):
      self.StartTracing(self.platform1, throw_exception=True)

    # Tracing is stopped on both devtools clients even if there is exception.
    self.assertFalse(devtool1.is_tracing_running)
    self.assertFalse(devtool2.is_tracing_running)

    devtool1.is_alive = False
    devtool2.is_alive = False
    # Register devtools 3 on platform 1 should not raise any exception.
    devtool3 = FakeDevtoolsClient(3, self.platform1)
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool3)

    # Start & Stop tracing on platform 1 should work just fine.
    tracing_agent2 = self.StartTracing(self.platform1)
    self.assertTrue(devtool3.is_tracing_running)
    self.assertIsNotNone(tracing_agent2.trace_config)

    self.StopTracing(tracing_agent2)
    self.assertIsNone(tracing_agent2.trace_config)
    self.assertFalse(devtool3.is_tracing_running)

  def testExceptionRaisedInClockSync(self):
    devtool1 = FakeDevtoolsClient(1, self.platform1)
    # Register devtools 1 on platform 1
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool1)
    tracing_agent1 = self.StartTracing(self.platform1)

    self.assertTrue(devtool1.is_tracing_running)

    devtool1.will_raise_exception_in_clock_sync = True
    with self.assertRaises(chrome_tracing_agent.ChromeClockSyncError):
      self.StopTracing(tracing_agent1)

    devtool1.is_alive = False
    # Register devtools 2 on platform 1 should not raise any exception.
    devtool2 = FakeDevtoolsClient(2, self.platform1)
    chrome_tracing_devtools_manager.RegisterDevToolsClient(devtool2)

    # Start & Stop tracing on platform 1 should work just fine.
    tracing_agent2 = self.StartTracing(self.platform1)
    self.StopTracing(tracing_agent2)
    self.assertIsNone(tracing_agent2.trace_config)
    self.assertFalse(devtool2.is_tracing_running)
