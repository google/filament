# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Handler to serve a simple time series for all points in a series.

This is used to show the revision slider for a chart; it includes data
for all past points, including those that are not recent. Each entry
in the returned list is a 3-item list: [revision, value, timestamp].
The revisions and values are used to plot a mini-chart, and the timestamps
are used to label this mini-chart with dates.

This list is cached, since querying all Row entities for a given test takes a
long time. This module also provides a function for updating the cache.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import bisect
import json
import math

from google.appengine.ext import ndb

from dashboard.common import cloud_metric
from dashboard.common import datastore_hooks
from dashboard.common import namespaced_stored_object
from dashboard.common import utils
from dashboard.models import graph_data

from flask import request, make_response

_CACHE_KEY = 'num_revisions_%s'


@cloud_metric.APIMetric("chromeperf", "/graph_revisions")
def GraphRevisionsPost():
  test_path = request.values.get('test_path')
  rows = namespaced_stored_object.Get(_CACHE_KEY % test_path)
  if not rows:
    rows = _UpdateCache(utils.TestKey(test_path))

  # TODO(simonhatch): Need to filter out NaN values.
  # https://github.com/catapult-project/catapult/issues/3474
  def _NaNtoNone(x):
    if math.isnan(x):
      return None
    return x

  rows = [(_NaNtoNone(r[0]), _NaNtoNone(r[1]), _NaNtoNone(r[2])) for r in rows
          ]
  return make_response(json.dumps(rows))


@ndb.synctasklet
def SetCache(test_path, rows):
  """Sets the saved graph revisions data for a test.

  Args:
    test_path: A test path string.
    rows: A list of [revision, value, timestamp] triplets.
  """
  yield SetCacheAsync(test_path, rows)


@ndb.tasklet
def SetCacheAsync(test_path, rows):
  # This first set generally only sets the internal-only cache.
  futures = [namespaced_stored_object.SetAsync(_CACHE_KEY % test_path, rows)]

  # If this is an internal_only query for externally available data,
  # set the cache for that too.
  if datastore_hooks.IsUnalteredQueryPermitted():
    test = utils.TestKey(test_path).get()
    if test and not test.internal_only:
      futures.append(
          namespaced_stored_object.SetExternalAsync(_CACHE_KEY % test_path,
                                                    rows))
  yield futures


@ndb.synctasklet
def DeleteCache(test_path):
  """Removes any saved data for the given path."""
  yield DeleteCacheAsync(test_path)


@ndb.tasklet
def DeleteCacheAsync(test_path):
  """Removes any saved data for the given path."""
  yield namespaced_stored_object.DeleteAsync(_CACHE_KEY % test_path)


def _UpdateCache(test_key):
  """Queries Rows for a test then updates the cache.

  Args:
    test_key: ndb.Key for a TestMetadata entity.

  Returns:
    The list of triplets that was just fetched and set in the cache.
  """
  test = test_key.get()
  if not test:
    return []
  assert utils.IsInternalUser() or not test.internal_only
  datastore_hooks.SetSinglePrivilegedRequest()

  # A projection query queries just for the values of particular properties;
  # this is faster than querying for whole entities.
  query = graph_data.Row.query(projection=['revision', 'timestamp', 'value'])
  query = query.filter(
      graph_data.Row.parent_test == utils.OldStyleTestKey(test_key))

  # Using a large batch_size speeds up queries with > 1000 Rows.
  rows = list(map(_MakeTriplet, query.iter(batch_size=1000)))
  # Note: Unit tests do not call datastore_hooks with the above query, but
  # it is called in production and with more recent SDK.
  datastore_hooks.CancelSinglePrivilegedRequest()
  SetCache(utils.TestPath(test_key), rows)
  return rows


def _MakeTriplet(row):
  """Makes a 3-item list of revision, value and timestamp for a Row."""
  timestamp = utils.TimestampMilliseconds(row.timestamp)
  return [row.revision, row.value, timestamp]


@ndb.synctasklet
def AddRowsToCache(row_entities):
  """Adds a list of rows to the cache, in revision order.

  Updates multiple cache entries for different tests.

  Args:
    row_entities: List of Row entities.
  """
  yield AddRowsToCacheAsync(row_entities)


@ndb.tasklet
def AddRowsToCacheAsync(row_entities):
  test_key_to_futures = {}
  for row in row_entities:
    test_key = row.parent_test
    if test_key in test_key_to_futures:
      continue

    test_path = utils.TestPath(test_key)
    test_key_to_futures[test_key] = namespaced_stored_object.GetAsync(
        _CACHE_KEY % test_path)

  yield list(test_key_to_futures.values())

  test_key_to_rows = {}
  for row in row_entities:
    test_key = row.parent_test
    if test_key in test_key_to_rows:
      graph_rows = test_key_to_rows[test_key]
    else:
      test_path = utils.TestPath(test_key)
      graph_rows_future = test_key_to_futures.get(test_key)
      graph_rows = graph_rows_future.get_result()
      if not graph_rows:
        # We only want to update caches for tests that people have looked at.
        continue
      test_key_to_rows[test_key] = graph_rows

    revisions = [r[0] for r in graph_rows]
    index = bisect.bisect_left(revisions, row.revision)
    if index < len(revisions) - 1:
      if revisions[index + 1] == row.revision:
        return  # Already in cache.
    graph_rows.insert(index, _MakeTriplet(row))

  futures = []
  for test_key in test_key_to_rows:
    graph_rows = test_key_to_rows[test_key]
    futures.append(SetCacheAsync(utils.TestPath(test_key), graph_rows))
  yield futures
