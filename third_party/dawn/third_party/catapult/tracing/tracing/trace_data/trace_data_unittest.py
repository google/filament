# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import base64
import json
import os
import tempfile
import unittest

from py_utils import tempfile_ext
from tracing.trace_data import trace_data

class TraceDataTest(unittest.TestCase):
  def testHasTracesForChrome(self):
    d = trace_data.CreateFromRawChromeEvents([{'ph': 'B'}])
    self.assertTrue(d.HasTracesFor(trace_data.CHROME_TRACE_PART))

  def testHasNotTracesForCpu(self):
    d = trace_data.CreateFromRawChromeEvents([{'ph': 'B'}])
    self.assertFalse(d.HasTracesFor(trace_data.CPU_TRACE_DATA))

  def testGetTracesForChrome(self):
    d = trace_data.CreateFromRawChromeEvents([{'ph': 'B'}])
    ts = d.GetTracesFor(trace_data.CHROME_TRACE_PART)
    self.assertEqual(len(ts), 1)
    self.assertEqual(ts[0], {'traceEvents': [{'ph': 'B'}]})

  def testGetNoTracesForCpu(self):
    d = trace_data.CreateFromRawChromeEvents([{'ph': 'B'}])
    ts = d.GetTracesFor(trace_data.CPU_TRACE_DATA)
    self.assertEqual(ts, [])


class TraceDataBuilderTest(unittest.TestCase):
  def testAddTraceDataAndSerialize(self):
    with tempfile_ext.TemporaryFileName() as trace_path:
      with trace_data.TraceDataBuilder() as builder:
        builder.AddTraceFor(trace_data.CHROME_TRACE_PART,
                            {'traceEvents': [1, 2, 3]})
        builder.Serialize(trace_path)
        self.assertTrue(os.path.exists(trace_path))
        self.assertGreater(os.stat(trace_path).st_size, 0)  # File not empty.

  def testAddTraceForRaisesWithInvalidPart(self):
    with trace_data.TraceDataBuilder() as builder:
      with self.assertRaises(TypeError):
        builder.AddTraceFor('not_a_trace_part', {})

  def testAddTraceWithUnstructuredData(self):
    with trace_data.TraceDataBuilder() as builder:
      builder.AddTraceFor(trace_data.TELEMETRY_PART, 'unstructured trace',
                          allow_unstructured=True)

  def testAddTraceRaisesWithImplicitUnstructuredData(self):
    with trace_data.TraceDataBuilder() as builder:
      with self.assertRaises(ValueError):
        builder.AddTraceFor(trace_data.TELEMETRY_PART, 'unstructured trace')

  def testAddTraceFileFor(self):
    original_data = {'msg': 'The answer is 42'}
    with tempfile.NamedTemporaryFile(
        delete=False, suffix='.json', mode='w+') as source:
      json.dump(original_data, source)
    with trace_data.TraceDataBuilder() as builder:
      builder.AddTraceFileFor(trace_data.CHROME_TRACE_PART, source.name)
      self.assertFalse(os.path.exists(source.name))
      out_data = builder.AsData().GetTraceFor(trace_data.CHROME_TRACE_PART)

    self.assertEqual(original_data, out_data)

  def testOpenTraceHandleFor(self):
    original_data = {'msg': 'The answer is 42'}
    with trace_data.TraceDataBuilder() as builder:
      with builder.OpenTraceHandleFor(
          trace_data.CHROME_TRACE_PART, suffix='.json', mode='w+') as handle:
        handle.write(json.dumps(original_data))
      out_data = builder.AsData().GetTraceFor(trace_data.CHROME_TRACE_PART)

    # Trace handle should be cleaned up.
    self.assertFalse(os.path.exists(handle.name))
    self.assertEqual(original_data, out_data)

  def testOpenTraceHandleForCompressedData(self):
    original_data = {'msg': 'The answer is 42'}
    # gzip.compress() does not work in python 2, so hardcode the encoded data.
    compressed_data = base64.b64decode(
        'H4sIAIDMblwAA6tWyi1OV7JSUArJSFVIzCsuTy1SyCxWMDFSquUCAA4QMtscAAAA')
    with trace_data.TraceDataBuilder() as builder:
      with builder.OpenTraceHandleFor(
          trace_data.CHROME_TRACE_PART, suffix='.json.gz') as handle:
        handle.write(compressed_data)
      out_data = builder.AsData().GetTraceFor(trace_data.CHROME_TRACE_PART)

    # Trace handle should be cleaned up.
    self.assertFalse(os.path.exists(handle.name))
    self.assertEqual(original_data, out_data)

  def testCantWriteAfterCleanup(self):
    with trace_data.TraceDataBuilder() as builder:
      builder.AddTraceFor(trace_data.CHROME_TRACE_PART,
                          {'traceEvents': [1, 2, 3]})
      builder.CleanUpTraceData()
      with self.assertRaises(RuntimeError):
        builder.AddTraceFor(trace_data.CHROME_TRACE_PART,
                            {'traceEvents': [1, 2, 3]})

  def testCleanupReraisesExceptions(self):
    with trace_data.TraceDataBuilder() as builder:
      try:
        raise Exception("test exception") # pylint: disable=broad-except
      except Exception: # pylint: disable=broad-except
        builder.RecordTraceDataException()
      with self.assertRaises(trace_data.TraceDataException):
        builder.CleanUpTraceData()

  def testCantWriteAfterFreeze(self):
    with trace_data.TraceDataBuilder() as builder:
      builder.AddTraceFor(trace_data.CHROME_TRACE_PART,
                          {'traceEvents': [1, 2, 3]})
      builder.Freeze()
      with self.assertRaises(RuntimeError):
        builder.AddTraceFor(trace_data.CHROME_TRACE_PART,
                            {'traceEvents': [1, 2, 3]})
