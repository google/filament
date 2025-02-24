# Copyright (c) 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A ReadFromDatastore alternative for fetching entities in a timestamp range.

The main difference from ReadFromDatastore is the query splitting method (for
splitting into parallel fetches).  ReadFromDatastore attempts to automatically
split the query by interrogating the datastore about how best to split the
query.  This strategy fails in many cases, e.g. for huge datasets this initial
query can be prohibitively expensive, and if there's an inequality filter no
splitting is attempted at all.

ReadTimestampRangeFromDatastore simply splits by a timestamp property, which
works well for datasets that are roughly evenly distributed across time (e.g.
most days have roughly the same number of entities).  This only requires a
trivial amount of upfront work, so tends to outperform ReadFromDatastore for
chromeperf's use cases.
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import datetime
import logging

import apache_beam as beam
from google.cloud.datastore import client as ds_client
from google.cloud.datastore import query as ds_query


class ReadTimestampRangeFromDatastore(beam.PTransform):
  """Similar to ReadFromDatastore; see module docstring."""

  def __init__(self,
               query_params,
               time_range_provider,
               step=datetime.timedelta(days=1),
               timestamp_property='timestamp'):
    """Constructor.

    Params:
      :query_params: kwargs for google.cloud.datastore.query.Query's
          constructor.
      :time_range: see BqExportOptions.GetTimeRangeProvider.
      :step: a datetime.timedelta of the interval to split the range to fetch by
          (default is 1 day).
      :timestamp_property: a str of the name of the timestamp property to filter
          on.
    """
    super().__init__()
    self._query_params = query_params
    self._time_range_provider = time_range_provider
    self._step = step
    self._timestamp_property = timestamp_property

  def expand(self, pcoll):  # pylint: disable=invalid-name
    return (
        pcoll.pipeline
        | 'Init' >> beam.Create([None])
        | 'SplitTimeRange' >> beam.FlatMap(lambda _: list(self._Splits()))
        # Reshuffle is necessary to prevent fusing between SplitTimeRange
        # and ReadRows, which would thwart parallel processing of ReadRows!
        | 'Reshuffle' >> beam.Reshuffle()
        | 'ReadRows' >> beam.ParDo(
            ReadTimestampRangeFromDatastore._QueryFn(self._query_params,
                                                     self._timestamp_property)))

  class _QueryFn(beam.DoFn):

    def __init__(self, query_params, timestamp_property):
      super().__init__()
      self._query_params = query_params
      self._timestamp_property = timestamp_property

    def process(
        self,
        start_end,
        *unused_args,  # pylint: disable=invalid-name
        **unused_kwargs):
      start, end = start_end
      client = ds_client.Client(project=self._query_params['project'])
      query = ds_query.Query(client=client, **self._query_params)
      query.add_filter(self._timestamp_property, '>=', start)
      query.add_filter(self._timestamp_property, '<', end)
      for entity in query.fetch(client=client, eventual=True):
        yield entity

  def _Splits(self):
    min_timestamp, max_timestamp = self._time_range_provider.Get()
    logging.getLogger().info('ReadTimestampRangeFromDatastore from %s to %s',
                             min_timestamp, max_timestamp)
    start = min_timestamp
    while True:
      end = start + self._step
      yield (start, end)

      if end >= max_timestamp:
        break
      start = end
