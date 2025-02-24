# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import six

from google.appengine.datastore import datastore_query

from dashboard import alerts
from dashboard.api import api_request_handler
from dashboard.api import utils
from dashboard.common import request_handler
from dashboard.common import utils as dashboard_utils
from dashboard.models import anomaly
from dashboard.models import report_template


from flask import request


def _CheckUser():
  pass


@api_request_handler.RequestHandlerDecoratorFactory(_CheckUser)
def AlertsPost():
  """Returns alert data in response to API requests.

    Possible list types:
      keys: A comma-separated list of urlsafe Anomaly keys.
      bug_id: A bug number on the Chromium issue tracker.
      rev: A revision number.

    Outputs:
      Alerts data; see README.md.
    """
  alert_list = None
  response = {}
  try:
    is_improvement = utils.ParseBool(request.values.get('is_improvement', None))
    recovered = utils.ParseBool(request.values.get('recovered', None))
    start_cursor = request.values.get('cursor', None)
    if start_cursor:
      start_cursor = datastore_query.Cursor(urlsafe=start_cursor)
    min_timestamp = utils.ParseISO8601(
        request.values.get('min_timestamp', None))
    max_timestamp = utils.ParseISO8601(
        request.values.get('max_timestamp', None))

    test_keys = []
    for template_id in request.values.getlist('report'):
      test_keys.extend(report_template.TestKeysForReportTemplate(template_id))

    try:
      sheriff = request.values.get('sheriff', None)
      alert_list, next_cursor, count = anomaly.Anomaly.QueryAsync(
          bot_name=request.values.get('bot', None),
          bug_id=request.values.get('bug_id', None),
          is_improvement=is_improvement,
          key=request.values.get('key', None),
          limit=int(request.values.get('limit', 100)),
          count_limit=int(request.values.get('count_limit', 0)),
          master_name=request.values.get('master', None),
          max_end_revision=request.values.get('max_end_revision', None),
          max_start_revision=request.values.get('max_start_revision', None),
          max_timestamp=max_timestamp,
          min_end_revision=request.values.get('min_end_revision', None),
          min_start_revision=request.values.get('min_start_revision', None),
          min_timestamp=min_timestamp,
          recovered=recovered,
          subscriptions=[sheriff] if sheriff else None,
          start_cursor=start_cursor,
          test=request.values.get('test', None),
          test_keys=test_keys,
          test_suite_name=request.values.get('test_suite', None)).get_result()
      response['count'] = count
    except AssertionError:
      alert_list, next_cursor = [], None
    if next_cursor:
      response['next_cursor'] = next_cursor.urlsafe()
  except AssertionError as e:
    # The only known assertion is in InternalOnlyModel._post_get_hook when a
    # non-internal user requests an internal-only entity.
    six.raise_from(api_request_handler.BadRequestError('Not found'), e)
  except request_handler.InvalidInputError as e:
    six.raise_from(api_request_handler.BadRequestError(str(e)), e)

  response['anomalies'] = alerts.AnomalyDicts(
      alert_list, utils.ParseBool(request.values.get('v2', None)))
  response = dashboard_utils.ConvertBytesBeforeJsonDumps(response)
  return response
