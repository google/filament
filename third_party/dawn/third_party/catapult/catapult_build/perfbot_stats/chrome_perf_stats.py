#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Script to pull chromium.perf stats from chrome-infra-stats API.

Currently this just pulls success rates from the API, averages daily per
builder, and uploads to perf dashboard. It could be improved to provide more
detailed success rates.

The API documentation for chrome-infra-stats is at:
https://apis-explorer.appspot.com/apis-explorer/?
   base=https://chrome-infra-stats.appspot.com/_ah/api#p/
"""

from __future__ import absolute_import
from __future__ import print_function
import calendar
import datetime
import json
import sys
import six.moves.urllib.request # pylint: disable=import-error
import six.moves.urllib.parse # pylint: disable=import-error
import six.moves.urllib.error # pylint: disable=import-error
from six.moves import range

BUILDER_LIST_URL = ('https://chrome-infra-stats.appspot.com/'
                    '_ah/api/stats/v1/masters/chromium.perf')

BUILDER_STATS_URL = ('https://chrome-infra-stats.appspot.com/_ah/api/stats/v1/'
                     'stats/chromium.perf/%s/overall__build__result__/%s')

USAGE = ('Usage: chrome_perf_stats.py <year> <month> <day>. If date is not '
         'specified, yesterday will be used.')


def main():
  if len(sys.argv) == 2 and sys.argv[0] == '--help':
    print(USAGE)
    sys.exit(0)
  year = None
  month = None
  days = None
  if len(sys.argv) == 4 or len(sys.argv) == 3:
    year = int(sys.argv[1])
    if year > 2016 or year < 2014:
      print(USAGE)
      sys.exit(0)
    month = int(sys.argv[2])
    if month > 12 or month <= 0:
      print(USAGE)
      sys.exit(0)
    if len(sys.argv) == 3:
      days = list(range(1, calendar.monthrange(year, month)[1] + 1))
    else:
      day = int(sys.argv[3])
      if day > 31 or day <= 0:
        print(USAGE)
        sys.exit(0)
      days = [day]
  elif len(sys.argv) != 1:
    print(USAGE)
    sys.exit(0)
  else:
    yesterday = datetime.date.today() - datetime.timedelta(days=1)
    year = yesterday.year
    month = yesterday.month
    days = [yesterday.day]

  response = six.moves.urllib.request.urlopen(BUILDER_LIST_URL)
  builders = [builder['name'] for builder in json.load(response)['builders']]
  success_rates = CalculateSuccessRates(year, month, days, builders)
  UploadToPerfDashboard(success_rates)


def _UpdateSuccessRatesWithResult(
    success_rates, results, date_dict_str, builder):
  count = int(results['count'])
  if count == 0:
    return
  success_count = count - int(results['failure_count'])
  success_rates.setdefault(date_dict_str, {})
  success_rates[date_dict_str].setdefault(builder, {
      'count': 0,
      'success_count': 0
  })
  success_rates[date_dict_str][builder]['count'] += count
  success_rates[date_dict_str][builder]['success_count'] += success_count


def _SummarizeSuccessRates(success_rates):
  overall_success_rates = []
  for day, results in success_rates.items():
    success_rate_sum = 0
    success_rate_count = 0
    for rates in results.values():
      if rates['count'] == 0:
        continue
      success_rate_sum += (
          float(rates['success_count']) / float(rates['count']))
      success_rate_count += 1
    overall_success_rates.append(
        [day, float(success_rate_sum) / float(success_rate_count)])
  return overall_success_rates


def UploadToPerfDashboard(success_rates):
  for success_rate in success_rates:
    date_str = '%s-%s-%s' % (success_rate[0][0:4],
                             success_rate[0][4:6],
                             success_rate[0][6:8])
    dashboard_data = {
        'master': 'WaterfallStats',
        'bot': 'ChromiumPerf',
        'point_id': int(success_rate[0]),
        'supplemental': {},
        'versions': {
            'date': date_str,
        },
        'chart_data': {
            'benchmark_name': 'success_rate',
            'benchmark_description': 'Success rates averaged per-builder',
            'format_version': 1.0,
            'charts': {
                'overall_success_rate': {
                    'summary': {
                        'name': 'overall_success_rate',
                        'type': 'scalar',
                        'units': '%',
                        'value': success_rate[1]
                    }
                }
            }
        }
    }
    url = 'https://chromeperf.appspot.com/add_point'
    data = six.moves.urllib.parse.urlencode(
        {'data': json.dumps(dashboard_data)})
    six.moves.urllib.request.urlopen(url, data).read()


def CalculateSuccessRates(year, month, days, builders):
  success_rates = {}
  for day in days:
    for hour in range(24):
      date_str = '%d-%02d-%02dT%02d:00Z' % (year, month, day, hour)
      date_dict_str = '%d%02d%02d' % (year, month, day)
      for builder in builders:
        url = BUILDER_STATS_URL % (
            six.moves.urllib.parse.quote(builder),
            six.moves.urllib.parse.quote(date_str))
        response = six.moves.urllib.request.urlopen(url)
        results = json.load(response)
        _UpdateSuccessRatesWithResult(
            success_rates, results, date_dict_str, builder)
  return _SummarizeSuccessRates(success_rates)


if __name__ == "__main__":
  main()
