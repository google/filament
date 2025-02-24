# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.common import bot_configurations
from dashboard.common import descriptor
from dashboard.common import namespaced_stored_object
from dashboard.common import report_query
from dashboard.common import stored_object
from dashboard.common import testing_common
from dashboard.models import anomaly
from dashboard.models import graph_data
from tracing.value import histogram as histogram_module


class ReportQueryTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    stored_object.Set(descriptor.PARTIAL_TEST_SUITES_KEY, [])
    stored_object.Set(descriptor.COMPOSITE_TEST_SUITES_KEY, [])
    stored_object.Set(descriptor.GROUPABLE_TEST_SUITE_PREFIXES_KEY, [])
    namespaced_stored_object.Set(bot_configurations.BOT_CONFIGURATIONS_KEY, {
        'a': {
            'alias': 'b',
        },
        'b': {},
        'c': {
            'alias': 'b',
        },
    })
    descriptor.Descriptor.ResetMemoizedConfigurationForTesting()

  def testBotAliases(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/a/suite/measure',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=100).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/b/suite/measure',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=20, parent=test.key, value=200).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:b'],
            'measurement': 'measure',
            'testCases': [],
        },],
        'statistics': ['avg'],
    }
    report = report_query.ReportQuery(template, [10, 20]).FetchSync()
    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][10]['statistics'])
    self.assertEqual(100, stats.mean)
    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][20]['statistics'])
    self.assertEqual(200, stats.mean)

  def testEmptyTestCases(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=100).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': [],
        },],
        'statistics': ['avg'],
    }
    report = report_query.ReportQuery(template, [10]).FetchSync()
    self.assertIsNone(
        report['rows'][0]['data'][10]['descriptors'][0]['testCase'])

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][10]['statistics'])
    self.assertEqual(100, stats.mean)

  def testMultipleRevisions(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=2, id=10, parent=test.key, value=20).put()
    graph_data.Row(error=3, id=20, parent=test.key, value=30).put()
    graph_data.Row(error=4, id=30, parent=test.key, value=40).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': [],
        },],
        'statistics': ['avg', 'std'],
    }
    report = report_query.ReportQuery(template, [10, 20, 30]).FetchSync()

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][10]['statistics'])
    self.assertEqual(20, stats.mean)
    self.assertEqual(2, stats.stddev)

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][20]['statistics'])
    self.assertEqual(30, stats.mean)
    self.assertEqual(3, stats.stddev)

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][30]['statistics'])
    self.assertEqual(40, stats.mean)
    self.assertEqual(4, stats.stddev)

  def testLatestRevision(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=2, id=10, parent=test.key, value=20).put()
    graph_data.Row(error=4, id=20, parent=test.key, value=40).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': [],
        },],
        'statistics': ['avg'],
    }
    report = report_query.ReportQuery(template, ['latest']).FetchSync()

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data']['latest']['statistics'])
    self.assertEqual(40, stats.mean)
    self.assertEqual(4, stats.stddev)

  def testMultipleRows(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/a',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=10).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/b',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=20).put()

    template = {
        'rows': [
            {
                'testSuites': ['suite'],
                'bots': ['master:bot'],
                'measurement': 'a',
                'testCases': [],
            },
            {
                'testSuites': ['suite'],
                'bots': ['master:bot'],
                'measurement': 'b',
                'testCases': [],
            },
        ],
        'statistics': ['avg'],
    }
    report = report_query.ReportQuery(template, [10]).FetchSync()

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][10]['statistics'])
    self.assertEqual(10, stats.mean)

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][1]['data'][10]['statistics'])
    self.assertEqual(20, stats.mean)

  def testMultipleTestSuites(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/a/measure',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=1, id=10, parent=test.key, value=10).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/b/measure',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=2, id=10, parent=test.key, value=20).put()

    template = {
        'rows': [{
            'testSuites': ['a', 'b'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': [],
        },],
        'statistics': ['avg'],
    }
    report = report_query.ReportQuery(template, [10]).FetchSync()

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][10]['statistics'])
    self.assertEqual(15, stats.mean)

  def testMultipleBots(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/a/suite/measure',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=1, id=10, parent=test.key, value=10).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/b/suite/measure',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=2, id=10, parent=test.key, value=20).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:a', 'master:b'],
            'measurement': 'measure',
            'testCases': [],
        },],
        'statistics': ['avg'],
    }
    report = report_query.ReportQuery(template, [10, 20]).FetchSync()

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][10]['statistics'])
    self.assertEqual(15, stats.mean)

  def testMultipleTestCases(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure/a',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=1, id=10, parent=test.key, value=10).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure/b',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=2, id=10, parent=test.key, value=20).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': ['a', 'b'],
        },],
        'statistics': ['avg'],
    }
    report = report_query.ReportQuery(template, [10]).FetchSync()

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][10]['statistics'])
    self.assertEqual(15, stats.mean)

  def testIgnoreNewTestCases(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure/a',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=1, id=10, parent=test.key, value=10).put()
    graph_data.Row(error=1, id=20, parent=test.key, value=10).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure/b',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=2, id=20, parent=test.key, value=20).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': ['a', 'b'],
        },],
        'statistics': ['avg'],
    }
    report = report_query.ReportQuery(template, [10, 20]).FetchSync()

    data = report['rows'][0]['data']
    self.assertEqual(1, len(data[10]['descriptors']))
    self.assertEqual('a', data[10]['descriptors'][0]['testCase'])
    self.assertEqual(1, len(data[20]['descriptors']))
    self.assertEqual('a', data[20]['descriptors'][0]['testCase'])

    stats = histogram_module.RunningStatistics.FromDict(data[10]['statistics'])
    self.assertEqual(10, stats.mean)

  def testCloseRevisions(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure/a',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=1, id=9, parent=test.key, value=10).put()
    graph_data.Row(error=1, id=19, parent=test.key, value=10).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure/b',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=2, id=9, parent=test.key, value=20).put()
    graph_data.Row(error=2, id=19, parent=test.key, value=20).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': ['a', 'b'],
        },],
        'statistics': ['avg'],
    }
    report = report_query.ReportQuery(template, [10, 20]).FetchSync()

    data = report['rows'][0]['data']
    self.assertEqual(2, len(data[10]['descriptors']))
    self.assertEqual(2, len(data[20]['descriptors']))

    stats = histogram_module.RunningStatistics.FromDict(data[10]['statistics'])
    self.assertEqual(15, stats.mean)

    stats = histogram_module.RunningStatistics.FromDict(data[20]['statistics'])
    self.assertEqual(15, stats.mean)

  def testIgnoreRemovedTestCases(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure/a',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=1, id=10, parent=test.key, value=10).put()
    graph_data.Row(error=1, id=20, parent=test.key, value=10).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure/b',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=2, id=10, parent=test.key, value=20).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': ['a', 'b'],
        },],
        'statistics': ['avg'],
    }
    report = report_query.ReportQuery(template, [10, 20]).FetchSync()

    data = report['rows'][0]['data']
    self.assertEqual(1, len(data[10]['descriptors']))
    self.assertEqual('a', data[10]['descriptors'][0]['testCase'])
    self.assertEqual(1, len(data[20]['descriptors']))
    self.assertEqual('a', data[20]['descriptors'][0]['testCase'])

    stats = histogram_module.RunningStatistics.FromDict(data[10]['statistics'])
    self.assertEqual(10, stats.mean)

  def testOldStyleUnsuffixedDataRows(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(error=10, id=10, parent=test.key, value=100).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': [],
        },],
        'statistics': ['avg', 'std', 'count'],
    }
    report = report_query.ReportQuery(template, [10]).FetchSync()
    self.assertIsNone(
        report['rows'][0]['data'][10]['descriptors'][0]['testCase'])

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][10]['statistics'])
    self.assertEqual(100, stats.mean)
    self.assertEqual(10, stats.stddev)
    self.assertEqual(10, stats.count)

  def testNewStyleUnsuffixedDataRows(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(
        error=10,
        id=10,
        parent=test.key,
        d_std=20,
        d_count=30,
        d_min=40,
        d_max=200,
        value=100).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': [],
        },],
        'statistics': ['avg', 'std', 'count', 'min', 'max'],
    }
    report = report_query.ReportQuery(template, [10]).FetchSync()

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][10]['statistics'])
    self.assertEqual(100, stats.mean)
    self.assertEqual(40, stats.min)
    self.assertEqual(200, stats.max)
    self.assertEqual(20, stats.stddev)
    self.assertEqual(30, stats.count)

  def testSuffixedDataRows(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_avg',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=10).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_std',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=20).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_count',
        improvement_direction=anomaly.DOWN,
        units='count')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=30).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_min',
        improvement_direction=anomaly.DOWN,
        units='count')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=4).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_max',
        improvement_direction=anomaly.DOWN,
        units='count')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=500).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': [],
        },],
        'statistics': ['avg', 'std', 'count', 'min', 'max', 'sum'],
    }
    report = report_query.ReportQuery(template, [10]).FetchSync()

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][10]['statistics'])
    self.assertEqual(10, stats.mean)
    self.assertEqual(4, stats.min)
    self.assertEqual(500, stats.max)
    self.assertEqual(20, stats.stddev)
    self.assertEqual(30, stats.count)
    self.assertEqual(300, stats.sum)

  def testFallBackToSuffixedTests(self):
    # Unsuffixed tests without data rows should fall back.
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_avg',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=10).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': [],
        },],
        'statistics': ['avg'],
    }
    report = report_query.ReportQuery(template, [10]).FetchSync()

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][10]['statistics'])
    self.assertEqual(10, stats.mean)

  def testIgnoreSuffixedDataRowsMissingAvg(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_avg/a',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=10).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_std/a',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=20).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_std/b',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=20).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_count/a',
        improvement_direction=anomaly.DOWN,
        units='count')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=30).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_count/b',
        improvement_direction=anomaly.DOWN,
        units='count')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=30).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_min/a',
        improvement_direction=anomaly.DOWN,
        units='count')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=4).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_min/b',
        improvement_direction=anomaly.DOWN,
        units='count')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=4).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_max/a',
        improvement_direction=anomaly.DOWN,
        units='count')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=500).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_max/b',
        improvement_direction=anomaly.DOWN,
        units='count')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=500).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': ['a', 'b'],
        },],
        'statistics': ['avg', 'std', 'count'],
    }
    report = report_query.ReportQuery(template, [10]).FetchSync()

    data = report['rows'][0]['data']
    self.assertEqual(1, len(data[10]['descriptors']))
    self.assertEqual('a', data[10]['descriptors'][0]['testCase'])

    stats = histogram_module.RunningStatistics.FromDict(
        report['rows'][0]['data'][10]['statistics'])
    self.assertEqual(10, stats.mean)
    self.assertEqual(4, stats.min)
    self.assertEqual(500, stats.max)
    self.assertEqual(20, stats.stddev)
    self.assertEqual(30, stats.count)
    self.assertEqual(300, stats.sum)

  def testIgnoreWrongUnits(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure/a',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=100).put()

    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure/b',
        improvement_direction=anomaly.DOWN,
        units='wrong')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(id=10, parent=test.key, value=100).put()

    template = {
        'rows': [{
            'testSuites': ['suite'],
            'bots': ['master:bot'],
            'measurement': 'measure',
            'testCases': ['a', 'b'],
        },],
        'statistics': ['avg'],
    }
    report = report_query.ReportQuery(template, [10]).FetchSync()

    self.assertEqual('units', report['rows'][0]['units'])
    self.assertEqual(1, len(report['rows'][0]['data'][10]['descriptors']))
    self.assertEqual(
        'a', report['rows'][0]['data'][10]['descriptors'][0]['testCase'])


if __name__ == '__main__':
  unittest.main()
