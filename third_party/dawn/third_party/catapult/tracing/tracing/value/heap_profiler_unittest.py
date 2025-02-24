# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import unittest

from six.moves import range  # pylint: disable=redefined-builtin
from tracing.value import heap_profiler
from tracing.value import histogram
from tracing.value import histogram_set


class HeapProfilerUnitTest(unittest.TestCase):

  def testHeapProfiler(self):
    test_data = histogram_set.HistogramSet()
    for i in range(10):
      test_hist = histogram.Histogram('test', 'n%')
      test_hist.AddSample(i / 10.0)
      test_data.AddHistogram(test_hist)

    histograms = heap_profiler.Profile(test_data)

    set_size_hist = histograms.GetHistogramNamed('heap:HistogramSet')
    self.assertEqual(set_size_hist.num_values, 1)
    # The exact sizes of python objects can vary between platforms and versions.
    self.assertGreater(set_size_hist.sum, 10000)

    hist_size_hist = histograms.GetHistogramNamed('heap:Histogram')
    self.assertEqual(hist_size_hist.num_values, 10)
    self.assertGreater(hist_size_hist.sum, 10000)

    related_names = hist_size_hist.diagnostics['types']
    self.assertEqual(related_names.Get('HistogramBin'), 'heap:HistogramBin')
    self.assertEqual(related_names.Get('DiagnosticMap'), 'heap:DiagnosticMap')

    diagnostic_bin = [
        hist_bin for hist_bin in hist_size_hist.bins
        if len(hist_bin.diagnostic_maps)][-1]
    properties = diagnostic_bin.diagnostic_maps[0]['properties']
    types = diagnostic_bin.diagnostic_maps[0]['types']
    self.assertGreater(len(properties), 3)
    self.assertGreater(properties.Get('_bins'), 1000)
    self.assertEqual(len(types), 4)
    self.assertGreater(types.Get('HistogramBin'), 1000)
    self.assertGreater(types.Get('(builtin types)'), 1000)

    bin_size_hist = histograms.GetHistogramNamed('heap:HistogramBin')
    self.assertEqual(bin_size_hist.num_values, 32)
    self.assertGreater(bin_size_hist.sum, 1000)

    diag_map_size_hist = histograms.GetHistogramNamed('heap:DiagnosticMap')
    self.assertEqual(diag_map_size_hist.num_values, 10)
    self.assertGreater(diag_map_size_hist.sum, 700)

    range_size_hist = histograms.GetHistogramNamed('heap:Range')
    self.assertEqual(range_size_hist.num_values, 22)
    self.assertGreater(range_size_hist.sum, 1000)

    stats_size_hist = histograms.GetHistogramNamed('heap:RunningStatistics')
    self.assertEqual(stats_size_hist.num_values, 10)
    self.assertGreater(stats_size_hist.sum, 1000)
