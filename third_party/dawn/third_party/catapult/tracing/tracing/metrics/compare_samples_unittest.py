# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import json
import math
import os
import random
import tempfile
import unittest

from six.moves import range  # pylint: disable=redefined-builtin
from tracing.metrics import compare_samples


REJECT = 'REJECT'
FAIL_TO_REJECT = 'FAIL_TO_REJECT'
NEED_MORE_DATA = 'NEED_MORE_DATA'


def Mean(l):
  if len(l):
    return float(sum(l))/len(l)
  return 0


class CompareSamplesUnittest(unittest.TestCase):
  def setUp(self):
    self._tempfiles = []
    self._tempdir = tempfile.mkdtemp()

  def tearDown(self):
    for tf in self._tempfiles:
      try:
        os.remove(tf)
      except OSError:
        pass
    try:
      os.rmdir(self._tempdir)
    except OSError:
      pass

  def NewJsonTempfile(self, jsonable_contents):
    f_handle, new_json_file = tempfile.mkstemp(
        suffix='.json',
        dir=self._tempdir,
        text=True)
    os.close(f_handle)
    self._tempfiles.append(new_json_file)
    with open(new_json_file, 'w') as f:
      json.dump(jsonable_contents, f)
    return new_json_file

  def MakeMultipleChartJSONHistograms(self, metric, seed, mu, sigma, n, m):
    result = []
    random.seed(seed)
    for _ in range(m):
      result.append(self.MakeChartJSONHistogram(metric, mu, sigma, n))
    return result

  def MakeChartJSONHistogram(self, metric, mu, sigma, n):
    """Creates a histogram for a normally distributed pseudo-random sample.

    This function creates a deterministic pseudo-random sample and stores it in
    chartjson histogram format to facilitate the testing of the sample
    comparison logic.

    For simplicity we use sqrt(n) buckets with equal widths.

    Args:
      metric (str pair): name of chart, name of the trace.
      seed (hashable obj): to make the sequences deterministic we seed the RNG.
      mu (float): desired mean for the sample
      sigma (float): desired standard deviation for the sample
      n (int): number of values to generate.
    """
    chart_name, trace_name = metric
    values = [random.gauss(mu, sigma) for _ in range(n)]
    bucket_count = int(math.ceil(math.sqrt(len(values))))
    width = (max(values) - min(values))/(bucket_count - 1)
    prev_bucket = min(values)
    buckets = []
    for _ in range(bucket_count):
      buckets.append({'low': prev_bucket,
                      'high': prev_bucket + width,
                      'count': 0})
      prev_bucket += width
    for value in values:
      for bucket in buckets:
        if bucket['low'] <= value < bucket['high']:
          bucket['count'] += 1
          break
    charts = {
        'charts': {
            chart_name: {
                trace_name: {
                    'type': 'histogram',
                    'buckets': buckets
                }
            }
        }
    }
    return self.NewJsonTempfile(charts)

  def MakeChart(self, metric, seed, mu, sigma, n, keys=None):
    """Creates a normally distributed pseudo-random sample. (continuous).

    This function creates a deterministic pseudo-random sample and stores it in
    chartjson format to facilitate the testing of the sample comparison logic.

    Args:
      metric (str pair): name of chart, name of the trace.
      seed (hashable obj): to make the sequences deterministic we seed the RNG.
      mu (float): desired mean for the sample
      sigma (float): desired standard deviation for the sample
      n (int): number of values to generate.
    """
    chart_name, trace_name = metric
    random.seed(seed)
    values = [random.gauss(mu, sigma) for _ in range(n)]
    charts = {
        'charts': {
            chart_name: {
                trace_name: {
                    'type': 'list_of_scalar_values',
                    'values': values}
            }
        }
    }
    if keys:
      grouping_keys = dict(enumerate(keys))
      charts['charts'][chart_name][trace_name]['grouping_keys'] = grouping_keys
    return self.NewJsonTempfile(charts)

  def MakeNoneValuesChart(self, metric, keys=None):
    """Creates a chart with merged None values.

    Args:
      metric (str pair): name of chart, name of the trace.
    """
    chart_name, trace_name = metric
    charts = {
        'charts': {
            chart_name: {
                trace_name: {
                    'type': 'list_of_scalar_values',
                    'values': None
                }
            }
        }
    }
    if keys:
      grouping_keys = dict(enumerate(keys))
      charts['charts'][chart_name][trace_name]['grouping_keys'] = grouping_keys
    return self.NewJsonTempfile(charts)

  def MakeCharts(self, metric, seed, mu, sigma, n, keys=None):
    return [
        self.MakeChartJSONScalar(metric, seed + '%d' % i, mu, sigma, keys)
        for i in range(n)]

  def MakeChartJSONScalar(self, metric, seed, mu, sigma, keys=None):
    """Creates a normally distributed pseudo-random sample. (continuous).

    This function creates a deterministic pseudo-random sample and stores it in
    chartjson format to facilitate the testing of the sample comparison logic.

    Args:
      metric (str pair): name of chart, name of the trace.
      seed (hashable obj): to make the sequences deterministic we seed the RNG.
      mu (float): desired mean for the sample
      sigma (float): desired standard deviation for the sample
    """
    chart_name, trace_name = metric
    random.seed(seed)
    charts = {
        'charts': {
            chart_name: {
                trace_name: {
                    'type': 'scalar',
                    'value': random.gauss(mu, sigma)}
            }
        }
    }
    if keys:
      grouping_keys = dict(enumerate(keys))
      charts['charts'][chart_name][trace_name]['grouping_keys'] = grouping_keys
    return self.NewJsonTempfile(charts)

  def testCompareClearRegressionListOfScalars(self):
    metric = ('some_chart', 'some_trace')
    lower_values = ','.join(self.MakeCharts(metric=metric, seed='lower',
                                            mu=10, sigma=1, n=10))
    higher_values = ','.join(self.MakeCharts(metric=metric, seed='higher',
                                             mu=20, sigma=2, n=10))
    result = json.loads(compare_samples.CompareSamples(
        lower_values, higher_values, '/'.join(metric)).stdout)
    self.assertEqual(result['result']['significance'], REJECT)

  def testCompareListOfScalarsWithNoneValue(self):
    metric = ('some_chart', 'some_trace')
    lower_values = ','.join(self.MakeCharts(metric=metric, seed='lower',
                                            mu=10, sigma=1, n=10))
    lower_values += ',' + self.MakeNoneValuesChart(metric=metric)
    higher_values = ','.join(self.MakeCharts(metric=metric, seed='higher',
                                             mu=20, sigma=2, n=10))
    result = json.loads(compare_samples.CompareSamples(
        lower_values, higher_values, '/'.join(metric)).stdout)
    self.assertEqual(result['result']['significance'], REJECT)

  def testCompareClearRegressionScalars(self):
    metric = ('some_chart', 'some_trace')
    lower_values = ','.join(
        [self.MakeChartJSONScalar(
            metric=metric, seed='lower', mu=10, sigma=1) for _ in range(10)])
    higher_values = ','.join(
        [self.MakeChartJSONScalar(
            metric=metric, seed='higher', mu=20, sigma=2) for _ in range(10)])
    result = json.loads(compare_samples.CompareSamples(
        lower_values, higher_values, '/'.join(metric)).stdout)
    self.assertEqual(result['result']['significance'], REJECT)

  def testCompareUnlikelyRegressionWithMultipleRuns(self):
    metric = ('some_chart', 'some_trace')
    lower_values = ','.join(
        self.MakeCharts(
            metric=metric, seed='lower', mu=10, sigma=1, n=20))
    higher_values = ','.join(
        self.MakeCharts(
            metric=metric, seed='higher', mu=10.01, sigma=0.95, n=20))
    result = json.loads(compare_samples.CompareSamples(
        lower_values, higher_values, '/'.join(metric)).stdout)
    self.assertEqual(result['result']['significance'], FAIL_TO_REJECT)

  def testCompareGroupingLabel(self):
    parts = ('some_chart', 'some_label', 'some_trace')
    metric_name = ('%s@@%s' % (parts[1], parts[0]), parts[2])
    lower_values = ','.join(self.MakeCharts(
        metric=metric_name, seed='lower', mu=10, sigma=1, n=10))
    higher_values = ','.join(self.MakeCharts(
        metric=metric_name, seed='higher', mu=20, sigma=2, n=10))
    result = json.loads(compare_samples.CompareSamples(
        lower_values, higher_values, '/'.join(parts)).stdout)
    self.assertEqual(result['result']['significance'], REJECT)

  def testCompareGroupingLabelMissingSummary(self):
    parts = ('some_chart', 'some_label')
    metric_name = ('%s@@%s' % (parts[1], parts[0]), 'summary')
    lower_values = ','.join(self.MakeCharts(
        metric=metric_name, seed='lower', mu=10, sigma=1, n=10))
    higher_values = ','.join(self.MakeCharts(
        metric=metric_name, seed='higher', mu=20, sigma=2, n=10))
    result = json.loads(compare_samples.CompareSamples(
        lower_values, higher_values, '/'.join(parts)).stdout)
    self.assertEqual(result['result']['significance'], REJECT)

  def testCompareInsufficientData(self):
    metric = ('some_chart', 'some_trace')
    lower_values = ','.join([self.MakeChart(metric=metric, seed='lower',
                                            mu=10, sigma=1, n=5)])
    higher_values = ','.join([self.MakeChart(metric=metric, seed='higher',
                                             mu=10.40, sigma=0.95, n=5)])
    result = json.loads(compare_samples.CompareSamples(
        lower_values, higher_values, '/'.join(metric)).stdout)
    self.assertEqual(result['result']['significance'], NEED_MORE_DATA)

  def testCompareMissingFile(self):
    metric = ('some_chart', 'some_trace')
    lower_values = ','.join([self.MakeChart(metric=metric, seed='lower',
                                            mu=10, sigma=1, n=5)])
    higher_values = '/path/does/not/exist.json'
    with self.assertRaises(RuntimeError):
      compare_samples.CompareSamples(
          lower_values, higher_values, '/'.join(metric))

  def testCompareMissingMetric(self):
    metric = ('some_chart', 'some_trace')
    lower_values = ','.join([self.MakeChart(metric=metric, seed='lower',
                                            mu=10, sigma=1, n=5)])
    higher_values = ','.join([self.MakeChart(metric=metric, seed='higher',
                                             mu=20, sigma=2, n=5)])
    metric = ('some_chart', 'missing_trace')
    result = json.loads(compare_samples.CompareSamples(
        lower_values, higher_values, '/'.join(metric)).stdout)
    self.assertEqual(result['result']['significance'], NEED_MORE_DATA)

  def testCompareBadChart(self):
    metric = ('some_chart', 'some_trace')
    lower_values = ','.join([self.MakeChart(metric=metric, seed='lower',
                                            mu=10, sigma=1, n=5)])
    higher_values = self.NewJsonTempfile(['obviously', 'not', 'a', 'chart]'])
    result = json.loads(compare_samples.CompareSamples(
        lower_values, higher_values, '/'.join(metric)).stdout)
    self.assertEqual(result['result']['significance'], NEED_MORE_DATA)

  def testCompareBuildbotOutput(self):
    bb = os.path.join(os.path.dirname(__file__),
                      'buildbot_output_for_compare_samples_test.txt')
    result = compare_samples.CompareSamples(
        bb, bb, 'DrawCallPerf_gl/score',
        data_format='buildbot')
    result = json.loads(result.stdout)
    self.assertEqual(result['result']['significance'], NEED_MORE_DATA)
    self.assertEqual(Mean(result['sampleA']), 4123)
    self.assertEqual(Mean(result['sampleB']), 4123)

  def testCompareChartJsonHistogram(self):
    metric = ('some_chart', 'some_trace')
    lower_values = ','.join(self.MakeMultipleChartJSONHistograms(
        metric=metric, seed='lower', mu=10, sigma=1, n=100, m=10))
    higher_values = ','.join(self.MakeMultipleChartJSONHistograms(
        metric=metric, seed='higher', mu=20, sigma=2, n=100, m=10))
    result = json.loads(compare_samples.CompareSamples(
        lower_values, higher_values, '/'.join(metric)).stdout)
    self.assertEqual(result['result']['significance'], REJECT)

  def testParseComplexMetricName(self):
    full_metric_name = ('memory:chrome:all_processes:reported_by_os:'
                        'system_memory:native_heap:'
                        'proportional_resident_size_avg/blank_about/'
                        'blank_about_blank')
    chart_name = ('blank_about@@memory:chrome:all_processes:reported_by_os:'
                  'system_memory:native_heap:proportional_resident_size_avg')
    trace_name = 'blank:about:blank'
    metric = chart_name, trace_name
    keys = 'blank', 'about'
    lower_values = ','.join(self.MakeCharts(metric=metric, seed='lower',
                                            mu=10, sigma=1, n=10, keys=keys))
    higher_values = ','.join(self.MakeCharts(metric=metric, seed='higher',
                                             mu=20, sigma=2, n=10, keys=keys))
    result = compare_samples.CompareSamples(
        lower_values, higher_values, full_metric_name).stdout
    print(result)
    result = json.loads(result)
    self.assertEqual(result['result']['significance'], REJECT)
