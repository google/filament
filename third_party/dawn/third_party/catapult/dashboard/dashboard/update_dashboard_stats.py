# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import datetime
from flask import make_response
from six.moves import http_client
import time

from google.appengine.ext import deferred
from google.appengine.ext import ndb

from dashboard import add_histograms
from dashboard.common import datastore_hooks
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import job_state

from tracing.value import histogram as histogram_module
from tracing.value import histogram_set
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import reserved_infos

_MAX_JOBS_TO_FETCH = 100


def UpdateDashboardStatsGet():
  """A simple request handler to refresh the cached test suites info."""
  datastore_hooks.SetPrivilegedRequest()
  deferred.defer(_ProcessAlerts)
  deferred.defer(_ProcessPinpointStats)
  deferred.defer(_ProcessPinpointJobs)
  return make_response('Dashboard stats updated', 200)


def _FetchCompletedPinpointJobs(start_date):
  query = job_module.Job.query().order(-job_module.Job.created)
  jobs, next_cursor, more = query.fetch_page(_MAX_JOBS_TO_FETCH)

  def _IsValidJob(job):
    if job.status != 'Completed':
      return False
    if not job.bug_id:
      return False
    if job.comparison_mode != job_state.PERFORMANCE:
      return False
    diffs = len(list(job.state.Differences()))
    if diffs != 1:
      return False
    return True

  jobs_in_range = [j for j in jobs if j.created > start_date]
  valid_jobs = [j for j in jobs_in_range if _IsValidJob(j)]
  total_jobs = []
  total_jobs.extend(valid_jobs)

  while jobs_in_range and more:
    jobs, next_cursor, more = query.fetch_page(
        _MAX_JOBS_TO_FETCH, start_cursor=next_cursor)
    jobs_in_range = [j for j in jobs if j.created > start_date]
    valid_jobs = [j for j in jobs_in_range if _IsValidJob(j)]
    total_jobs.extend(valid_jobs)

  total_jobs = [(j, _GetDiffCommitTimeFromJob(j)) for j in total_jobs]
  total_jobs = [(j, c) for j, c in total_jobs if c]

  return total_jobs


def _CreateHistogramSet(master, bot, benchmark, commit_position, histograms):
  histograms = histogram_set.HistogramSet(histograms)
  histograms.AddSharedDiagnosticToAllHistograms(
      reserved_infos.MASTERS.name, generic_set.GenericSet([master]))
  histograms.AddSharedDiagnosticToAllHistograms(reserved_infos.BOTS.name,
                                                generic_set.GenericSet([bot]))
  histograms.AddSharedDiagnosticToAllHistograms(
      reserved_infos.CHROMIUM_COMMIT_POSITIONS.name,
      generic_set.GenericSet([commit_position]))
  histograms.AddSharedDiagnosticToAllHistograms(
      reserved_infos.BENCHMARKS.name, generic_set.GenericSet([benchmark]))

  return histograms


def _CreateHistogram(name, unit, story=None, summary_options=None):
  h = histogram_module.Histogram(name, unit)
  if summary_options:
    h.CustomizeSummaryOptions(summary_options)

  if story:
    h.diagnostics[reserved_infos.STORIES.name] = (
        generic_set.GenericSet([story]))
  return h


def _GetDiffCommitTimeFromJob(job):
  diffs = job.state.Differences()
  try:
    for d in diffs:
      diff = d[1].AsDict()
      commit_time = datetime.datetime.strptime(diff['commits'][0]['created'],
                                               '%Y-%m-%dT%H:%M:%S')
      return commit_time
  except http_client.HTTPException:
    return None


@ndb.tasklet
def _FetchStatsForJob(job, commit_time):
  culprit_time = job.updated
  create_time = job.created

  # Alert time, we'll approximate this out by querying for all alerts for this
  # bug and taking the earliest.
  query = anomaly.Anomaly.query()
  query = query.filter(anomaly.Anomaly.bug_id == job.bug_id)
  alerts = yield query.fetch_async(limit=1000)
  if not alerts:
    raise ndb.Return(None)

  alert_time = min([a.timestamp for a in alerts])
  if alert_time < commit_time:
    raise ndb.Return(None)

  time_from_job_to_culprit = (culprit_time -
                              create_time).total_seconds() * 1000.0
  time_from_commit_to_alert = (alert_time -
                               commit_time).total_seconds() * 1000.0
  time_from_alert_to_job = (create_time - alert_time).total_seconds() * 1000.0
  time_from_commit_to_culprit = (culprit_time -
                                 commit_time).total_seconds() * 1000.0

  raise ndb.Return((time_from_commit_to_culprit, time_from_commit_to_alert,
                    time_from_alert_to_job, time_from_job_to_culprit))


@ndb.tasklet
def _FetchPerformancePinpointJobs(start_date, end_date):
  query = job_module.Job.query().order(-job_module.Job.updated)
  jobs, next_cursor, more = yield query.fetch_page_async(_MAX_JOBS_TO_FETCH)

  def _IsValidJob(job):
    if not job.completed:
      return False
    if job.comparison_mode != job_state.PERFORMANCE:
      return False
    return job.updated > start_date and job.updated <= end_date

  oldest_job = None
  if jobs:
    oldest_job = jobs[-1].updated
  total_jobs = [j for j in jobs if _IsValidJob(j)]

  # We'll search back up to a week for a job that ended between the range
  # specified.
  while (oldest_job and oldest_job >= start_date) and more:
    jobs, next_cursor, more = yield query.fetch_page_async(
        _MAX_JOBS_TO_FETCH, start_cursor=next_cursor)

    total_jobs.extend([j for j in jobs if _IsValidJob(j)])

    oldest_job = None
    if jobs:
      oldest_job = jobs[-1].updated

  raise ndb.Return(total_jobs)


@ndb.synctasklet
def _ProcessPinpointStats(offset=0):
  end_date = datetime.datetime.now() - datetime.timedelta(days=offset)
  start_date = end_date - datetime.timedelta(days=7)
  commit_pos = int(time.mktime(end_date.timetuple()))

  completed_jobs = yield _FetchPerformancePinpointJobs(start_date, end_date)

  jobs_by_bot = collections.defaultdict(
      lambda: collections.defaultdict(lambda: {
          'pass': 0,
          'fail': 0,
          'norepro': 0,
          'total': 0
      }))

  for j in completed_jobs:
    bot = j.arguments.get('configuration')
    benchmark = j.arguments.get('benchmark')

    if j.failed:
      jobs_by_bot[bot][benchmark]['fail'] += 1
    else:
      if j.difference_count == 0:
        jobs_by_bot[bot][benchmark]['norepro'] += 1
      else:
        jobs_by_bot[bot][benchmark]['pass'] += 1
    jobs_by_bot[bot][benchmark]['total'] += 1

  default_opts = {
      'avg': True,
      'std': False,
      'count': False,
      'max': False,
      'min': False,
      'sum': True
  }

  avg_opts = {
      'avg': True,
      'std': False,
      'count': False,
      'max': False,
      'min': False,
      'sum': False
  }

  def _UnitType(k):
    unit = 'count_biggerIsBetter'
    if k in ['fail', 'norepro']:
      unit = 'count_smallerIsBetter'
    return unit

  data_by_benchmark = collections.defaultdict(lambda: {
      'pass': 0,
      'fail': 0,
      'norepro': 0,
      'total': 0
  })

  for bot, benchmark_dict in jobs_by_bot.items():
    hists = []

    summaries = {'total': 0, 'norepro': 0, 'fail': 0, 'pass': 0}

    for benchmark, values in benchmark_dict.items():
      for k, v in values.items():
        h = _CreateHistogram(
            k, _UnitType(k), story=benchmark, summary_options=default_opts)
        h.AddSample(v)
        hists.append(h)
        summaries[k] += v
        data_by_benchmark[benchmark][k] += v

    for k, v in summaries.items():
      h = _CreateHistogram(k, _UnitType(k), summary_options=default_opts)
      h.AddSample(v)
      hists.append(h)

      if summaries['total'] > 0 and k != 'total':
        h = _CreateHistogram(
            k + '.normalized', _UnitType(k), summary_options=avg_opts)
        h.AddSample(v / float(summaries['total']))
        hists.append(h)

    hs = _CreateHistogramSet('ChromiumPerfFyi', bot, 'pinpoint.success',
                             commit_pos, hists)
    deferred.defer(add_histograms.ProcessHistogramSet, hs.AsDicts())

  # Create one "uber" bot to contain all benchmarks results, just for ease of
  # use.
  summaries = {'total': 0, 'norepro': 0, 'fail': 0, 'pass': 0}
  hists = []

  for benchmark, values in data_by_benchmark.items():
    for k, v in values.items():
      h = _CreateHistogram(
          k, _UnitType(k), story=benchmark, summary_options=default_opts)
      h.AddSample(v)
      hists.append(h)
      summaries[k] += v

  for k, v in summaries.items():
    h = _CreateHistogram(k, _UnitType(k), summary_options=default_opts)
    h.AddSample(v)
    hists.append(h)

    if summaries['total'] > 0 and k != 'total':
      h = _CreateHistogram(
          k + '.normalized', _UnitType(k), summary_options=avg_opts)
      h.AddSample(v / float(summaries['total']))
      hists.append(h)

  hs = _CreateHistogramSet('ChromiumPerfFyi.all', 'all', 'pinpoint.success',
                           commit_pos, hists)
  deferred.defer(add_histograms.ProcessHistogramSet, hs.AsDicts())


@ndb.synctasklet
def _ProcessAlerts():
  ts_start = datetime.datetime.now() - datetime.timedelta(days=1)

  alerts, _, _ = yield anomaly.Anomaly.QueryAsync(
      min_timestamp=ts_start,
      subscriptions=['Chromium Perf Sheriff'],
  )
  if not alerts:
    raise ndb.Return()

  alerts_by_bot = collections.defaultdict(list)
  for a in alerts:
    alerts_by_bot[a.bot_name].append(a)

  for bot_name, bot_alerts in alerts_by_bot.items():
    yield _ProcessAlertsForBot(bot_name, bot_alerts)


@ndb.tasklet
def _ProcessAlertsForBot(bot_name, alerts):
  alerts_total = _CreateHistogram('chromium.perf.alerts', 'count')
  alerts_total.AddSample(len(alerts))

  count_by_suite = {}

  for a in alerts:
    test_suite_name = utils.TestSuiteName(a.test)
    if test_suite_name not in count_by_suite:
      count_by_suite[test_suite_name] = 0
    count_by_suite[test_suite_name] += 1

  hists_by_suite = {}
  for s, c in count_by_suite.items():
    hists_by_suite[s] = _CreateHistogram(
        'chromium.perf.alerts', 'count', story=s)
    hists_by_suite[s].AddSample(c)

  hs = _CreateHistogramSet('ChromiumPerfFyi', bot_name, 'chromeperf.stats',
                           int(time.time()),
                           [alerts_total] + list(hists_by_suite.values()))

  deferred.defer(add_histograms.ProcessHistogramSet, hs.AsDicts())


@ndb.synctasklet
def _ProcessPinpointJobs():
  jobs_and_commits = yield _FetchCompletedPinpointJobs(datetime.datetime.now() -
                                                       datetime.timedelta(
                                                           days=14))

  job_results = yield [_FetchStatsForJob(j, c) for j, c in jobs_and_commits]
  job_results = [j for j in job_results if j]
  if not job_results:
    raise ndb.Return(None)

  commit_to_culprit = _CreateHistogram('pinpoint', 'msBestFitFormat')
  commit_to_culprit.CustomizeSummaryOptions({'percentile': [0.5, 0.9]})

  commit_to_alert = _CreateHistogram(
      'pinpoint', 'msBestFitFormat', story='commitToAlert')
  alert_to_job = _CreateHistogram(
      'pinpoint', 'msBestFitFormat', story='alertToJob')
  job_to_culprit = _CreateHistogram(
      'pinpoint', 'msBestFitFormat', story='jobToCulprit')

  for result in job_results:
    time_from_land_to_culprit = result[0]
    time_from_commit_to_alert = result[1]
    time_from_alert_to_job = result[2]
    time_from_job_to_culprit = result[3]

    commit_to_alert.AddSample(time_from_commit_to_alert)
    alert_to_job.AddSample(time_from_alert_to_job)
    job_to_culprit.AddSample(time_from_job_to_culprit)
    commit_to_culprit.AddSample(time_from_land_to_culprit)

  hs = _CreateHistogramSet(
      'ChromiumPerfFyi', 'test1', 'chromeperf.stats', int(time.time()),
      [commit_to_alert, alert_to_job, job_to_culprit, commit_to_culprit])

  deferred.defer(add_histograms.ProcessHistogramSet, hs.AsDicts())
