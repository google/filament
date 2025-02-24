# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import datetime
# Need to have dateutil module installed locally.
# pylint:disable=import-error
import dateutil.parser
import dateutil.tz

from benchmark_health_report import dashboard_api
from benchmark_health_report import drive_api
import six

def LoadBenchmarkData(
    start_date, end_date, single_benchmark):
  """Pulls benchmark data from dashboard API and sheets, and summarizes."""
  today = _GetTodayUtc()
  num_days = (today - start_date).days + 1
  alerts = dashboard_api.GetAlerts(num_days, single_benchmark)
  benchmarks = {}
  bugs = {}
  owners = _GetOwners()
  builds = _GetBuilds()
  for alert in alerts:
    utc_timestamp = _GetUtcDatetimeFromIsoTimestamp(alert['timestamp'])
    if utc_timestamp > end_date or utc_timestamp < start_date:
      continue
    benchmark_name = alert['testsuite']
    if single_benchmark and benchmark_name != single_benchmark:
      continue
    benchmark = benchmarks.setdefault(
        benchmark_name, _GetDefaultBenchmark(benchmark_name, owners, builds))
    _AddAlertDataToBenchmark(benchmark, alert, bugs)
  for bug_id in bugs:
    bugs[bug_id] = dashboard_api.GetBug(bug_id)
  for _, benchmark in six.iteritems(benchmarks):
    for bug_id in benchmark['bugs']:
      bug = bugs[bug_id]
      _AddBugDataToBenchmark(benchmark, bug)
  return benchmarks, bugs

def _GetOwners():
  """Reads the owners spreadsheet and returns a map of benchmark:owner."""
  owners = {}
  owner_values = drive_api.ReadSpreadsheet(
      '1xaAo0_SU3iDfGdqDJZX_jRV0QtkufwHUKH3kQKF3YQs', 'A4:B')
  for row in owner_values:
    if len(row) < 2:
      continue
    owners[row[0]] = row[1]
  return owners

def _GetBuilds():
  """Reads the build failure spreadsheet and returns a map of benchmark:data.

  TODO(sullivan): replace with flakiness dashboard data when available.
  """
  build_values = drive_api.ReadSpreadsheet(
      '1FG217E4V_E7Wxn1_mlMaqRAPIQyebzcGIjv4SPxddzQ', 'A2:D')
  builds = {}
  for row in build_values:
    name = row[0]
    successful = float(row[1])
    failed = float(row[2])
    builds[name] = {
        'successful': successful,
        'failed': failed,
        'total': successful + failed,
        'percent_failed': round((failed / (successful + failed)) * 100)
    }
  return builds

def _GetDefaultBenchmark(benchmark_name, owners, builds):
  return {
      'name': benchmark_name,
      'total_alerts': 0,
      'untriaged_alerts': 0,
      'invalid_alerts': 0,
      'valid_alerts': 0,
      'total_bugs': 0,
      'open_bugs': 0,
      'closed_bugs': 0,
      'fixed_bugs': 0,
      'wontfix_bugs': 0,
      'duplicate_bugs': 0,
      'total_bisects': 0,
      'successful_bisects': 0,
      'no_repro_bisects': 0,
      'infra_failure_bisects': 0,
      'bugs': set([]),
      'alerts': [],
      'owner': owners.get(benchmark_name),
      'percent_failed_builds': builds.get(
          benchmark_name, {}).get('percent_failed'),
      'total_builds': builds.get(benchmark_name, {}).get('total'),
  }

def _AddAlertDataToBenchmark(benchmark, alert, bugs):
  benchmark['alerts'].append(alert)
  if not alert['improvement']:
    benchmark['total_alerts'] += 1
    bug_id = alert['bug_id']
    if bug_id is None:
      benchmark['untriaged_alerts'] += 1
    elif bug_id < 0:
      benchmark['invalid_alerts'] += 1
    else:
      benchmark['valid_alerts'] += 1
      benchmark['bugs'].add(bug_id)
      bugs[bug_id] = {}

def _AddBugDataToBenchmark(benchmark, bug):
  benchmark['total_bugs'] += 1
  if bug['state'] == 'open':
    benchmark['open_bugs'] += 1
  else:
    benchmark['closed_bugs'] += 1
    if bug['status'] == 'Fixed' or bug['status'] == 'Verified':
      benchmark['fixed_bugs'] += 1
    elif bug['status'] == 'WontFix' or bug['status'] == 'Archived':
      benchmark['wontfix_bugs'] += 1
    elif bug['status'] == 'Duplicate':
      # TODO(sullivan): report the duplicate bug status
      benchmark['duplicate_bugs'] += 1
  for bisect in bug['legacy_bisects']:
    benchmark['total_bisects'] += 1
    if bisect['status'] == 'success':
      benchmark['successful_bisects'] += 1
    elif bisect['status'] == 'no-repro':
      benchmark['no_repro_bisects'] += 1
    elif bisect['status'] == 'failed':
      benchmark['infra_failure_bisects'] += 1

def GetTimeseriesList(benchmark):
  return dashboard_api.GetTimeseriesList(benchmark)

def _GetUtcDatetimeFromIsoTimestamp(timestamp_str):
  if not timestamp_str.endswith('Z'):
    # Dashboard keeps all dates in UTC, and always writes ISO timestamps, but
    # it does not append the 'Z'. Since we want the datetime to be UTC and
    # timezone-aware, just append the 'Z'.
    timestamp_str += 'Z'
  return dateutil.parser.parse(timestamp_str)

def _GetTodayUtc():
  return datetime.datetime.utcnow().replace(tzinfo=dateutil.tz.tzutc())

def GetTimeseries(name, start_date, end_date):
  num_days = (_GetTodayUtc() - start_date).days + 1
  full_timeseries = dashboard_api.GetTimeseries(name, num_days)['timeseries']
  filtered_timeseries = [full_timeseries[0]]
  for row in full_timeseries[1:]:
    row_datetime = _GetUtcDatetimeFromIsoTimestamp(row[2])
    if row_datetime < start_date or row_datetime > end_date:
      continue
    filtered_timeseries.append(row)
  return filtered_timeseries
