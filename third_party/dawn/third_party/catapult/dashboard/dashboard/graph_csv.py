# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Downloads a single time series as CSV."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import csv
import logging
import six

from dashboard.common import datastore_hooks
from dashboard.common import request_handler
from dashboard.common import utils
from dashboard.models import graph_data


# Request handler for getting data from one series as CSV.

from flask import request, make_response


def GraphCSVGet():
  """Gets CSV from data store and outputs it.

  Request parameters:
    test_path: Full test path of one trace.
    rev: End revision number; if not given, latest revision is used.
    num_points: Number of Rows to get data for.
    attr: Comma-separated list of attributes (columns) to return.

  Outputs:
    CSV file contents.
  """
  test_path = request.values.get('test_path')
  rev = request.values.get('rev')
  num_points = int(request.values.get('num_points', 500))
  attributes = request.values.get('attr', 'revision,value').split(',')

  if not test_path:
    return request_handler.RequestHandlerReportError(
        'No test path given.', status=400)

  logging.info('Got request to /graph_csv for test: "%s".', test_path)

  test_key = utils.TestKey(test_path)
  test = test_key.get()
  assert (datastore_hooks.IsUnalteredQueryPermitted() or not test.internal_only)
  datastore_hooks.SetSinglePrivilegedRequest()
  q = graph_data.Row.query()
  q = q.filter(graph_data.Row.parent_test == utils.OldStyleTestKey(test_key))
  if rev:
    q = q.filter(graph_data.Row.revision <= int(rev))
  q = q.order(-graph_data.Row.revision)  # pylint: disable=invalid-unary-operand-type
  points = reversed(q.fetch(limit=num_points))

  rows = _GraphCSVGenerateRows(points, attributes)

  output = six.StringIO()
  csv.writer(output).writerows(rows)
  res = make_response(output.getvalue())
  res.headers['Content-Type'] = 'text/csv'
  res.headers['Content-Disposition'] = ('attachment; filename=%s.csv' %
                                        test.test_name)
  return res


def GraphCSVPost():
  """A post request is the same as a get request for this endpoint."""
  return GraphCSVGet()


def _GraphCSVGenerateRows(points, attributes):
  """Generates CSV rows based on the attributes given.

  Args:
    points: A list of Row entities.
    attributes: A list of properties of Row entities to get.

  Returns:
    A list of lists of attribute values for the given points.
  """
  rows = [attributes]
  for point in points:
    row = []
    for attr in attributes:
      row.append(getattr(point, attr, ''))
    rows.append(row)
  return rows
