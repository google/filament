# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import copy
import json
import math
from multiprocessing.dummy import Pool as ThreadPool
import unittest

from six.moves import range  # pylint: disable=redefined-builtin
from tracing.value import histogram
from tracing.value.diagnostics import date_range
from tracing.value.diagnostics import diagnostic
from tracing.value.diagnostics import diagnostic_ref
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import related_event_set
from tracing.value.diagnostics import related_name_map
from tracing.value.diagnostics import reserved_infos
from tracing.value.diagnostics import unmergeable_diagnostic_set as ums

# pylint: disable=too-many-lines

class PercentToStringUnittest(unittest.TestCase):
  def testPercentToString(self):
    with self.assertRaises(Exception) as ex:
      histogram.PercentToString(-1)
    self.assertEqual(str(ex.exception), 'percent must be in [0,1]')

    with self.assertRaises(Exception) as ex:
      histogram.PercentToString(2)
    self.assertEqual(str(ex.exception), 'percent must be in [0,1]')

    self.assertEqual(histogram.PercentToString(0), '000')
    self.assertEqual(histogram.PercentToString(1), '100')

    with self.assertRaises(Exception) as ex:
      histogram.PercentToString(float('nan'))
    self.assertEqual(str(ex.exception), 'Unexpected percent')

    self.assertEqual(histogram.PercentToString(0.50), '050')
    self.assertEqual(histogram.PercentToString(0.95), '095')


class StatisticsUnittest(unittest.TestCase):
  def testFindHighIndexInSortedArray(self):
    self.assertEqual(histogram.FindHighIndexInSortedArray(
        list(range(0, -10, -1)), lambda x: x + 5), 6)

  def testUniformlySampleArray(self):
    self.assertEqual(len(histogram.UniformlySampleArray(
        list(range(10)), 5)), 5)

  def testUniformlySampleStream(self):
    samples = []
    histogram.UniformlySampleStream(samples, 1, 'A', 5)
    self.assertEqual(samples, ['A'])
    histogram.UniformlySampleStream(samples, 2, 'B', 5)
    histogram.UniformlySampleStream(samples, 3, 'C', 5)
    histogram.UniformlySampleStream(samples, 4, 'D', 5)
    histogram.UniformlySampleStream(samples, 5, 'E', 5)
    self.assertEqual(samples, ['A', 'B', 'C', 'D', 'E'])
    histogram.UniformlySampleStream(samples, 6, 'F', 5)
    self.assertEqual(len(samples), 5)

    samples = [0, 0, 0]
    histogram.UniformlySampleStream(samples, 1, 'G', 5)
    self.assertEqual(samples, ['G', 0, 0])

  def testMergeSampledStreams(self):
    samples = []
    histogram.MergeSampledStreams(samples, 0, ['A'], 1, 5)
    self.assertEqual(samples, ['A'])
    histogram.MergeSampledStreams(samples, 1, ['B', 'C', 'D', 'E'], 4, 5)
    self.assertEqual(samples, ['A', 'B', 'C', 'D', 'E'])
    histogram.MergeSampledStreams(samples, 9, ['F', 'G', 'H', 'I', 'J'], 7, 5)
    self.assertEqual(len(samples), 5)


class RangeUnittest(unittest.TestCase):
  def testAddValue(self):
    r = histogram.Range()
    self.assertEqual(r.empty, True)
    r.AddValue(1)
    self.assertEqual(r.empty, False)
    self.assertEqual(r.min, 1)
    self.assertEqual(r.max, 1)
    self.assertEqual(r.center, 1)
    r.AddValue(2)
    self.assertEqual(r.empty, False)
    self.assertEqual(r.min, 1)
    self.assertEqual(r.max, 2)
    self.assertEqual(r.center, 1.5)


class RunningStatisticsUnittest(unittest.TestCase):
  def _Run(self, data):
    running = histogram.RunningStatistics()
    for datum in data:
      running.Add(datum)
    return running

  def testStatistics(self):
    running = self._Run([1, 2, 3])
    self.assertEqual(running.sum, 6)
    self.assertEqual(running.mean, 2)
    self.assertEqual(running.min, 1)
    self.assertEqual(running.max, 3)
    self.assertEqual(running.variance, 1)
    self.assertEqual(running.stddev, 1)
    self.assertEqual(running.geometric_mean, math.pow(6, 1./3))
    self.assertEqual(running.count, 3)

    running = self._Run([2, 4, 4, 2])
    self.assertEqual(running.sum, 12)
    self.assertEqual(running.mean, 3)
    self.assertEqual(running.min, 2)
    self.assertEqual(running.max, 4)
    self.assertEqual(running.variance, 4./3)
    self.assertEqual(running.stddev, math.sqrt(4./3))
    self.assertAlmostEqual(running.geometric_mean, math.pow(64, 1./4))
    self.assertEqual(running.count, 4)

  def testMerge(self):
    def Compare(data1, data2):
      a_running = self._Run(data1 + data2)
      b_running = self._Run(data1).Merge(self._Run(data2))
      CompareRunningStatistics(a_running, b_running)
      a_running = histogram.RunningStatistics.FromDict(a_running.AsDict())
      CompareRunningStatistics(a_running, b_running)
      b_running = histogram.RunningStatistics.FromDict(b_running.AsDict())
      CompareRunningStatistics(a_running, b_running)

    def CompareRunningStatistics(a_running, b_running):
      self.assertEqual(a_running.sum, b_running.sum)
      self.assertEqual(a_running.mean, b_running.mean)
      self.assertEqual(a_running.min, b_running.min)
      self.assertEqual(a_running.max, b_running.max)
      self.assertAlmostEqual(a_running.variance, b_running.variance)
      self.assertAlmostEqual(a_running.stddev, b_running.stddev)
      self.assertAlmostEqual(a_running.geometric_mean, b_running.geometric_mean)
      self.assertEqual(a_running.count, b_running.count)

    Compare([], [])
    Compare([], [1, 2, 3])
    Compare([1, 2, 3], [])
    Compare([1, 2, 3], [10, 20, 100])
    Compare([1, 1, 1, 1, 1], [10, 20, 10, 40])


def ToJSON(x):
  return json.dumps(x, separators=(',', ':'), sort_keys=True)


class HistogramUnittest(unittest.TestCase):
  TEST_BOUNDARIES = histogram.HistogramBinBoundaries.CreateLinear(0, 1000, 10)

  def assertDeepEqual(self, a, b):
    self.assertEqual(ToJSON(a), ToJSON(b))

  def testDefaultBoundaries(self):
    hist = histogram.Histogram('', 'ms')
    self.assertEqual(len(hist.bins), 102)

    hist = histogram.Histogram('', 'tsMs')
    self.assertEqual(len(hist.bins), 1002)

    hist = histogram.Histogram('', 'n%')
    self.assertEqual(len(hist.bins), 22)

    hist = histogram.Histogram('', 'n%+')
    self.assertEqual(len(hist.bins), 22)

    hist = histogram.Histogram('', 'n%-')
    self.assertEqual(len(hist.bins), 22)

    hist = histogram.Histogram('', 'sizeInBytes')
    self.assertEqual(len(hist.bins), 102)

    hist = histogram.Histogram('', 'J')
    self.assertEqual(len(hist.bins), 52)

    hist = histogram.Histogram('', 'W')
    self.assertEqual(len(hist.bins), 52)

    hist = histogram.Histogram('', 'unitless')
    self.assertEqual(len(hist.bins), 52)

    hist = histogram.Histogram('', 'count')
    self.assertEqual(len(hist.bins), 22)

    hist = histogram.Histogram('', 'sigma')
    self.assertEqual(len(hist.bins), 52)

    hist = histogram.Histogram('', 'sigma_smallerIsBetter')
    self.assertEqual(len(hist.bins), 52)

    hist = histogram.Histogram('', 'sigma_biggerIsBetter')
    self.assertEqual(len(hist.bins), 52)

  def testSerializationSize(self):
    hist = histogram.Histogram('', 'unitless', self.TEST_BOUNDARIES)
    d = hist.AsDict()
    self.assertEqual(61, len(ToJSON(d)))
    self.assertIsNone(d.get('allBins'))
    self.assertDeepEqual(d, histogram.Histogram.FromDict(d).AsDict())

    hist.AddSample(100)
    d = hist.AsDict()
    self.assertEqual(152, len(ToJSON(d)))
    self.assertIsInstance(d['allBins'], dict)
    self.assertDeepEqual(d, histogram.Histogram.FromDict(d).AsDict())

    hist.AddSample(100)
    d = hist.AsDict()
    # SAMPLE_VALUES grew by "100,"
    self.assertEqual(156, len(ToJSON(d)))
    self.assertIsInstance(d['allBins'], dict)
    self.assertDeepEqual(d, histogram.Histogram.FromDict(d).AsDict())

    hist.AddSample(271, {'foo': generic_set.GenericSet(['bar'])})
    d = hist.AsDict()
    self.assertEqual(222, len(ToJSON(d)))
    self.assertIsInstance(d['allBins'], dict)
    self.assertDeepEqual(d, histogram.Histogram.FromDict(d).AsDict())

    # Add samples to most bins so that allBinsArray is more efficient than
    # allBinsDict.
    for i in range(10, 100):
      hist.AddSample(10 * i)
    d = hist.AsDict()
    self.assertEqual(651, len(ToJSON(d)))
    self.assertIsInstance(d['allBins'], list)
    self.assertDeepEqual(d, histogram.Histogram.FromDict(d).AsDict())

    # Lowering maxNumSampleValues takes a random sub-sample of the existing
    # sampleValues. We have deliberately set all samples to 3-digit numbers so
    # that the serialized size is constant regardless of which samples are
    # retained.
    hist.max_num_sample_values = 10
    d = hist.AsDict()
    self.assertEqual(343, len(ToJSON(d)))
    self.assertIsInstance(d['allBins'], list)
    self.assertDeepEqual(d, histogram.Histogram.FromDict(d).AsDict())

    # Test the case where 'allBins' isn't a list and we're attempting to index
    # an invalid bucket.
    e = copy.deepcopy(d)
    e['allBins'] = {'1000': {}}
    with self.assertRaises(histogram.InvalidBucketError):
      _ = histogram.Histogram.FromDict(e).AsDict()

  def testManyBinsRoundtrip(self):
    # In this test we want to create a histogram which will have less than half
    # of the bins populated, so we can force the bin compaction (instead of a
    # list of bins, use a dictionary of bins) to do the conversion. We ensure
    # that the dict key ordering is not going to be an issue in the
    # serialisation and deserialisation roundtrip.
    hist = histogram.Histogram('', 'unitless', self.TEST_BOUNDARIES)

    # 400 allows us to fit the elements in approximately log10(10000) bins which
    # should give us more empty bins than there are non-empty bins. We also add
    # samples from either end of the range, by treating odd numbers in the
    # beginning and even numbers as negative offsets from the max of the range
    # (10,000).
    for sample in range(0, 400):
      hist.AddSample(sample if sample % 2 else 10000 - sample)

    d = hist.AsDict()
    self.assertIsInstance(d['allBins'], dict)
    self.assertEqual(400, hist.num_values)
    self.assertEqual((len(hist.bins) / 2) - 1, len(d['allBins']))

    # Ensure that we can reconstitute a histogram properly from a dict.
    e = histogram.Histogram.FromDict(d)
    self.assertEqual(d, e.AsDict())

  def testManyBinsRoundtripProto(self):
    hist = histogram.Histogram('name', 'unitless', self.TEST_BOUNDARIES)

    for sample in range(0, 400):
      hist.AddSample(sample if sample % 2 else 10000 - sample)

    d = hist.AsProto()
    self.assertEqual(400, hist.num_values)
    self.assertEqual((len(hist.bins) / 2) - 1, len(d.all_bins))

    # Ensure that we can reconstitute a histogram properly from a proto.
    e = histogram.Histogram.FromProto(d)
    self.assertEqual(d, e.AsProto())

  def testBasic(self):
    hist = histogram.Histogram('', 'unitless', self.TEST_BOUNDARIES)
    self.assertEqual(hist.GetBinForValue(250).range.min, 200)
    self.assertEqual(hist.GetBinForValue(250).range.max, 300)

    hist.AddSample(-1)
    hist.AddSample(0)
    hist.AddSample(0)
    hist.AddSample(500)
    hist.AddSample(999)
    hist.AddSample(1000)
    self.assertEqual(hist.bins[0].count, 1)

    self.assertEqual(hist.GetBinForValue(0).count, 2)
    self.assertEqual(hist.GetBinForValue(500).count, 1)
    self.assertEqual(hist.GetBinForValue(999).count, 1)
    self.assertEqual(hist.bins[-1].count, 1)
    self.assertEqual(hist.num_values, 6)
    self.assertAlmostEqual(hist.average, 416.3333333)

  def testNans(self):
    hist = histogram.Histogram('', 'unitless', self.TEST_BOUNDARIES)
    hist.AddSample(None)
    hist.AddSample(float('nan'))
    self.assertEqual(hist.num_nans, 2)

  def testAddHistogramValid(self):
    hist0 = histogram.Histogram('', 'unitless', self.TEST_BOUNDARIES)
    hist1 = histogram.Histogram('', 'unitless', self.TEST_BOUNDARIES)
    hist0.AddSample(0)
    hist0.AddSample(None)
    hist1.AddSample(1)
    hist1.AddSample(float('nan'))
    hist0.AddHistogram(hist1)
    self.assertEqual(hist0.num_nans, 2)
    self.assertEqual(hist0.GetBinForValue(0).count, 2)

  def testAddHistogramInvalid(self):
    hist0 = histogram.Histogram(
        '', 'ms', histogram.HistogramBinBoundaries.CreateLinear(0, 1000, 10))
    hist1 = histogram.Histogram(
        '', 'unitless', histogram.HistogramBinBoundaries.CreateLinear(
            0, 1000, 10))
    hist2 = histogram.Histogram(
        '', 'ms', histogram.HistogramBinBoundaries.CreateLinear(0, 1001, 10))
    hist3 = histogram.Histogram(
        '', 'ms', histogram.HistogramBinBoundaries.CreateLinear(0, 1000, 11))
    hists = [hist0, hist1, hist2, hist3]
    for hista in hists:
      for histb in hists:
        if hista is histb:
          continue
        self.assertFalse(hista.CanAddHistogram(histb))
        with self.assertRaises(Exception):
          hista.AddHistogram(histb)

  def testPercentile(self):
    def Check(ary, mn, mx, bins, precision):
      boundaries = histogram.HistogramBinBoundaries.CreateLinear(mn, mx, bins)
      hist = histogram.Histogram('', 'ms', boundaries)
      for x in ary:
        hist.AddSample(x)
      for percent in [0.25, 0.5, 0.75, 0.8, 0.95, 0.99]:
        self.assertLessEqual(
            abs(histogram.Percentile(ary, percent) -
                hist.GetApproximatePercentile(percent)), precision)
    Check([1, 2, 5, 7], 0.5, 10.5, 10, 1e-3)
    Check([3, 3, 4, 4], 0.5, 10.5, 10, 1e-3)
    Check([1, 10], 0.5, 10.5, 10, 1e-3)
    Check([1, 2, 3, 4, 5], 0.5, 10.5, 10, 1e-3)
    Check([3, 3, 3, 3, 3], 0.5, 10.5, 10, 1e-3)
    Check([1, 2, 3, 4, 5, 6, 7, 8, 9, 10], 0.5, 10.5, 10, 1e-3)
    Check([1, 2, 3, 4, 5, 5, 6, 7, 8, 9, 10], 0.5, 10.5, 10, 1e-3)
    Check([0, 11], 0.5, 10.5, 10, 1)
    Check([0, 6, 11], 0.5, 10.5, 10, 1)
    array = []
    for i in range(1000):
      array.append((i * i) % 10 + 1)
    Check(array, 0.5, 10.5, 10, 1e-3)
    # If the real percentile is outside the bin range then the approximation
    # error can be high.
    Check([-10000], 0, 10, 10, 10000)
    Check([10000], 0, 10, 10, 10000 - 10)
    # The result is no more than the bin width away from the real percentile.
    Check([1, 1], 0, 10, 1, 10)

  def _CheckBoundaries(self, boundaries, expected_min_boundary,
                       expected_max_boundary, expected_bin_ranges):
    self.assertEqual(boundaries.range.min, expected_min_boundary)
    self.assertEqual(boundaries.range.max, expected_max_boundary)

    # Check that the boundaries can be used multiple times.
    for _ in range(3):
      hist = histogram.Histogram('', 'unitless', boundaries)
      self.assertEqual(len(expected_bin_ranges), len(hist.bins))
      for j, hbin in enumerate(hist.bins):
        self.assertAlmostEqual(hbin.range.min, expected_bin_ranges[j].min)
        self.assertAlmostEqual(hbin.range.max, expected_bin_ranges[j].max)

  def testAddBinBoundary(self):
    b = histogram.HistogramBinBoundaries(-100)
    b.AddBinBoundary(50)
    self._CheckBoundaries(b, -100, 50, [
        histogram.Range.FromExplicitRange(-histogram.JS_MAX_VALUE, -100),
        histogram.Range.FromExplicitRange(-100, 50),
        histogram.Range.FromExplicitRange(50, histogram.JS_MAX_VALUE),
    ])

    b.AddBinBoundary(60)
    b.AddBinBoundary(75)
    self._CheckBoundaries(b, -100, 75, [
        histogram.Range.FromExplicitRange(-histogram.JS_MAX_VALUE, -100),
        histogram.Range.FromExplicitRange(-100, 50),
        histogram.Range.FromExplicitRange(50, 60),
        histogram.Range.FromExplicitRange(60, 75),
        histogram.Range.FromExplicitRange(75, histogram.JS_MAX_VALUE),
    ])

  def testAddLinearBins(self):
    b = histogram.HistogramBinBoundaries(1000)
    b.AddLinearBins(1200, 5)
    self._CheckBoundaries(b, 1000, 1200, [
        histogram.Range.FromExplicitRange(-histogram.JS_MAX_VALUE, 1000),
        histogram.Range.FromExplicitRange(1000, 1040),
        histogram.Range.FromExplicitRange(1040, 1080),
        histogram.Range.FromExplicitRange(1080, 1120),
        histogram.Range.FromExplicitRange(1120, 1160),
        histogram.Range.FromExplicitRange(1160, 1200),
        histogram.Range.FromExplicitRange(1200, histogram.JS_MAX_VALUE),
    ])

  def testAddExponentialBins(self):
    b = histogram.HistogramBinBoundaries(0.5)
    b.AddExponentialBins(8, 4)
    self._CheckBoundaries(b, 0.5, 8, [
        histogram.Range.FromExplicitRange(-histogram.JS_MAX_VALUE, 0.5),
        histogram.Range.FromExplicitRange(0.5, 1),
        histogram.Range.FromExplicitRange(1, 2),
        histogram.Range.FromExplicitRange(2, 4),
        histogram.Range.FromExplicitRange(4, 8),
        histogram.Range.FromExplicitRange(8, histogram.JS_MAX_VALUE),
    ])

  def testBinBoundariesCombined(self):
    b = histogram.HistogramBinBoundaries(-273.15)
    b.AddBinBoundary(-50)
    b.AddLinearBins(4, 3)
    b.AddExponentialBins(16, 2)
    b.AddLinearBins(17, 4)
    b.AddBinBoundary(100)

    self._CheckBoundaries(b, -273.15, 100, [
        histogram.Range.FromExplicitRange(-histogram.JS_MAX_VALUE, -273.15),
        histogram.Range.FromExplicitRange(-273.15, -50),
        histogram.Range.FromExplicitRange(-50, -32),
        histogram.Range.FromExplicitRange(-32, -14),
        histogram.Range.FromExplicitRange(-14, 4),
        histogram.Range.FromExplicitRange(4, 8),
        histogram.Range.FromExplicitRange(8, 16),
        histogram.Range.FromExplicitRange(16, 16.25),
        histogram.Range.FromExplicitRange(16.25, 16.5),
        histogram.Range.FromExplicitRange(16.5, 16.75),
        histogram.Range.FromExplicitRange(16.75, 17),
        histogram.Range.FromExplicitRange(17, 100),
        histogram.Range.FromExplicitRange(100, histogram.JS_MAX_VALUE)
    ])

  def testBinBoundariesRaises(self):
    b = histogram.HistogramBinBoundaries(-7)
    with self.assertRaises(Exception):
      b.AddBinBoundary(-10)
    with self.assertRaises(Exception):
      b.AddBinBoundary(-7)
    with self.assertRaises(Exception):
      b.AddLinearBins(-10, 10)
    with self.assertRaises(Exception):
      b.AddLinearBins(-7, 10)
    with self.assertRaises(Exception):
      b.AddLinearBins(10, 0)
    with self.assertRaises(Exception):
      b.AddExponentialBins(16, 4)
    b = histogram.HistogramBinBoundaries(8)
    with self.assertRaises(Exception):
      b.AddExponentialBins(20, 0)
    with self.assertRaises(Exception):
      b.AddExponentialBins(5, 3)
    with self.assertRaises(Exception):
      b.AddExponentialBins(8, 3)

  def testStatisticsScalars(self):
    b = histogram.HistogramBinBoundaries.CreateLinear(0, 100, 100)
    hist = histogram.Histogram('', 'unitless', b)
    hist.AddSample(50)
    hist.AddSample(60)
    hist.AddSample(70)
    hist.AddSample('i am not a number')
    hist.CustomizeSummaryOptions({
        'count': True,
        'min': True,
        'max': True,
        'sum': True,
        'avg': True,
        'std': True,
        'nans': True,
        'geometricMean': True,
        'percentile': [0.5, 1],
        'ci': [0.01, 0.95],
    })

    # Test round-tripping summaryOptions
    hist = hist.Clone()
    stats = hist.statistics_scalars
    self.assertEqual(stats['nans'].unit, 'count_smallerIsBetter')
    self.assertEqual(stats['nans'].value, 1)
    self.assertEqual(stats['count'].unit, 'count_smallerIsBetter')
    self.assertEqual(stats['count'].value, 3)
    self.assertEqual(stats['min'].unit, hist.unit)
    self.assertEqual(stats['min'].value, 50)
    self.assertEqual(stats['max'].unit, hist.unit)
    self.assertEqual(stats['max'].value, 70)
    self.assertEqual(stats['sum'].unit, hist.unit)
    self.assertEqual(stats['sum'].value, 180)
    self.assertEqual(stats['avg'].unit, hist.unit)
    self.assertEqual(stats['avg'].value, 60)
    self.assertEqual(stats['std'].unit, hist.unit)
    self.assertEqual(stats['std'].value, 10)
    self.assertEqual(stats['pct_050'].unit, hist.unit)
    self.assertEqual(stats['pct_050'].value, 60.5)
    self.assertEqual(stats['pct_100'].unit, hist.unit)
    self.assertEqual(stats['pct_100'].value, 70.5)
    self.assertEqual(stats['geometricMean'].unit, hist.unit)
    self.assertLess(abs(stats['geometricMean'].value - 59.439), 1e-3)
    self.assertLess(stats['ci_095_lower'].value, stats['avg'].value)
    self.assertLess(stats['avg'].value, stats['ci_095_upper'].value)
    self.assertEqual(stats['ci_001_lower'].value, stats['avg'].value)
    self.assertEqual(stats['ci_001_upper'].value, stats['avg'].value)

    hist.CustomizeSummaryOptions({
        'count': False,
        'min': False,
        'max': False,
        'sum': False,
        'avg': False,
        'std': False,
        'nans': False,
        'geometricMean': False,
        'percentile': [],
        'ci': [],
    })
    self.assertEqual(0, len(hist.statistics_scalars))

  def testStatisticsScalarsEmpty(self):
    b = histogram.HistogramBinBoundaries.CreateLinear(0, 100, 100)
    hist = histogram.Histogram('', 'unitless', b)
    hist.CustomizeSummaryOptions({
        'count': True,
        'min': True,
        'max': True,
        'sum': True,
        'avg': True,
        'std': True,
        'nans': True,
        'geometricMean': True,
        'percentile': [0, 0.01, 0.1, 0.5, 0.995, 1],
        'ci': [0.1, 0.8],
    })
    stats = hist.statistics_scalars
    self.assertEqual(stats['nans'].value, 0)
    self.assertEqual(stats['count'].value, 0)
    self.assertNotIn('avg', stats)
    self.assertNotIn('stddev', stats)
    self.assertNotIn('pct_000', stats)
    self.assertNotIn('pct_001', stats)
    self.assertNotIn('pct_010', stats)
    self.assertNotIn('pct_050', stats)
    self.assertNotIn('pct_099_5', stats)
    self.assertNotIn('pct_100', stats)
    self.assertNotIn('ci_010_lower', stats)
    self.assertNotIn('ci_010_upper', stats)
    self.assertNotIn('ci_010', stats)
    self.assertNotIn('ci_080_lower', stats)
    self.assertNotIn('ci_080_upper', stats)
    self.assertNotIn('ci_080', stats)

  def testSampleValues(self):
    hist0 = histogram.Histogram('', 'unitless', self.TEST_BOUNDARIES)
    hist1 = histogram.Histogram('', 'unitless', self.TEST_BOUNDARIES)
    self.assertEqual(hist0.max_num_sample_values, 120)
    self.assertEqual(hist1.max_num_sample_values, 120)
    values0 = []
    values1 = []
    for i in range(10):
      values0.append(i)
      hist0.AddSample(i)
      values1.append(10 + i)
      hist1.AddSample(10 + i)
    self.assertDeepEqual(hist0.sample_values, values0)
    self.assertDeepEqual(hist1.sample_values, values1)
    hist0.AddHistogram(hist1)
    self.assertDeepEqual(hist0.sample_values, values0 + values1)
    hist2 = hist0.Clone()
    self.assertDeepEqual(hist2.sample_values, values0 + values1)

    for i in range(200):
      hist0.AddSample(i)
    self.assertEqual(len(hist0.sample_values), hist0.max_num_sample_values)

    hist3 = histogram.Histogram('', 'unitless', self.TEST_BOUNDARIES)
    hist3.max_num_sample_values = 10
    for i in range(100):
      hist3.AddSample(i)
    self.assertEqual(len(hist3.sample_values), 10)

  def testSingularBin(self):
    hist = histogram.Histogram(
        '', 'unitless', histogram.HistogramBinBoundaries.SINGULAR)
    self.assertEqual(1, len(hist.bins))
    d = hist.AsDict()
    self.assertNotIn('binBoundaries', d)
    clone = histogram.Histogram.FromDict(d)
    self.assertEqual(1, len(clone.bins))
    self.assertDeepEqual(d, clone.AsDict())

    self.assertEqual(0, hist.GetApproximatePercentile(0))
    self.assertEqual(0, hist.GetApproximatePercentile(1))
    hist.AddSample(0)
    self.assertEqual(0, hist.GetApproximatePercentile(0))
    self.assertEqual(0, hist.GetApproximatePercentile(1))
    hist.AddSample(1)
    self.assertEqual(0, hist.GetApproximatePercentile(0))
    self.assertEqual(1, hist.GetApproximatePercentile(1))
    hist.AddSample(2)
    self.assertEqual(0, hist.GetApproximatePercentile(0))
    self.assertEqual(1, hist.GetApproximatePercentile(0.5))
    self.assertEqual(2, hist.GetApproximatePercentile(1))
    hist.AddSample(3)
    self.assertEqual(0, hist.GetApproximatePercentile(0))
    self.assertEqual(1, hist.GetApproximatePercentile(0.5))
    self.assertEqual(2, hist.GetApproximatePercentile(0.9))
    self.assertEqual(3, hist.GetApproximatePercentile(1))
    hist.AddSample(4)
    self.assertEqual(0, hist.GetApproximatePercentile(0))
    self.assertEqual(1, hist.GetApproximatePercentile(0.4))
    self.assertEqual(2, hist.GetApproximatePercentile(0.7))
    self.assertEqual(3, hist.GetApproximatePercentile(0.9))
    self.assertEqual(4, hist.GetApproximatePercentile(1))

  def testFromDictMultithreaded(self):
    hdict = {
        "allBins": {"23": [1]},
        "binBoundaries": [0.001, [1, 100000, 30]],
        "name": "foo",
        "running": [1, 1, 1, 1, 1, 1, 0],
        "sampleValues": [1],
        "unit": "ms",
    }
    pool = ThreadPool(10)
    histograms = pool.map(histogram.Histogram.FromDict, [hdict] * 10)
    self.assertEqual(len(histograms), 10)
    for h in histograms:
      self.assertEqual(h.name, 'foo')

  def testProtoOnlyWritesSummaryOptionsIfNotDefault(self):
    hist = histogram.Histogram('name', 'unitless')

    with_defaults = hist.AsProto()

    hist.CustomizeSummaryOptions({
        'count': True,
        'min': False,
        'max': True,
        'sum': True,
        'avg': True,
        'std': True,
        'nans': True,
        'geometricMean': True,
        'percentile': [0.5, 1]
    })

    proto = hist.AsProto()

    self.assertFalse(with_defaults.HasField('summary_options'))
    self.assertTrue(proto.HasField('summary_options'))

  def testProtoRoundtrip(self):
    generic = generic_set.GenericSet(['generic diagnostic'])
    hist = histogram.Histogram(
        'name', 'count_smallerIsBetter', self.TEST_BOUNDARIES)
    hist.diagnostics['foo'] = generic

    hist.CustomizeSummaryOptions({
        'count': True,
        'min': False,
    })

    hist.max_num_sample_values = 84
    hist.description = "desc"

    hist.AddSample(17)
    hist.AddSample(18)
    hist.AddSample(-1)
    # TODO(http://crbug.com/1029452): uncomment once proto supports non-float
    # samples and nan diagnostics are supported by protos.
    # hist.AddSample('i am not a number', diagnostic_map=generic)

    clone = histogram.Histogram.FromProto(hist.AsProto())

    self.assertEqual(hist.sample_values, clone.sample_values)
    self.assertEqual(hist.description, clone.description)
    self.assertEqual(len(hist.diagnostics), len(clone.diagnostics))
    self.assertEqual(hist.diagnostics['foo'], clone.diagnostics['foo'])
    self.assertEqual(list(hist.statistics_scalars.keys()),
                     list(clone.statistics_scalars.keys()))
    self.assertEqual(hist.max_num_sample_values, clone.max_num_sample_values)

class DiagnosticMapUnittest(unittest.TestCase):
  def testDisallowReservedNames(self):
    diagnostics = histogram.DiagnosticMap()
    with self.assertRaises(TypeError):
      diagnostics[None] = generic_set.GenericSet(())
    with self.assertRaises(TypeError):
      diagnostics['generic'] = None
    diagnostics[reserved_infos.TRACE_URLS.name] = date_range.DateRange(0)
    diagnostics.DisallowReservedNames()
    diagnostics[reserved_infos.TRACE_URLS.name] = generic_set.GenericSet(())
    with self.assertRaises(TypeError):
      diagnostics[reserved_infos.TRACE_URLS.name] = date_range.DateRange(0)

  def testResetGuid(self):
    generic = generic_set.GenericSet(['generic diagnostic'])
    guid1 = generic.guid
    generic.ResetGuid()
    guid2 = generic.guid
    self.assertNotEqual(guid1, guid2)

  # TODO(eakuefner): Find a better place for these non-map tests once we
  # break up the Python implementation more.
  def testInlineSharedDiagnostic(self):
    generic = generic_set.GenericSet(['generic diagnostic'])
    hist = histogram.Histogram('', 'count')
    _ = generic.guid  # First access sets guid
    hist.diagnostics['foo'] = generic
    generic.Inline()
    self.assertFalse(generic.has_guid)
    hist_dict = hist.AsDict()
    diag_dict = hist_dict['diagnostics']['foo']
    self.assertIsInstance(diag_dict, dict)
    self.assertEqual(diag_dict['type'], 'GenericSet')

  def testCloneWithRef(self):
    diagnostics = histogram.DiagnosticMap()
    diagnostics['ref'] = diagnostic_ref.DiagnosticRef('abc')

    clone = histogram.DiagnosticMap.FromDict(diagnostics.AsDict())
    self.assertIsInstance(clone.get('ref'), diagnostic_ref.DiagnosticRef)
    self.assertEqual(clone.get('ref').guid, 'abc')

  def testDiagnosticGuidDeserialized(self):
    d = {
        'type': 'GenericSet',
        'values': [],
        'guid': 'bar'
    }
    g = diagnostic.Diagnostic.FromDict(d)
    self.assertEqual('bar', g.guid)

  def testMerge(self):
    events = related_event_set.RelatedEventSet()
    events.Add({
        'stableId': '0.0',
        'title': 'foo',
        'start': 0,
        'duration': 1,
    })
    generic = generic_set.GenericSet(['generic diagnostic'])
    generic2 = generic_set.GenericSet(['generic diagnostic 2'])
    related_map = related_name_map.RelatedNameMap()
    related_map.Set('a', 'histogram')

    hist = histogram.Histogram('', 'count')

    # When Histograms are merged, first an empty clone is created with an empty
    # DiagnosticMap.
    hist2 = histogram.Histogram('', 'count')
    hist2.diagnostics['a'] = generic
    hist.diagnostics.Merge(hist2.diagnostics)
    self.assertIs(generic, hist.diagnostics['a'])

    # Separate keys are not merged.
    hist3 = histogram.Histogram('', 'count')
    hist3.diagnostics['b'] = generic2
    hist.diagnostics.Merge(hist3.diagnostics)
    self.assertIs(generic, hist.diagnostics['a'])
    self.assertIs(generic2, hist.diagnostics['b'])

    # Merging unmergeable diagnostics should produce an
    # UnmergeableDiagnosticSet.
    hist4 = histogram.Histogram('', 'count')
    hist4.diagnostics['a'] = related_map
    hist.diagnostics.Merge(hist4.diagnostics)
    self.assertIsInstance(hist.diagnostics['a'], ums.UnmergeableDiagnosticSet)
    diagnostics = list(hist.diagnostics['a'])
    self.assertIs(generic, diagnostics[0])
    self.assertIs(related_map, diagnostics[1])

    # UnmergeableDiagnosticSets are mergeable.
    hist5 = histogram.Histogram('', 'count')
    hist5.diagnostics['a'] = ums.UnmergeableDiagnosticSet([events, generic2])
    hist.diagnostics.Merge(hist5.diagnostics)
    self.assertIsInstance(hist.diagnostics['a'], ums.UnmergeableDiagnosticSet)
    diagnostics = list(hist.diagnostics['a'])
    self.assertIs(generic, diagnostics[0])
    self.assertIs(related_map, diagnostics[1])
    self.assertIs(events, diagnostics[2])
    self.assertIs(generic2, diagnostics[3])
