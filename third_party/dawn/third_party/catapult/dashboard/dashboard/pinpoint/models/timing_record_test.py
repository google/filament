# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
from unittest import mock

from dashboard.common import math_utils
from dashboard.pinpoint.models import job
from dashboard.pinpoint.models import job_state
from dashboard.pinpoint.models import timing_record
from dashboard.pinpoint import test


@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
class RecordTimingTest(test.TestCase):

  def assertClose(self, a, b):
    self.assertTrue(abs(a - b) < 0.0001)

  def _Job(self, args, started, completed, comparison_mode=None):
    j = job.Job.New((), ())
    j.comparison_mode = comparison_mode
    j.arguments = args
    j.started_time = started
    j.updated = completed

    return j

  def _RecordTiming(self, args, started, completed, comparison_mode=None):
    j = self._Job(args, started, completed, comparison_mode)

    timing_record.RecordJobTiming(j)

    return j

  def testRecord_Try_Success(self):
    j = self._RecordTiming(
        {
            'configuration': 'linux',
            'benchmark': 'foo',
            'story': 'bar'
        },
        datetime.datetime.now() - datetime.timedelta(minutes=1),
        datetime.datetime.now())

    q = timing_record.TimingRecord.query()
    results = q.fetch()

    self.assertEqual(1, len(results))

    _, tags = timing_record.GetSimilarHistoricalTimings(j)
    self.assertEqual(['try', 'linux', 'foo', 'bar'], tags)

  def testRecord_Performance_Success(self):
    j = self._RecordTiming(
        {
            'configuration': 'linux',
            'benchmark': 'foo',
            'story': 'bar'
        },
        datetime.datetime.now() - datetime.timedelta(minutes=1),
        datetime.datetime.now(),
        comparison_mode=job_state.PERFORMANCE)

    _, tags = timing_record.GetSimilarHistoricalTimings(j)
    self.assertEqual(['performance', 'linux', 'foo', 'bar'], tags)

  def testRecord_Once(self):
    j = self._RecordTiming(
        {
            'configuration': 'linux',
            'benchmark': 'foo',
            'story': 'bar'
        },
        datetime.datetime.now() - datetime.timedelta(minutes=1),
        datetime.datetime.now())

    timing_record.RecordJobTiming(j)

    q = timing_record.TimingRecord.query()
    results = q.fetch()

    self.assertEqual(1, len(results))

  def testGetSimilarHistoricalTimings_Same(self):
    now = datetime.datetime.now()
    self._RecordTiming(
        {
            'configuration': 'linux',
            'benchmark': 'foo',
            'story': 'bar1'
        }, now - datetime.timedelta(minutes=1), now)

    median = math_utils.Median(list(range(0, 10)))
    std_dev = math_utils.StandardDeviation(list(range(0, 10)))
    p90 = math_utils.Percentile(list(range(0, 10)), 0.9)
    for i in range(0, 10):
      j = self._RecordTiming(
          {
              'configuration': 'linux',
              'benchmark': 'foo',
              'story': 'bar2'
          }, now - datetime.timedelta(seconds=i), now)

    timings, tags = timing_record.GetSimilarHistoricalTimings(j)

    self.assertEqual(['try', 'linux', 'foo', 'bar2'], tags)
    self.assertClose(median, timings[0].total_seconds())
    self.assertClose(std_dev, timings[1].total_seconds())
    self.assertClose(p90, timings[2].total_seconds())

  def testGetSimilarHistoricalTimings_NoStory_MatchesBenchmark(self):
    now = datetime.datetime.now()

    self._RecordTiming(
        {
            'configuration': 'linux',
            'benchmark': 'foo1',
            'story': 'bar1'
        }, now - datetime.timedelta(hours=1), now)

    self._RecordTiming(
        {
            'configuration': 'linux',
            'benchmark': 'foo2',
            'story': 'bar1'
        }, now - datetime.timedelta(seconds=12), now)

    self._RecordTiming(
        {
            'configuration': 'linux',
            'benchmark': 'foo2',
            'story': 'bar2'
        }, now - datetime.timedelta(seconds=10), now)

    j = job.Job.New((), ())
    j.arguments = {
        'configuration': 'linux',
        'benchmark': 'foo2',
        'story': 'bar3'
    }

    timings, tags = timing_record.GetSimilarHistoricalTimings(j)

    self.assertEqual(['try', 'linux', 'foo2'], tags)
    self.assertEqual(11, timings[0].total_seconds())

  def testGetSimilarHistoricalTimings_NoBenchmark_MatchesConfiguration(self):
    now = datetime.datetime.now()

    self._RecordTiming(
        {
            'configuration': 'windows',
            'benchmark': 'foo1',
            'story': 'bar1'
        }, now - datetime.timedelta(seconds=14), now)

    self._RecordTiming(
        {
            'configuration': 'linux',
            'benchmark': 'foo1',
            'story': 'bar1'
        }, now - datetime.timedelta(seconds=14), now)

    self._RecordTiming(
        {
            'configuration': 'linux',
            'benchmark': 'foo2',
            'story': 'bar1'
        }, now - datetime.timedelta(seconds=12), now)

    self._RecordTiming(
        {
            'configuration': 'linux',
            'benchmark': 'foo2',
            'story': 'bar2'
        }, now - datetime.timedelta(seconds=10), now)

    j = job.Job.New((), ())
    j.arguments = {
        'configuration': 'linux',
        'benchmark': 'foo3',
        'story': 'bar1'
    }

    timings, tags = timing_record.GetSimilarHistoricalTimings(j)

    self.assertEqual(['try', 'linux'], tags)
    self.assertEqual(12, timings[0].total_seconds())

  def testGetSimilarHistoricalTimings_NoConfiguration_MatchesAnyTry(self):
    now = datetime.datetime.now()

    self._RecordTiming(
        {
            'configuration': 'mac',
            'benchmark': 'foo1',
            'story': 'bar1'
        }, now - datetime.timedelta(seconds=14), now)

    self._RecordTiming(
        {
            'configuration': 'windows',
            'benchmark': 'foo1',
            'story': 'bar1'
        }, now - datetime.timedelta(seconds=12), now)

    self._RecordTiming(
        {
            'configuration': 'linux',
            'benchmark': 'foo2',
            'story': 'bar2'
        }, now - datetime.timedelta(seconds=10), now)

    j = job.Job.New((), ())
    j.arguments = {
        'configuration': 'coco',
        'benchmark': 'foo1',
        'story': 'bar1'
    }

    timings, tags = timing_record.GetSimilarHistoricalTimings(j)

    self.assertEqual(['try'], tags)
    self.assertEqual(12, timings[0].total_seconds())

  def testRecord_Estimate_Recorded(self):
    now = datetime.datetime.now()
    j1 = self._Job(
        {
            'configuration': 'linux',
            'benchmark': 'foo',
            'story': 'bar'
        }, now - datetime.timedelta(minutes=5),
        now - datetime.timedelta(minutes=4))
    e1 = timing_record.GetSimilarHistoricalTimings(j1)
    timing_record.RecordJobTiming(j1)

    t1 = timing_record.TimingRecord.get_by_id(j1.job_id)

    self.assertIsNone(e1)
    self.assertIsNone(t1.estimate)

    j2 = self._RecordTiming(
        {
            'configuration': 'linux',
            'benchmark': 'foo',
            'story': 'bar'
        }, now - datetime.timedelta(minutes=3),
        now - datetime.timedelta(minutes=2))
    e2 = timing_record.GetSimilarHistoricalTimings(j2)
    timing_record.RecordJobTiming(j2)

    t2 = timing_record.TimingRecord.get_by_id(j2.job_id)

    self.assertEqual(
        int(e2[0][0].total_seconds()),
        int((t2.estimate - j2.started_time).total_seconds()))
