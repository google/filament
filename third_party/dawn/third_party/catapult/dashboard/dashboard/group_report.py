# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides the web interface for a set of alerts and their graphs."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
import logging
import six

from google.appengine.ext import ndb

from dashboard import alerts
from dashboard import chart_handler
from dashboard import update_test_suites
from dashboard.common import cloud_metric
from dashboard.common import request_handler
from dashboard.common import utils
from dashboard.models import alert_group
from dashboard.models import anomaly
from dashboard.models import page_state
from dashboard.services import perf_issue_service_client

from flask import request, make_response

# This is the max number of alerts to query at once. This is used in cases
# when we may want to query more many more alerts than actually get displayed.
_QUERY_LIMIT = 5000


def GroupReportGet():
  return request_handler.RequestHandlerRenderStaticHtml('group_report.html')


@cloud_metric.APIMetric("chromeperf", "/group_report")
def GroupReportPost():
  """Returns dynamic data for /group_report with some set of alerts.

    The set of alerts is determined by the sid, keys, bug ID, AlertGroup ID,
    or revision given.

    Request parameters:
      keys: A comma-separated list of urlsafe Anomaly keys (optional).
      bug_id: A bug number on the Chromium issue tracker (optional).
      project_id: A project ID in Monorail (optional).
      rev: A revision number (optional).
      sid: A hash of a group of keys from /short_uri (optional).
      group_id: An AlertGroup ID (optional).

    Outputs:
      JSON for the /group_report page XHR request.
    """
  bug_id = request.values.get('bug_id')
  project_id = request.values.get('project_id', 'chromium') or 'chromium'
  rev = request.values.get('rev')
  keys = request.values.get('keys')
  hash_code = request.values.get('sid')
  group_id = request.values.get('group_id')

  # sid takes precedence.
  if hash_code:
    state = ndb.Key(page_state.PageState, hash_code).get()
    if state:
      keys = json.loads(state.value)
  elif keys:
    keys = keys.split(',')

  try:
    alert_list = None
    if bug_id:
      try:
        alert_list, _, _ = anomaly.Anomaly.QueryAsync(
            bug_id=bug_id, project_id=project_id,
            limit=_QUERY_LIMIT).get_result()
      except ValueError as e:
        six.raise_from(
            request_handler.InvalidInputError('Invalid bug ID "%s:%s".' %
                                              (project_id, bug_id)), e)
    elif keys:
      alert_list = GetAlertsForKeys(keys)
    elif rev:
      alert_list = GetAlertsAroundRevision(rev)
    elif group_id:
      alert_list = GetAlertsForGroupID(group_id)
    else:
      raise request_handler.InvalidInputError('No anomalies specified.')

    alert_dicts = alerts.AnomalyDicts([
        a for a in alert_list
        if a.key.kind() == 'Anomaly' and a.source != 'skia'
    ])

    values = {
        'alert_list': alert_dicts,
        'test_suites': update_test_suites.FetchCachedTestSuites(),
    }
    if bug_id:
      values['bug_id'] = bug_id
      values['project_id'] = project_id
    if keys:
      values['selected_keys'] = keys
    chart_handler.GetDynamicVariables(values)

    return make_response(json.dumps(values))
  except request_handler.InvalidInputError as error:
    return make_response(json.dumps({'error': str(error)}))


def GetAlertsAroundRevision(rev):
  """Gets the alerts whose revision range includes the given revision.

  Args:
    rev: A revision number, as a string.

  Returns:
    list of anomaly.Anomaly
  """
  if not _IsInt(rev):
    raise request_handler.InvalidInputError('Invalid rev "%s".' % rev)
  rev = int(rev)

  # We can't make a query that has two inequality filters on two different
  # properties (start_revision and end_revision). Therefore we first query
  # Anomaly entities based on one of these, then filter the resulting list.
  anomaly_query = anomaly.Anomaly.query(anomaly.Anomaly.end_revision >= rev)
  anomaly_query = anomaly_query.order(anomaly.Anomaly.end_revision)
  anomalies = anomaly_query.fetch(limit=_QUERY_LIMIT)
  return [a for a in anomalies if a.start_revision <= rev]


def GetAlertsForKeys(keys):
  """Get alerts for |keys|.

  Query for anomalies with overlapping revision. The |keys|
  parameter for group_report is a comma-separated list of urlsafe strings
  for Keys for Anomaly entities. (Each key corresponds to an alert)

  Args:
    keys: Comma-separated list of urlsafe strings for Anomaly keys.

  Returns:
    list of anomaly.Anomaly
  """
  urlsafe_keys = keys

  try:
    keys = [ndb.Key(urlsafe=k) for k in urlsafe_keys]
  # Errors that can be thrown here include ProtocolBufferDecodeError
  # in google.net.proto.ProtocolBuffer. We want to catch any errors here
  # because they're almost certainly urlsafe key decoding errors.
  except Exception as e:  # pylint: disable=broad-except
    six.raise_from(
        request_handler.InvalidInputError('Invalid Anomaly key given.'), e)

  requested_anomalies = utils.GetMulti(keys)

  for i, anomaly_entity in enumerate(requested_anomalies):
    if anomaly_entity is None:
      raise request_handler.InvalidInputError('No Anomaly found for key %s.' %
                                              urlsafe_keys[i])

  if not requested_anomalies:
    raise request_handler.InvalidInputError('No anomalies found.')

  # Just an optimization because we can't fetch anomalies directly based
  # on revisions. Apply some filters to reduce unrelated anomalies.
  subscriptions = []
  for anomaly_entity in requested_anomalies:
    subscriptions.extend(anomaly_entity.subscription_names)
  subscriptions = list(set(subscriptions))
  min_range = utils.MinimumAlertRange(requested_anomalies)
  if min_range:
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        subscriptions=subscriptions, limit=_QUERY_LIMIT).get_result()

    # Filter out anomalies that have been marked as invalid or ignore.
    # Include all anomalies with an overlapping revision range that have
    # been associated with a bug, or are not yet triaged.
    requested_anomalies_set = {a.key for a in requested_anomalies}

    def _IsValidAlert(a):
      if a.key in requested_anomalies_set:
        return False
      return a.bug_id is None or a.bug_id > 0

    anomalies = [a for a in anomalies if _IsValidAlert(a)]
    anomalies = _GetOverlaps(anomalies, min_range[0], min_range[1])
    anomalies = requested_anomalies + anomalies
  else:
    anomalies = requested_anomalies
  return anomalies


def GetAlertsForGroupID(group_id):
  """Get alerts for AlertGroup.

  Args:
    group_id: AlertGroup ID

  Returns:
    list of anomaly.Anomaly
  """
  group = alert_group.AlertGroup.GetByID(group_id)
  if not group:
    raise request_handler.InvalidInputError('Invalid AlertGroup ID "%s".' %
                                            group_id)
  anomalies = perf_issue_service_client.GetAnomaliesByAlertGroupID(group_id)
  anomaly_keys = [
      ndb.Key('Anomaly', a) for a in anomalies if isinstance(a, int)
  ]
  if sorted(anomaly_keys) != sorted(group.anomalies):
    logging.warning('Imparity found for GetAnomaliesByAlertGroupID. %s, %s',
                    group.anomalies, anomaly_keys)
    cloud_metric.PublishPerfIssueServiceGroupingImpariry(
        'GetAnomaliesByAlertGroupID')
  return ndb.get_multi(anomaly_keys)


def _IsInt(x):
  """Returns True if the input can be parsed as an int."""
  try:
    int(x)
    return True
  except ValueError:
    return False


def _GetOverlaps(anomalies, start, end):
  """Gets the minimum range for the list of anomalies.

  Args:
    anomalies: The list of anomalies.
    start: The start revision.
    end: The end revision.

  Returns:
    A list of anomalies.
  """
  return [
      a for a in anomalies
      if a.start_revision <= end and a.end_revision >= start
  ]
