# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import timeit
import unittest

from telemetry.internal.backends.chrome_inspector import tracing_backend
from telemetry.testing import fakes
from telemetry.timeline import tracing_config
from tracing.trace_data import trace_data


class TracingBackendUnittest(unittest.TestCase):
  def setUp(self):
    self._fake_timer = fakes.FakeTimer(tracing_backend)
    self._inspector_socket = fakes.FakeInspectorWebsocket(self._fake_timer)

  def tearDown(self):
    self._fake_timer.Restore()

  def testCollectTracingDataTimeout(self):
    self._inspector_socket.AddEvent(
        'Tracing.dataCollected', {'value': {'traceEvents': [{'ph': 'B'}]}}, 9)
    self._inspector_socket.AddEvent(
        'Tracing.dataCollected', {'value': {'traceEvents': [{'ph': 'E'}]}}, 19)
    self._inspector_socket.AddEvent('Tracing.tracingComplete', {}, 35)
    backend = tracing_backend.TracingBackend(self._inspector_socket)

    with trace_data.TraceDataBuilder() as builder:
      # The third response is 16 seconds after the second response, so we expect
      # a TracingTimeoutException.
      with self.assertRaises(tracing_backend.TracingTimeoutException):
        backend._CollectTracingData(builder, 10)
      traces = builder.AsData().GetTracesFor(trace_data.CHROME_TRACE_PART)

    self.assertEqual(2, len(traces))
    self.assertEqual(1, len(traces[0].get('traceEvents', [])))
    self.assertEqual(1, len(traces[1].get('traceEvents', [])))
    self.assertFalse(backend._has_received_all_tracing_data)

  def testCollectTracingDataNoTimeout(self):
    self._inspector_socket.AddEvent(
        'Tracing.dataCollected', {'value': {'traceEvents': [{'ph': 'B'}]}}, 9)
    self._inspector_socket.AddEvent(
        'Tracing.dataCollected', {'value': {'traceEvents': [{'ph': 'E'}]}}, 14)
    self._inspector_socket.AddEvent('Tracing.tracingComplete', {}, 19)
    backend = tracing_backend.TracingBackend(self._inspector_socket)

    with trace_data.TraceDataBuilder() as builder:
      backend._CollectTracingData(builder, 10)
      traces = builder.AsData().GetTracesFor(trace_data.CHROME_TRACE_PART)

    self.assertEqual(2, len(traces))
    self.assertEqual(1, len(traces[0].get('traceEvents', [])))
    self.assertEqual(1, len(traces[1].get('traceEvents', [])))
    self.assertTrue(backend._has_received_all_tracing_data)

  def testCollectTracingDataFromStreamNoContainer(self):
    self._inspector_socket.AddEvent(
        'Tracing.tracingComplete', {'stream': '42'}, 1)
    self._inspector_socket.AddAsyncResponse(
        'IO.read', {'data': '{"traceEvents": [{},{},{'}, 2)
    self._inspector_socket.AddAsyncResponse(
        'IO.read', {'data': '},{},{}]}', 'eof': True}, 3)
    backend = tracing_backend.TracingBackend(self._inspector_socket)

    with trace_data.TraceDataBuilder() as builder:
      backend._CollectTracingData(builder, 10)
      trace = builder.AsData().GetTraceFor(trace_data.CHROME_TRACE_PART)

    self.assertEqual(5, len(trace['traceEvents']))
    self.assertTrue(backend._has_received_all_tracing_data)

  def testCollectTracingDataFromStreamJSONContainer(self):
    self._inspector_socket.AddEvent(
        'Tracing.tracingComplete', {'stream': '42'}, 1)
    self._inspector_socket.AddAsyncResponse(
        'IO.read', {'data': '{"traceEvents": [{},{},{}],'}, 2)
    self._inspector_socket.AddAsyncResponse(
        'IO.read', {'data': '"metadata": {"a": "b"}'}, 3)
    self._inspector_socket.AddAsyncResponse(
        'IO.read', {'data': '}', 'eof': True}, 4)
    backend = tracing_backend.TracingBackend(self._inspector_socket)

    with trace_data.TraceDataBuilder() as builder:
      backend._CollectTracingData(builder, 10)
      trace = builder.AsData().GetTraceFor(trace_data.CHROME_TRACE_PART)

    self.assertEqual(3, len(trace['traceEvents']))
    self.assertEqual(dict, type(trace.get('metadata')))
    self.assertTrue(backend._has_received_all_tracing_data)

  def testDumpMemorySuccess(self):
    self._inspector_socket.AddResponseHandler(
        'Tracing.requestMemoryDump',
        lambda req: {'result': {'success': True, 'dumpGuid': '42abc'}})
    backend = tracing_backend.TracingBackend(self._inspector_socket)

    self.assertEqual(backend.DumpMemory(), '42abc')

  def testDumpMemoryFailure(self):
    self._inspector_socket.AddResponseHandler(
        'Tracing.requestMemoryDump',
        lambda req: {'result': {'success': False, 'dumpGuid': '42abc'}})
    backend = tracing_backend.TracingBackend(self._inspector_socket)

    with self.assertRaises(tracing_backend.TracingUnexpectedResponseException):
      backend.DumpMemory()

  def testStartTracingFailure(self):
    self._inspector_socket.AddResponseHandler(
        'Tracing.start',
        lambda req: {'error': {'message': 'Tracing is already started'}})
    self._inspector_socket.AddResponseHandler(
        'Tracing.hasCompleted', lambda req: {})
    backend = tracing_backend.TracingBackend(self._inspector_socket)
    config = tracing_config.TracingConfig()
    self.assertRaisesRegex(
        tracing_backend.TracingUnexpectedResponseException,
        'Tracing is already started',
        backend.StartTracing, config.chrome_trace_config)

  def testStopTracingFailure(self):
    self._inspector_socket.AddResponseHandler('Tracing.start', lambda req: {})
    self._inspector_socket.AddResponseHandler(
        'Tracing.end',
        lambda req: {'error': {'message': 'Tracing had an error'}})
    self._inspector_socket.AddResponseHandler(
        'Tracing.hasCompleted', lambda req: {})
    backend = tracing_backend.TracingBackend(self._inspector_socket)
    config = tracing_config.TracingConfig()
    backend.StartTracing(config._chrome_trace_config)
    self.assertRaisesRegex(
        tracing_backend.TracingUnexpectedResponseException,
        'Tracing had an error',
        backend.StopTracing)

  def testStartTracingWithoutCollection(self):
    self._inspector_socket.AddResponseHandler('Tracing.start', lambda req: {})
    self._inspector_socket.AddResponseHandler('Tracing.end', lambda req: {})
    self._inspector_socket.AddEvent(
        'Tracing.dataCollected', {'value': [{'ph': 'B'}]}, 1)
    self._inspector_socket.AddEvent(
        'Tracing.dataCollected', {'value': [{'ph': 'E'}]}, 2)
    self._inspector_socket.AddEvent('Tracing.tracingComplete', {}, 3)
    self._inspector_socket.AddResponseHandler(
        'Tracing.hasCompleted', lambda req: {})

    backend = tracing_backend.TracingBackend(self._inspector_socket)
    config = tracing_config.TracingConfig()
    backend.StartTracing(config._chrome_trace_config)
    backend.StopTracing()
    with self.assertRaisesRegex(AssertionError, 'Data not collected from .*'):
      backend.StartTracing(config._chrome_trace_config)


class DevToolsStreamPerformanceTest(unittest.TestCase):
  def setUp(self):
    self._fake_timer = fakes.FakeTimer(tracing_backend)
    self._inspector_socket = fakes.FakeInspectorWebsocket(self._fake_timer)

  def _MeasureReadTime(self, count):
    fake_time = self._fake_timer.time() + 1
    payload = ','.join(['{}'] * 5000)
    self._inspector_socket.AddAsyncResponse('IO.read', {'data': '[' + payload},
                                            fake_time)
    start_clock = timeit.default_timer()

    done = {'done': False}
    def MarkDone():
      done['done'] = True

    with open(os.devnull, 'wb') as trace_handle:
      reader = tracing_backend._DevToolsStreamReader(
          self._inspector_socket, 'dummy', trace_handle)
      reader.Read(MarkDone)
      while not done['done']:
        fake_time += 1
        if count > 0:
          self._inspector_socket.AddAsyncResponse(
              'IO.read', {'data': payload},
              fake_time)
        elif count == 0:
          self._inspector_socket.AddAsyncResponse(
              'IO.read',
              {'data': payload + ']', 'eof': True}, fake_time)
        count -= 1
        self._inspector_socket.DispatchNotifications(10)
    return timeit.default_timer() - start_clock

  def testReadTime(self):
    n1 = 1000
    while True:
      t1 = self._MeasureReadTime(n1)
      if t1 > 0.01:
        break
      n1 *= 5
    t2 = self._MeasureReadTime(n1 * 10)
    # Time is an illusion, CPU time is doubly so, allow great deal of tolerance.
    tolerance_factor = 5
    self.assertLess(t2, t1 * 10 * tolerance_factor)
