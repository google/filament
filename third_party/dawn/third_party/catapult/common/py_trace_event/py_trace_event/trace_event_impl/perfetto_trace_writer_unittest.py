#!/usr/bin/env python
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
from io import BytesIO

from py_trace_event.trace_event_impl import perfetto_trace_writer


class PerfettoTraceWriterTest(unittest.TestCase):
  """ Tests functions that write perfetto protobufs.

  TODO(crbug.com/944078): Switch to using python-protobuf library
  and implement proper protobuf parsing then.
  """
  def setUp(self):
    perfetto_trace_writer.reset_global_state()

  def testWriteThreadDescriptorEvent(self):
    result = BytesIO()
    perfetto_trace_writer.write_thread_descriptor_event(
        output=result,
        pid=1,
        tid=2,
        ts=1556716807306000,
    )
    expected_output = b"".join([
        b'\n\x1b@\x80\xec\xea\xda\x83\xb6\xa4\xcd\x15P\x80\x80@',
        b'\xc8\x02\x01\xe2\x02\x04\x08\x01\x10\x02\xd0\x03@'
    ])
    self.assertEqual(expected_output, result.getvalue())

  def testWriteTwoEvents(self):
    result = BytesIO()
    perfetto_trace_writer.write_thread_descriptor_event(
        output=result,
        pid=1,
        tid=2,
        ts=1556716807306000,
    )
    perfetto_trace_writer.write_event(
        output=result,
        ph="M",
        category="category",
        name="event_name",
        ts=1556716807406000,
        args={},
        tid=2,
    )
    expected_output = b"".join([
        b'\n\x1b@\x80\xec\xea\xda\x83\xb6\xa4\xcd\x15P\x80\x80@',
        b'\xc8\x02\x01\xe2\x02\x04\x08\x01\x10\x02\xd0\x03@\n;@',
        b'\x80\xb0\xc2\x8a\x84\xb6\xa4\xcd\x15P\x80\x80@Z\x08',
        b'\x18\x012\x04\x08\x01\x10Mb\x1e\n\x0c\x08\x01\x12\x08',
        b'category\x12\x0e\x08\x01\x12\nevent_name\xd0\x03@'
    ])
    self.assertEqual(expected_output, result.getvalue())

  def testWriteMetadata(self):
    result = BytesIO()
    perfetto_trace_writer.write_metadata(
        output=result,
        benchmark_start_time_us=1556716807306000,
        story_run_time_us=1556716807406000,
        benchmark_name="benchmark",
        benchmark_description="description",
        story_name="story",
        story_tags=["foo", "bar"],
        story_run_index=0,
        label="label",
    )
    expected_output = b"".join([
        b'\nG\x82\x03D\x08\x90\xf6\xc2\x82\xb6\xfa\xe1',
        b'\x02\x10\xb0\x83\xc9\x82\xb6\xfa\xe1\x02\x1a\tbenchmark"',
        b'\x0bdescription*\x05label2\x05story:\x03foo:\x03bar@\x00'
        ])
    self.assertEqual(expected_output, result.getvalue())

  def testWriteArgs(self):
    result = BytesIO()
    perfetto_trace_writer.write_thread_descriptor_event(
        output=result,
        pid=1,
        tid=2,
        ts=0,
    )
    perfetto_trace_writer.write_event(
        output=result,
        ph="M",
        category="",
        name="",
        ts=0,
        args={'int': 123, 'double': 1.23, 'string': 'onetwothree'},
        tid=2,
    )
    expected_output = b"".join([
        b'\n\x13@\x00P\x80\x80@\xc8\x02\x01\xe2\x02\x04\x08\x01\x10',
        b'\x02\xd0\x03@\nT@\x00P\x80\x80@Z;\x18\x01"\x07R\x03int {"\x11',
        b'R\x06double)\xaeG\xe1z\x14\xae\xf3?"\x15R\x06string2\x0bonetwothree2',
        b'\x04\x08\x01\x10Mb\x0c\n\x04\x08\x01\x12\x00\x12\x04\x08\x01\x12\x00',
        b'\xd0\x03@'
    ])
    self.assertEqual(expected_output, result.getvalue())

  def testWriteChromeMetadata(self):
    result = BytesIO()
    perfetto_trace_writer.write_chrome_metadata(
        output=result,
        clock_domain='FOO',
    )
    expected_output = b'\n\x17*\x15\x12\x13\n\x0cclock-domain\x12\x03FOO'
    self.assertEqual(expected_output, result.getvalue())

  def testWriteClockSnapshot(self):
    result = BytesIO()
    perfetto_trace_writer.write_clock_snapshot(
        output=result,
        tid=1,
        telemetry_ts=1234.567,
        boottime_ts=7654.321,
    )
    expected_output = b"".join([
       b'\n\x172\x11\n\x06\x08@\x10\x87\xadK\n\x07\x08\x06\x10',
       b'\xb1\x97\xd3\x03P\x80\x80@'
    ])
    self.assertEqual(expected_output, result.getvalue())
