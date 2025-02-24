# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import os
import shutil
import tempfile
import unittest

from tracing.trace_data import trace_data
from tracing_build import merge_traces


def _FakeDumpEvent(pid, tid):
  return {'ph': 'v', 'ts': 100, 'pid': pid, 'tid': tid, 'args': {'dumps': {}}}


class MergeTracesTest(unittest.TestCase):
  def setUp(self):
    self.test_dir = tempfile.mkdtemp()

  def tearDown(self):
    shutil.rmtree(self.test_dir)

  def _SerializeTrace(self, filename, trace):
    filepath = os.path.join(self.test_dir, filename)
    with trace_data.TraceDataBuilder() as builder:
      builder.AddTraceFor(trace_data.CHROME_TRACE_PART, trace)
      builder.Serialize(filepath)
    return filepath

  def testSimple(self):
    """Simple integration test for the main MergeTraceFiles function."""
    trace1 = self._SerializeTrace(
        'trace1.html', {'traceEvents': [_FakeDumpEvent(pid=1, tid=3)]})
    trace2 = self._SerializeTrace(
        'trace2.html', {'traceEvents': [_FakeDumpEvent(pid=2, tid=4)]})
    merged = os.path.join(self.test_dir, 'merged.json')
    merge_traces.MergeTraceFiles([trace1, trace2], merged)
    with open(merged) as f:
      events = json.load(f)['traceEvents']
    # Check that both dumps are found in the merged trace.
    dump_pids = [e['pid'] for e in events if e['ph'] == 'v']
    self.assertEqual([1, 2], dump_pids)
