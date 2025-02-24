# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import unittest

from six.moves import range  # pylint: disable=redefined-builtin
from tracing_build import snapdragon2trace


def _FakeSDSample(value, timestamp):
  return {
      'Process': 'Global',
      'Metric': 'GPU % Utilization',
      'Value': value,
      'TimestampRaw': timestamp}


class Snapdragon2traceTests(unittest.TestCase):

  def testAddSnapdragonProfilerData(self):
    snapdragon_csv = [_FakeSDSample(i + 1, 1000 + i * 100) for i in range(10)]
    traces = [{
        'traceEvents': [
            {'pid': 1, 'tid': 1, 'ph': 'X', 'dur': 10, 'ts': 350},
            {'pid': 1, 'tid': 1, 'ph': 'X', 'dur': 10, 'ts': 450},
            {'pid': 1, 'tid': 1, 'ph': 'X', 'dur': 10, 'ts': 550},
            {'pid': 2, 'tid': 2, 'ph': 'X', 'dur': 10, 'ts': 400},
            {'pid': 2, 'tid': 2, 'ph': 'X', 'dur': 100, 'ts': 650},
            {'pid': 3, 'tid': 3, 'ph': 'X', 'dur': 10, 'ts': 500},
            {'pid': 3, 'tid': 3, 'ph': 'X', 'dur': 10, 'ts': 550}],
        'metadata': {'clock-offset-since-epoch': '1000'}}]
    snapdragon2trace.AddSnapdragonProfilerData(traces, snapdragon_csv)

    # Just a sanity check that the Chrome trace events are not modified.
    self.assertEqual(7, len(traces[0]['traceEvents']))

    # Out of 10 SD counter events, 6 are out of the range.
    counter_events = [x for x in traces[1]['traceEvents'] if x['ph'] == 'C']
    self.assertEqual(4, len(counter_events))

    # Chrome trace events start from 350 and end at 750 (650 + 100). Snapdragon
    # counter events that are in this range have real timestamps 1400, 1500,
    # 1600, and 1700 which correspond to monotomic timestamps 400 to 700.
    self.assertEqual([400, 500, 600, 700], [x['ts'] for x in counter_events])
    self.assertEqual([5, 6, 7, 8], [x['args']['Value'] for x in counter_events])

    # We use a PID that is more that all PIDs in the Chrome trace.
    self.assertEqual([4, 4, 4, 4], [x['pid'] for x in counter_events])
