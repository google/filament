# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import datetime

from google.appengine.ext import ndb
from dashboard.common import math_utils

FETCH_LIMIT = 500

Timings = collections.namedtuple('Timings',
                                 ('median', 'standard_deviation', 'p90'))

EstimateResult = collections.namedtuple('EstimateResult', ('timings', 'tags'))


class TimingRecord(ndb.Model):
  started = ndb.DateTimeProperty(indexed=False, required=True)
  completed = ndb.DateTimeProperty(indexed=True, required=True)
  estimate = ndb.DateTimeProperty(indexed=False)
  tags = ndb.StringProperty(indexed=True, repeated=True)


def GetSimilarHistoricalTimings(job):
  """Gets historical timing data for similar jobs.

  This returns historical data for jobs run on Pinpoint previously, with
  arguments that are similar to specified job.

  Returns a tuple ((median, std_dev, 90th percentil), matching_tags) if similar
  jobs were found, otherwise None.

  Arguments:
  - job: a job entity instance.
  """
  tags = _JobTags(job)

  return _Estimate(tags)


def RecordJobTiming(job):
  tags = _JobTags(job)

  # Calculate the estimated completion using data before the job's starting
  # time. Can use this later to get an idea of how accurate the estimates are.
  estimate = _Estimate(tags, job.started_time)
  if estimate:
    estimate = job.started_time + estimate.timings.median

  e = TimingRecord(
      id=job.job_id,
      started=job.started_time,
      completed=job.updated,
      estimate=estimate,
      tags=tags)
  e.put()


def _JobTags(job):
  tags = [
      _ComparisonMode(job), job.configuration,
      job.arguments.get('benchmark', ''),
      job.arguments.get('story', '')
  ]

  tags = [t for t in tags if t]

  return tags


def _ComparisonMode(job):
  cmp_mode = job.comparison_mode
  if not cmp_mode:
    cmp_mode = 'try'
  return cmp_mode


def _Estimate(tags, completed_before=None):
  records = _QueryTimingRecords(tags, completed_before)

  if not records:
    if tags:
      return _Estimate(tags[:-1])
    return None

  times = [(r.completed - r.started).total_seconds() for r in records]

  median = math_utils.Median(times)
  std_dev = math_utils.StandardDeviation(times)
  p90 = math_utils.Percentile(times, 0.9)
  timings = Timings(
      datetime.timedelta(seconds=median), datetime.timedelta(seconds=std_dev),
      datetime.timedelta(seconds=p90))

  return EstimateResult(timings, tags)


def _QueryTimingRecords(tags, completed_before):
  q = TimingRecord.query()
  for t in tags:
    q = q.filter(TimingRecord.tags == t)
  q = q.order(-TimingRecord.completed)

  if completed_before:
    q = q.filter(TimingRecord.completed < completed_before)

  return q.fetch(limit=FETCH_LIMIT)
