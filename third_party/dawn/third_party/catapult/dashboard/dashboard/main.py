# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""URL endpoint for the main page which lists recent anomalies."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
import logging

from google.appengine.ext import ndb

from dashboard import email_template
from dashboard.common import request_handler
from dashboard.common import utils
from dashboard.models import anomaly

_ANOMALY_FETCH_LIMIT = 1000
_DEFAULT_DAYS_TO_SHOW = 7
_DEFAULT_CHANGES_TO_SHOW = 10

from flask import request


def MainHandlerGet():
  days = int(request.args.get('days', _DEFAULT_DAYS_TO_SHOW))
  num_changes = int(request.args.get('num_changes', _DEFAULT_CHANGES_TO_SHOW))
  sheriff_name = request.args.get('sheriff', None)
  sheriff = None
  if sheriff_name:
    sheriff = ndb.Key('Sheriff', sheriff_name)

  anomalies = _GetRecentAnomalies(days, sheriff)

  top_improvements = _TopImprovements(anomalies, num_changes)
  top_regressions = _TopRegressions(anomalies, num_changes)
  tests = _GetKeyToTestDict(top_improvements + top_regressions)

  # Set the sheriff name to "all subscriptions" for the UI if not specified.
  if not sheriff_name:
    sheriff_name = 'all subscriptions'

  template_dict = {
      'num_days': days,
      'num_changes': num_changes,
      'sheriff_name': sheriff_name,
      'improvements': _AnomalyInfoDicts(top_improvements, tests),
      'regressions': _AnomalyInfoDicts(top_regressions, tests),
  }
  return request_handler.RequestHandlerRenderHtml('main.html', template_dict)


def _GetRecentAnomalies(days, sheriff):
  """Fetches recent Anomalies from the datastore.

  Args:
    days: Number of days old of the oldest Anomalies to fetch.
    sheriff: The ndb.Key of the Sheriff to fetch Anomalies for.

  Returns:
    A list of Anomaly entities sorted from large to small relative change.
  """
  sheriff_ids = None
  if sheriff:
    sheriff_ids = [sheriff.id()]
  anomalies, _, _ = anomaly.Anomaly.QueryAsync(
      min_timestamp=datetime.datetime.now() - datetime.timedelta(days=days),
      subscriptions=sheriff_ids,
      limit=_ANOMALY_FETCH_LIMIT).get_result()
  # We only want to list alerts that aren't marked invalid or ignored.
  anomalies = [
      a for a in anomalies
      if ((a.bug_id is None or a.bug_id > 0) and (a.source != 'skia'))
  ]
  anomalies.sort(key=lambda a: abs(a.percent_changed), reverse=True)
  return anomalies


def _GetKeyToTestDict(anomalies):
  """Returns a map of TestMetadata keys to entities for the given anomalies."""
  test_keys = {a.GetTestMetadataKey() for a in anomalies}
  tests = utils.GetMulti(test_keys)
  return {t.key: t for t in tests}


def _GetColorClass(percent_changed):
  """Returns a CSS class name for the anomaly, based on percent changed."""
  if percent_changed > 50:
    return 'over-50'
  if percent_changed > 40:
    return 'over-40'
  if percent_changed > 30:
    return 'over-30'
  if percent_changed > 20:
    return 'over-20'
  if percent_changed > 10:
    return 'over-10'
  return 'under-10'


def _AnomalyInfoDicts(anomalies, tests):
  """Returns information info about the given anomalies.

  Args:
    anomalies: A list of anomalies.
    tests: A dictionary mapping TestMetadata keys to TestMetadata entities.

  Returns:
    A list of dictionaries with information about the given anomalies.
  """
  anomaly_list = []
  for anomaly_entity in anomalies:
    test = tests.get(anomaly_entity.GetTestMetadataKey())
    if not test:
      logging.warning('No TestMetadata entity for key: %s.',
                      anomaly_entity.GetTestMetadataKey())
      continue
    subtest_path = '/'.join(test.test_path.split('/')[3:])
    graph_link = email_template.GetGroupReportPageLink(anomaly_entity)
    anomaly_list.append({
        'key': anomaly_entity.key.urlsafe(),
        'bug_id': anomaly_entity.bug_id or '',
        'start_revision': anomaly_entity.start_revision,
        'end_revision': anomaly_entity.end_revision,
        'master': test.master_name,
        'bot': test.bot_name,
        'testsuite': test.suite_name,
        'test': subtest_path,
        'percent_changed': anomaly_entity.GetDisplayPercentChanged(),
        'color_class': _GetColorClass(abs(anomaly_entity.percent_changed)),
        'improvement': anomaly_entity.is_improvement,
        'dashboard_link': graph_link,
    })
  return anomaly_list


def _TopImprovements(recent_anomalies, num_to_show):
  """Fills in the given template dictionary with top improvements.

  Args:
    recent_anomalies: A list of Anomaly entities sorted from large to small.
    num_to_show: The number of improvements to return.

  Returns:
    A list of top improvement Anomaly entities, in decreasing order.
  """
  improvements = [a for a in recent_anomalies if a.is_improvement]
  return improvements[:num_to_show]


def _TopRegressions(recent_anomalies, num_to_show):
  """Fills in the given template dictionary with top regressions.

  Args:
    recent_anomalies: A list of Anomaly entities sorted from large to small.
    num_to_show: The number of regressions to return.

  Returns:
    A list of top regression Anomaly entities, in decreasing order.
  """
  regressions = [a for a in recent_anomalies if not a.is_improvement]
  return regressions[:num_to_show]
