# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from devil.android.perf import surface_stats_collector


class SurfaceStatsCollectorTests(unittest.TestCase):
  def testParseFrameData_simple(self):
    actual = surface_stats_collector.ParseFrameData([
        '16954612',
        '7657467895508   7657482691352   7657493499756',
        '7657484466553   7657499645964   7657511077881',
        '7657500793457   7657516600576   7657527404785',
    ],
                                                    parse_timestamps=True)
    self.assertEqual(
        actual, (16.954612, [7657482.691352, 7657499.645964, 7657516.600576]))

  def testParseFrameData_withoutTimestamps(self):
    actual = surface_stats_collector.ParseFrameData([
        '16954612',
        '7657467895508   7657482691352   7657493499756',
        '7657484466553   7657499645964   7657511077881',
        '7657500793457   7657516600576   7657527404785',
    ],
                                                    parse_timestamps=False)
    self.assertEqual(actual, (16.954612, []))

  def testParseFrameData_withWarning(self):
    actual = surface_stats_collector.ParseFrameData([
        'SurfaceFlinger appears to be unresponsive, dumping anyways',
        '16954612',
        '7657467895508   7657482691352   7657493499756',
        '7657484466553   7657499645964   7657511077881',
        '7657500793457   7657516600576   7657527404785',
    ],
                                                    parse_timestamps=True)
    self.assertEqual(
        actual, (16.954612, [7657482.691352, 7657499.645964, 7657516.600576]))
