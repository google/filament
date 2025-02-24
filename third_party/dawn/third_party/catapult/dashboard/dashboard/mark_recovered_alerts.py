# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""URL endpoint for a cron job to automatically mark alerts which recovered."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging
from datetime import datetime
from datetime import timedelta

from google.appengine.api import taskqueue
from google.appengine.ext import ndb

from dashboard import find_anomalies
from dashboard.common import math_utils
from dashboard.common import utils
from dashboard.common import datastore_hooks
from dashboard.models import anomaly
from dashboard.models import anomaly_config
from dashboard.services import perf_issue_service_client

from flask import request, make_response

_TASK_QUEUE_NAME = 'auto-triage-queue'

_MAX_UNTRIAGED_ANOMALIES = 20000

_MAX_DATES_TO_IGNORE_ALERT = 180

# Maximum relative difference between two steps for them to be considered
# similar enough for the second to be a "recovery" of the first.
# For example, if there's an increase of 5 units followed by a decrease of 6
# units, the relative difference of the deltas is 0.2.
_MAX_DELTA_DIFFERENCE = 0.25


def MarkRecoveredAlertsPost():
  """Checks if alerts have recovered, and marks them if so.

  This includes checking untriaged alerts, as well as alerts associated with
  open bugs..
  """
  datastore_hooks.SetPrivilegedRequest()

  # Handle task queue requests.
  bug_id = request.values.get('bug_id')
  project_id = request.values.get('project_id')
  if bug_id:
    bug_id = int(bug_id)
  if not project_id:
    project_id = 'chromium'
  if request.values.get('check_alert'):
    MarkAlertAndBugIfRecovered(
        request.values.get('alert_key'), bug_id, project_id)
    return make_response('')
  if request.values.get('check_bug'):
    CheckRecoveredAlertsForBug(bug_id, project_id)
    return make_response('')

  # Kick off task queue jobs for untriaged anomalies.
  alerts = _FetchUntriagedAnomalies()
  logging.info('Kicking off tasks for %d alerts', len(alerts))
  for alert in alerts:
    taskqueue.add(
        url='/mark_recovered_alerts',
        params={
            'check_alert': 1,
            'alert_key': alert.key.urlsafe()
        },
        queue_name=_TASK_QUEUE_NAME)

  # Kick off task queue jobs for open bugs.
  bugs = _FetchOpenBugs()
  logging.info('Kicking off tasks for %d bugs', len(bugs))
  for bug in bugs:
    taskqueue.add(
        url='/mark_recovered_alerts',
        params={
            'check_bug': 1,
            'bug_id': bug['id']
        },
        queue_name=_TASK_QUEUE_NAME)
  return make_response('')


def _FetchUntriagedAnomalies():
  """Fetches recent untriaged anomalies asynchronously from all sheriffs."""
  # Previous code process anomalies by sheriff with LIMIT. It prevents some
  # extreme cases that anomalies produced by a single sheriff prevent other
  # sheriff's anomalies being processed. But it introduced some unnecessary
  # complex to system and considered almost impossible happened.
  min_timestamp_to_check = (
      datetime.now() - timedelta(days=_MAX_DATES_TO_IGNORE_ALERT))
  logging.info('Fetching untriaged anomalies fired after %s',
               min_timestamp_to_check)
  future = anomaly.Anomaly.QueryAsync(
      keys_only=True,
      limit=_MAX_UNTRIAGED_ANOMALIES,
      recovered=False,
      is_improvement=False,
      bug_id='',
      min_timestamp=min_timestamp_to_check)
  future.wait()
  anomalies = future.get_result()[0]
  return anomalies


def _FetchOpenBugs():
  """Fetches a list of open bugs on all sheriffing labels."""
  bugs = perf_issue_service_client.GetIssues(
      status='open',
      age=365,
      labels='Performance-Sheriff,Performance-Sheriff-V8',
      limit=1000)

  return bugs


def MarkAlertAndBugIfRecovered(alert_key_urlsafe, bug_id, project_id):
  """Checks whether an alert has recovered, and marks it if so.

  An alert will be considered "recovered" if there's a change point in
  the series after it with roughly equal magnitude and opposite direction.

  Args:
    alert_key_urlsafe: The original regression Anomaly.

  Returns:
    True if the Anomaly should be marked as recovered, False otherwise.
  """
  alert_entity = ndb.Key(urlsafe=alert_key_urlsafe).get()
  logging.info('Checking alert %s', alert_entity)
  if not _IsAlertRecovered(alert_entity, bug_id, project_id):
    return

  logging.info('Recovered')
  alert_entity.recovered = True
  alert_entity.put()
  if bug_id:
    unrecovered, _, _ = anomaly.Anomaly.QueryAsync(
        bug_id=bug_id, project_id=project_id, recovered=False).get_result()
    if not unrecovered:
      # All alerts recovered! Update bug.
      logging.info('All alerts for bug %s recovered!', bug_id)
      comment = 'Automatic message: All alerts recovered.\nGraphs: %s' % (
          'https://chromeperf.appspot.com/group_report?bug_id=%s' % bug_id)
      perf_issue_service_client.PostIssueComment(
          issue_id=bug_id,
          comment=comment,
          project_name=project_id,
          labels='Performance-Regression-Recovered')


def _IsAlertRecovered(alert_entity, bug_id, project_id):
  if alert_entity.recovered:
    return True
  test = alert_entity.GetTestMetadataKey().get()
  if not test:
    logging.error('TestMetadata %s not found for Anomaly %s.',
                  utils.TestPath(alert_entity.GetTestMetadataKey()),
                  alert_entity)
    datetime_since_fired = (datetime.now() - alert_entity.timestamp).days
    if datetime_since_fired > _MAX_DATES_TO_IGNORE_ALERT:
      comment = """
        The test related to the alert below cannot be found in data store.
        As it has been %s days since reported, we consider this alert no
        longer valid and mark it as recovered.
        Alert: %s
        TestMetadataKey: %s
        """ % (datetime_since_fired, alert_entity,
               alert_entity.GetTestMetadataKey())
      logging.info(comment)
      alert_entity.recovered = True
      alert_entity.put()
      if bug_id:
        perf_issue_service_client.PostIssueComment(
            issue_id=bug_id, comment=comment, project_name=project_id)
    return alert_entity.recovered
  config = anomaly_config.GetAnomalyConfigDict(test)
  max_num_rows = config.get('max_window_size',
                            find_anomalies.DEFAULT_NUM_POINTS)
  rows = find_anomalies.GetRowsToAnalyze(test, max_num_rows)
  statistic = getattr(alert_entity, 'statistic')
  if not statistic:
    statistic = 'avg'
  rows = rows.get(statistic, [])
  rows = [(rev, row, val)
          for (rev, row, val) in rows
          if row.revision > alert_entity.end_revision]

  change_points = find_anomalies.FindChangePointsForTest(rows, config)
  delta_anomaly = (
      alert_entity.median_after_anomaly - alert_entity.median_before_anomaly)
  for change in change_points:
    delta_change = change.median_after - change.median_before
    if (_IsOppositeDirection(delta_anomaly, delta_change)
        and _IsApproximatelyEqual(delta_anomaly, -delta_change)):
      logging.debug('Anomaly %s recovered; recovery change point %s.',
                    alert_entity.key, change.AsDict())
      return True
  return False


def _IsOppositeDirection(delta1, delta2):
  return delta1 * delta2 < 0


def _IsApproximatelyEqual(delta1, delta2):
  smaller = min(delta1, delta2)
  larger = max(delta1, delta2)
  return math_utils.RelativeChange(smaller, larger) <= _MAX_DELTA_DIFFERENCE


def CheckRecoveredAlertsForBug(bug_id, project_id):
  unrecovered, _, _ = anomaly.Anomaly.QueryAsync(
      bug_id=bug_id, recovered=False).get_result()
  logging.info('Queueing %d alerts for bug %s:%s', len(unrecovered), project_id,
               bug_id)
  for alert in unrecovered:
    taskqueue.add(
        url='/mark_recovered_alerts',
        params={
            'check_alert': 1,
            'alert_key': alert.key.urlsafe(),
            'bug_id': bug_id,
            'project_id': project_id,
        },
        queue_name=_TASK_QUEUE_NAME)
