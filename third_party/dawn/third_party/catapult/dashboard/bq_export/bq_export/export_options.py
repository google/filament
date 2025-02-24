# Copyright (c) 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import datetime
import pytz

from apache_beam.options.pipeline_options import PipelineOptions


def _YesterdayUTC():
  return (datetime.datetime.utcnow() -
          datetime.timedelta(days=1)).strftime('%Y%m%d')


class BqExportOptions(PipelineOptions):

  @classmethod
  def _add_argparse_args(cls, parser):  # pylint: disable=invalid-name
    parser.add_value_provider_argument(
        '--end_date',
        help=('Last day of data to export in YYYYMMDD format, or special value '
              '"yesterday".  Default is yesterday.  Timezone is always UTC.'),
        default="yesterday")
    parser.add_value_provider_argument(
        '--num_days', help='Number of days data to export', type=int, default=1)
    parser.add_argument(
        '--table_suffix',
        help='Suffix to add to table name (for dev purposes, e.g. "_test").',
        default='')
    parser.add_value_provider_argument(
        '--dataset',
        help='BigQuery dataset name.  Overrideable for testing/dev purposes.',
        default='chromeperf_dashboard_data')

  def GetTimeRangeProvider(self):
    """Return an object with .Get() method that returns (start, end) tuple.

    In other words, returns the time range specified by --end_date and
    --num_days as a pair of datetime.datetime objects.
    """
    return _TimeRangeProvider(self.end_date, self.num_days)


class _TimeRangeProvider:
  """A ValueProvider-like based on the end_date and num_days ValueProviders.

  This class is a workaround for the lack of NestedValueProviders in Beam's
  Python SDK.
  """

  def __init__(self, end_date, num_days):
    self._end_date = end_date
    self._num_days = num_days

  def Get(self):
    return (self._StartTime(), self._EndTime())

  def __str__(self):
    return '_TimeRangeProvider({}, {})'.format(self._end_date, self._num_days)

  def _EndAsDatetime(self):
    # pylint: disable=access-member-before-definition
    end_date = self._end_date.get()
    if end_date == 'yesterday':
      end_date = _YesterdayUTC()
    return datetime.datetime.strptime(end_date,
                                      '%Y%m%d').replace(tzinfo=pytz.UTC)

  def _StartTime(self):
    # pylint: disable=access-member-before-definition
    return self._EndTime() - datetime.timedelta(days=self._num_days.get())

  def _EndTime(self):
    # We want to include all the timestamps during the given day, so return a
    # timestamp at midnight of the _following_ day.
    return self._EndAsDatetime() + datetime.timedelta(days=1)
