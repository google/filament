# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for the tracing_controller_backend.

These are written to test the public API of the TracingControllerBackend,
using a mock platform and mock tracing agents.

Integrations tests using a real running browser and tracing agents are included
among tests for the public facing telemetry.core.tracing_controller.
"""

from __future__ import absolute_import
import unittest
from unittest import mock

from telemetry import decorators
from telemetry.internal.platform import platform_backend
from telemetry.internal.platform import tracing_agent
from telemetry.internal.platform.tracing_agent import telemetry_tracing_agent
from telemetry.internal.platform import tracing_controller_backend
from telemetry.timeline import tracing_config


class FakeTraceDataBuilder():
  """Discards trace data but used to keep track of clock syncs."""
  def __init__(self):
    self.clock_syncs = []

  def AddTraceFor(self, trace_part, value):
    del trace_part  # Unused.
    del value  # Unused.

  def Freeze(self):
    return self


class TracingControllerBackendTest(unittest.TestCase):
  def setUp(self):
    # Create a real TracingControllerBackend with a mock platform backend.
    mock_platform = mock.Mock(spec=platform_backend.PlatformBackend)
    self.controller = (
        tracing_controller_backend.TracingControllerBackend(mock_platform))
    self.config = tracing_config.TracingConfig()

    # Replace the telemetry_tracing_agent module with a mock.
    self._clock_syncs = []
    def record_issuer_clock_sync(sync_id, issue_ts):
      del issue_ts  # Unused.
      self._clock_syncs.append(sync_id)

    self.telemetry_tracing_agent = mock.patch(
        'telemetry.internal.platform.tracing_controller_backend'
        '.telemetry_tracing_agent').start()
    self.telemetry_tracing_agent.RecordIssuerClockSyncMarker.side_effect = (
        record_issuer_clock_sync)

    # Replace the list of real tracing agent classes with one containing:
    # - a mock TelemetryTracingAgent to work as clock sync recorder, and
    # - a simple mock TracingAgent.
    # Tests can also override this list using _SetTracingAgentClasses.
    self._TRACING_AGENT_CLASSES = [
        self.MockAgentClass(clock_sync_recorder=True),
        self.MockAgentClass()]
    mock.patch(
        'telemetry.internal.platform.tracing_controller_backend'
        '._TRACING_AGENT_CLASSES', new=self._TRACING_AGENT_CLASSES).start()

    # Replace the real TraceDataBuilder with a fake one to collect clock_syncs.
    mock.patch('tracing.trace_data.trace_data.TraceDataBuilder',
               new=FakeTraceDataBuilder).start()

  def tearDown(self):
    if self.controller.is_tracing_running:
      self.controller.StopTracing()
    mock.patch.stopall()

  def MockAgentClass(self, can_start=True, supports_clock_sync=True,
                     clock_sync_recorder=False):
    """Factory to create mock tracing agent classes."""
    if clock_sync_recorder:
      supports_clock_sync = False  # Can't be both issuer and recorder.
      spec = telemetry_tracing_agent.TelemetryTracingAgent
    else:
      spec = tracing_agent.TracingAgent

    agent = mock.Mock(spec=spec)
    agent.StartAgentTracing.return_value = can_start
    agent.SupportsExplicitClockSync.return_value = supports_clock_sync

    if clock_sync_recorder:
      def collect_clock_syncs(trace_data, timeout=None):
        del timeout  # Unused.
        # Copy the clock_syncs to the trace data, then clear our own list.
        trace_data.clock_syncs = list(self._clock_syncs)
        self._clock_syncs[:] = []

      agent.CollectAgentTraceData.side_effect = collect_clock_syncs
    elif supports_clock_sync:
      def issue_clock_sync(sync_id, callback):
        callback(sync_id, 1)

      agent.RecordClockSyncMarker.side_effect = issue_clock_sync

    AgentClass = mock.Mock(return_value=agent)
    AgentClass.IsSupported.return_value = True
    return AgentClass

  def _SetTracingAgentClasses(self, *agent_classes):
    # Replace contents of the list with the agent classes given as args.
    self._TRACING_AGENT_CLASSES[:] = agent_classes

  @decorators.Isolated
  def testStartTracing(self):
    self.assertFalse(self.controller.is_tracing_running)
    self.assertTrue(self.controller.StartTracing(self.config, 30))
    self.assertTrue(self.controller.is_tracing_running)

  @decorators.Isolated
  def testDoubleStartTracing(self):
    self.assertFalse(self.controller.is_tracing_running)
    self.assertTrue(self.controller.StartTracing(self.config, 30))
    self.assertTrue(self.controller.is_tracing_running)
    self.assertFalse(self.controller.StartTracing(self.config, 30))

  @decorators.Isolated
  def testStopTracingNotStarted(self):
    with self.assertRaises(AssertionError):
      self.controller.StopTracing()

  @decorators.Isolated
  def testStopTracing(self):
    self.assertFalse(self.controller.is_tracing_running)
    self.assertTrue(self.controller.StartTracing(self.config, 30))
    self.assertTrue(self.controller.is_tracing_running)
    data = self.controller.StopTracing()
    self.assertEqual(len(data.clock_syncs), 1)
    self.assertFalse(self.controller.is_tracing_running)

  @decorators.Isolated
  def testDoubleStopTracing(self):
    self.assertFalse(self.controller.is_tracing_running)
    self.assertTrue(self.controller.StartTracing(self.config, 30))
    self.assertTrue(self.controller.is_tracing_running)
    data = self.controller.StopTracing()
    self.assertEqual(len(data.clock_syncs), 1)
    self.assertFalse(self.controller.is_tracing_running)
    with self.assertRaises(AssertionError):
      self.controller.StopTracing()

  @decorators.Isolated
  def testMultipleStartStop(self):
    self.assertFalse(self.controller.is_tracing_running)
    self.assertTrue(self.controller.StartTracing(self.config, 30))
    self.assertTrue(self.controller.is_tracing_running)
    data = self.controller.StopTracing()
    self.assertEqual(len(data.clock_syncs), 1)
    sync_event_one = data.clock_syncs[0]
    self.assertFalse(self.controller.is_tracing_running)
    # Run 2
    self.assertTrue(self.controller.StartTracing(self.config, 30))
    self.assertTrue(self.controller.is_tracing_running)
    data = self.controller.StopTracing()
    self.assertEqual(len(data.clock_syncs), 1)
    sync_event_two = data.clock_syncs[0]
    self.assertFalse(self.controller.is_tracing_running)
    # Test difference between events
    self.assertNotEqual(sync_event_one, sync_event_two)

  @decorators.Isolated
  def testFlush(self):
    self.assertFalse(self.controller.is_tracing_running)
    self.assertIsNone(self.controller._current_state)

    # Start tracing.
    self.assertTrue(self.controller.StartTracing(self.config, 30))
    self.assertTrue(self.controller.is_tracing_running)
    self.assertIs(self.controller._current_state.config, self.config)
    self.assertEqual(self.controller._current_state.timeout, 30)
    self.assertIsNotNone(self.controller._current_state.builder)

    # Flush tracing several times.
    for _ in range(5):
      self.controller.FlushTracing()
      self.assertTrue(self.controller.is_tracing_running)
      self.assertIs(self.controller._current_state.config, self.config)
      self.assertEqual(self.controller._current_state.timeout, 30)
      self.assertIsNotNone(self.controller._current_state.builder)

    # Stop tracing.
    data = self.controller.StopTracing()
    self.assertFalse(self.controller.is_tracing_running)
    self.assertIsNone(self.controller._current_state)

    self.assertEqual(len(data.clock_syncs), 6)

  @decorators.Isolated
  def testNoWorkingAgents(self):
    self._SetTracingAgentClasses(self.MockAgentClass(can_start=False))
    self.assertFalse(self.controller.is_tracing_running)
    self.assertTrue(self.controller.StartTracing(self.config, 30))
    self.assertTrue(self.controller.is_tracing_running)
    self.assertEqual(self.controller._active_agents_instances, [])
    data = self.controller.StopTracing()
    self.assertEqual(len(data.clock_syncs), 0)
    self.assertFalse(self.controller.is_tracing_running)

  @decorators.Isolated
  def testNoClockSyncSupport(self):
    self._SetTracingAgentClasses(
        self.MockAgentClass(clock_sync_recorder=True),
        self.MockAgentClass(supports_clock_sync=False))
    self.assertFalse(self.controller.is_tracing_running)
    self.assertTrue(self.controller.StartTracing(self.config, 30))
    self.assertTrue(self.controller.is_tracing_running)
    self.assertEqual(len(self.controller._active_agents_instances), 2)
    data = self.controller.StopTracing()
    self.assertFalse(self.controller.is_tracing_running)
    self.assertEqual(len(data.clock_syncs), 0)

  @decorators.Isolated
  def testMultipleAgents(self):
    # Only 5 agents can start and, from those, only 2 support clock sync.
    self._SetTracingAgentClasses(
        self.MockAgentClass(clock_sync_recorder=True),
        self.MockAgentClass(),
        self.MockAgentClass(),
        self.MockAgentClass(can_start=False),
        self.MockAgentClass(can_start=False),
        self.MockAgentClass(supports_clock_sync=False),
        self.MockAgentClass(supports_clock_sync=False)
    )
    self.assertFalse(self.controller.is_tracing_running)
    self.assertTrue(self.controller.StartTracing(self.config, 30))
    self.assertTrue(self.controller.is_tracing_running)
    self.assertEqual(len(self.controller._active_agents_instances), 5)
    data = self.controller.StopTracing()
    self.assertFalse(self.controller.is_tracing_running)
    self.assertEqual(len(data.clock_syncs), 2)

  @decorators.Isolated
  def testIssueClockSyncMarker_noClockSyncRecorder(self):
    # Only 4 agents can start, but the clock sync recorder cant.
    self._SetTracingAgentClasses(
        self.MockAgentClass(clock_sync_recorder=True, can_start=False),
        self.MockAgentClass(),
        self.MockAgentClass(),
        self.MockAgentClass(can_start=False),
        self.MockAgentClass(can_start=False),
        self.MockAgentClass(supports_clock_sync=False),
        self.MockAgentClass(supports_clock_sync=False)
    )
    self.assertFalse(self.controller.is_tracing_running)
    self.assertTrue(self.controller.StartTracing(self.config, 30))
    self.assertTrue(self.controller.is_tracing_running)
    self.assertEqual(len(self.controller._active_agents_instances), 4)
    data = self.controller.StopTracing()
    self.assertFalse(self.controller.is_tracing_running)
    self.assertEqual(len(data.clock_syncs), 0)  # No clock syncs found.
