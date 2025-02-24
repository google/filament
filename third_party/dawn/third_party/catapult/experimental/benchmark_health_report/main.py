# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A script to generate the benchmark health report.

Getting set up:
1. Install the python dependencies:
`$ pip install -r experimental/benchmark_health_report/requirements.txt`

2. Download the service account credentials and update _PATH_TO_JSON_KEYFILE
   in both drive_api.py and dashboard_api.py (can use different credentials)
   Ask sullivan@ to share credentials.
"""

from __future__ import absolute_import
from __future__ import print_function
import argparse
import datetime
# Need to have dateutil module installed locally.
# pylint:disable=import-error
import dateutil.parser
import os
import sys

sys.path.insert(
    0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from benchmark_health_report import drive_api
from benchmark_health_report import load_benchmark_data
from benchmark_health_report import sheet_writer

def _UtcDatetimeFromDateStr(yyyymmdd):
  year, month, day = yyyymmdd.split('-')
  d = datetime.datetime(int(year), int(month), int(day))
  return dateutil.parser.parse(d.isoformat() + 'Z')  # Append 'Z' for UTC

def _AddAndParseArgs():
  parser = argparse.ArgumentParser(description='Generate health report.')
  parser.add_argument(
      '--start-date', dest='start_date', required=True,
      help='Start date for report in YYYY-MM-DD format')
  parser.add_argument(
      '--end-date', dest='end_date',
      help='End date for report in YYYY-MM-DD format')
  parser.add_argument(
      '--benchmark', dest='benchmark', help='Limit to the given benchmark')
  parser.add_argument(
      '--include-timeseries', dest='include_timeseries', action='store_true',
      help='Only available with --benchmark, include timeseries spreadsheets. '
           'Unfortunately, this option does not work for long timeseries names')
  args = parser.parse_args()
  if args.include_timeseries:
    assert args.benchmark, 'Timeseries only available for a single benchmark'
  start_date = _UtcDatetimeFromDateStr(args.start_date)
  if args.end_date:
    end_date = _UtcDatetimeFromDateStr(args.end_date)
  else:
    end_date = datetime.datetime.today()
  return (start_date, end_date, args.benchmark, args.include_timeseries)


def main():
  start_date, end_date, benchmark, include_timeseries = _AddAndParseArgs()
  benchmarks, bugs = load_benchmark_data.LoadBenchmarkData(
      start_date, end_date, benchmark)
  folder_id = sheet_writer.WriteSummarySpreadsheets(
      benchmarks, bugs, start_date, end_date)
  if include_timeseries:
    paths = load_benchmark_data.GetTimeseriesList(benchmark)
    for path in paths:
      print(drive_api.CreateSpreadsheet(path, [{
          'name': path,
          'values': load_benchmark_data.GetTimeseries(
              path, start_date, end_date),
      }], folder_id))


if __name__ == '__main__':
  main()
