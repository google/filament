# Copyright (c) 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from tracing.metrics import discover

class MetricsDiscoverUnittest(unittest.TestCase):
  def testMetricsDiscoverEmpty(self):
    self.assertFalse(discover.DiscoverMetrics([]))

  def testMetricsDiscoverNonEmpty(self):
    self.assertEqual(['sampleMetric'], discover.DiscoverMetrics(
        ['/tracing/metrics/sample_metric.html']))

  def testMetricsDiscoverMultipleMetrics(self):
    self.assertGreater(
        len(discover.DiscoverMetrics(
            ['/tracing/metrics/all_metrics.html'])), 1)
