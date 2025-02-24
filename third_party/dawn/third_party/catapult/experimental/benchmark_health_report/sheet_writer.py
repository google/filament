# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from benchmark_health_report import drive_api
import six

_SUMMARY_HEADER = [
    'Benchmark',
    'Owner',
    'Total Alerts',
    'Valid Alerts',
    'Invalid Alerts',
    'Untriaged Alerts',
    'Total Bugs',
    'Open Bugs',
    'Closed Bugs',
    'Fixed Bugs',
    'WontFix Bugs',
    'Duplicate Bugs',
    'Total Bisects',
    'Successful Bisects',
    'No Repro Bisects',
    'Bisect Failures',
    'Total Builds',
    '% Failed Builds',
]


def WriteSummarySpreadsheets(
    benchmarks, bugs, start_date, end_date):
  date_range = '%s - %s' % (
      start_date.strftime('%Y-%m-%d'), end_date.strftime('%Y-%m-%d'))
  folder_id = drive_api.CreateFolder('Benchmark Health Reports: %s' % date_range)
  summary_sheet = [_SUMMARY_HEADER]

  for name, benchmark in six.iteritems(benchmarks):
    title = 'Benchmark Health Report %s' % name
    url = drive_api.CreateSpreadsheet(title, [{
        'name': 'Summary',
        'values': _GetOverviewValues(benchmark),
    }, {
        'name': 'Alerts',
        'values': _GetAlertValues(benchmark),
    }, {
        'name': 'Bugs',
        'values': _GetBugValues(benchmark, bugs),
    }, {
        'name': 'Bisects',
        'values': _GetBisectValues(benchmark, bugs),
    }], folder_id)
    summary_sheet.append(_GetSummaryValues(benchmark, url))
  url = drive_api.CreateSpreadsheet(
      'Benchmark Health Report',
      [{'name': 'Overview', 'values': summary_sheet}],
      folder_id)
  return folder_id

def _GetOverviewValues(benchmark):
  return [
      ['Owner', benchmark['owner']],
      ['Alerts'],
      ['Total alerts', benchmark['total_alerts']],
      ['Valid alerts', benchmark['valid_alerts']],
      ['Invalid alerts', benchmark['invalid_alerts']],
      ['Untriaged alerts', benchmark['untriaged_alerts']],
      ['Bugs'],
      ['Total Bugs', benchmark['total_bugs']],
      ['Open Bugs', benchmark['open_bugs']],
      ['Closed Bugs', benchmark['closed_bugs']],
      ['Fixed Bugs', benchmark['fixed_bugs']],
      ['WontFix Bugs', benchmark['wontfix_bugs']],
      ['Duplicate Bugs', benchmark['duplicate_bugs']],
      ['Bisects'],
      ['Total Bisects', benchmark['total_bisects']],
      ['Successful Bisects', benchmark['successful_bisects']],
      ['No Repro Bisects', benchmark['no_repro_bisects']],
      ['Failed Bisects', benchmark['infra_failure_bisects']],
      ['Total Builds', benchmark['total_builds']],
      ['% Failed Builds', benchmark['percent_failed_builds']]
  ]

def _GetAlertValues(benchmark):
  alert_values = [[
      'Timestamp',
      'Bot',
      'Metric',
      '% Change',
      'Abs Change',
      'Units',
      'Improvement',
      'Bug',
      'Link'
  ]]
  alert_values += [[
      alert['timestamp'],
      alert['bot'],
      alert['test'],
      alert['percent_changed'],
      alert['absolute_delta'],
      alert['units'],
      'Yes' if alert['improvement'] else 'No',
      _GetBugLink(alert['bug_id']),
      _GetHyperlink(
          'Graphs',
          'https://chromeperf.appspot.com/group_report?keys=%s' % (
              alert['key']))
  ] for alert in benchmark['alerts']]
  return alert_values

def _GetBugValues(benchmark, bugs):
  bug_values = [[
      'Summary',
      'Published',
      'Components',
      'Labels',
      'State',
      'Status',
      'Link',
  ]]
  bug_values += [[
      bugs[bug_id]['summary'],
      bugs[bug_id]['published'],
      ','.join(bugs[bug_id]['components']),
      ','.join(bugs[bug_id]['labels']),
      bugs[bug_id]['state'],
      bugs[bug_id]['status'],
      _GetBugLink(bug_id),
  ] for bug_id in benchmark['bugs']]
  return bug_values

def _GetBisectValues(benchmark, bugs):
  bisect_values = [[
      'Status',
      'Bug',
      'Buildbucket Link',
      'Metric',
      'Started',
      'Culprit',
      'Link']]
  for bug_id in benchmark['bugs']:
    for bisect in bugs[bug_id]['legacy_bisects']:
      culprit = bisect.get('culprit')
      bisect_values.append([
          bisect['status'],
          _GetBugLink(bug_id),
          _GetHyperlink('link', bisect['buildbucket_link']),
          bisect.get('metric'),
          bisect.get('started_timestamp'),
          culprit['subject'] if culprit else '',
          _GetCulpritLink(culprit),
      ])
  return bisect_values

def _GetSummaryValues(benchmark, url):
  return [
      benchmark['name'],
      benchmark['owner'],
      benchmark['total_alerts'],
      benchmark['valid_alerts'],
      benchmark['invalid_alerts'],
      benchmark['untriaged_alerts'],
      benchmark['total_bugs'],
      benchmark['open_bugs'],
      benchmark['closed_bugs'],
      benchmark['fixed_bugs'],
      benchmark['wontfix_bugs'],
      benchmark['duplicate_bugs'],
      benchmark['total_bisects'],
      benchmark['successful_bisects'],
      benchmark['no_repro_bisects'],
      benchmark['infra_failure_bisects'],
      benchmark['total_builds'],
      benchmark['percent_failed_builds'],
      _GetHyperlink('Full Data', url),
  ]

def _GetHyperlink(title, url):
  return '=HYPERLINK("%s", "%s")' % (url, six.text_type(title).replace('"', '\''))

def _GetCulpritLink(culprit):
  if culprit:
    return _GetHyperlink(
        culprit['cl'],
        'https://chromium.googlesource.com/chromium/src/+/%s' % (culprit['cl']))
  return ''

def _GetBugLink(bug_id):
  if not bug_id:
    return ''
  if bug_id == -1:
    return 'Invalid'
  if bug_id == -2:
    return 'Ignored'
  return _GetHyperlink(bug_id, 'http://crbug.com/%s' % bug_id)

def _FormatYearMonthDay(dt):
  return dt.strftime('%Y-%m-%d')
