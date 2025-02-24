# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides the web interface for displaying an overview of alerts."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
import six

from google.appengine.datastore.datastore_query import Cursor
from google.appengine.ext import ndb

from dashboard import email_template
from dashboard.common import descriptor
from dashboard.common import request_handler
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.models import bug_label_patterns
from dashboard.models import skia_helper

# We need to import this last because importing it earlier causes issues with
# module lookups when importing protobuf messages before loading the
# appengine-specific shims to resolve package name lookup. This shows up as
# interesting non-deterministic issues with test loading order.
from dashboard import sheriff_config_client

_MAX_ANOMALIES_TO_COUNT = 5000
_MAX_ANOMALIES_TO_SHOW = 500


# Shows an overview of recent anomalies for perf sheriffing.
from flask import make_response, request


def AlertsHandlerGet():
  """Renders the UI for listing alerts."""
  return request_handler.RequestHandlerRenderStaticHtml('alerts.html')


def AlertsHandlerPost():
  """Returns dynamic data for listing alerts in response to XHR.

  Request parameters:
    sheriff: The name of a sheriff (optional).
    triaged: Whether to include triaged alerts (i.e. with a bug ID).
    improvements: Whether to include improvement anomalies.
    anomaly_cursor: Where to begin a paged query for anomalies (optional).

  Outputs:
    JSON data for an XHR request to show a table of alerts.
  """
  sheriff_name = request.values.get('sheriff', None)
  if sheriff_name and not _SheriffIsFound(sheriff_name):
    return make_response(
        json.dumps({'error': 'Sheriff "%s" not found.' % sheriff_name}))

  # Cursors are used to fetch paged queries. If none is supplied, then the
  # first 500 alerts will be returned. If a cursor is given, the next
  # 500 alerts (starting at the given cursor) will be returned.
  anomaly_cursor = request.values.get('anomaly_cursor', None)
  if anomaly_cursor:
    anomaly_cursor = Cursor(urlsafe=anomaly_cursor)

  is_improvement = None
  if not bool(request.values.get('improvements')):
    is_improvement = False

  bug_id = None
  recovered = None
  if not bool(request.values.get('triaged')):
    bug_id = ''
    recovered = False

  max_anomalies_to_show = _MAX_ANOMALIES_TO_SHOW
  if request.values.get('max_anomalies_to_show'):
    max_anomalies_to_show = int(request.values.get('max_anomalies_to_show'))

  subs = None
  if sheriff_name:
    subs = [sheriff_name]
  anomalies, next_cursor, count = anomaly.Anomaly.QueryAsync(
      start_cursor=anomaly_cursor,
      subscriptions=subs,
      bug_id=bug_id,
      is_improvement=is_improvement,
      recovered=recovered,
      count_limit=_MAX_ANOMALIES_TO_COUNT,
      limit=max_anomalies_to_show).get_result()

  values = {
      'anomaly_list': AnomalyDicts(anomalies),
      'anomaly_count': count,
      'sheriff_list': _GetSheriffList(),
      'anomaly_cursor':
          (six.ensure_str(next_cursor.urlsafe()) if next_cursor else None),
      'show_more_anomalies': next_cursor != None,
  }
  request_handler.RequestHandlerGetDynamicVariables(values)
  return make_response(json.dumps(values))


def _SheriffIsFound(sheriff_name):
  """Checks whether the sheriff can be found for the current user."""
  # TODO(fancl): Add an api for verifying single subscription visibility
  subscriptions = _GetSheriffList()
  return sheriff_name in subscriptions


def _GetSheriffList():
  """Returns a list of sheriff names for all sheriffs in the datastore."""
  client = sheriff_config_client.GetSheriffConfigClient()
  subscriptions, _ = client.List(check=True)
  return [s.name for s in subscriptions]


def AnomalyDicts(anomalies, v2=False):
  """Makes a list of dicts with properties of Anomaly entities."""
  bisect_statuses = _GetBisectStatusDict(anomalies)
  return [
      GetAnomalyDict(a, bisect_statuses.get(a.bug_id), v2) for a in anomalies
  ]


def GetAnomalyDict(anomaly_entity, bisect_status=None, v2=False):
  """Returns a dictionary for an Anomaly which can be encoded as JSON.

  Args:
    anomaly_entity: An Anomaly entity.
    bisect_status: String status of bisect run.

  Returns:
    A dictionary which is safe to be encoded as JSON.
  """
  test_key = anomaly_entity.GetTestMetadataKey()
  test_path = utils.TestPath(test_key)
  dashboard_link = email_template.GetReportPageLink(
      test_path, rev=anomaly_entity.end_revision, add_protocol_and_host=False)
  project_id = anomaly_entity.project_id if (
      anomaly_entity.project_id != '') else 'chromium'

  new_url = skia_helper.GetSkiaUrlForAnomaly(anomaly_entity)
  dct = {
      'bug_id': anomaly_entity.bug_id,
      'project_id': project_id,
      'dashboard_link': dashboard_link,
      'end_revision': anomaly_entity.end_revision,
      'improvement': anomaly_entity.is_improvement,
      'key': six.ensure_str(anomaly_entity.key.urlsafe()),
      'median_after_anomaly': anomaly_entity.median_after_anomaly,
      'median_before_anomaly': anomaly_entity.median_before_anomaly,
      'recovered': anomaly_entity.recovered,
      'start_revision': anomaly_entity.start_revision,
      'units': anomaly_entity.units,
      'new_url': new_url
  }

  if v2:
    bug_labels = set()
    bug_components = set()
    if anomaly_entity.internal_only:
      bug_labels.add('Restrict-View-Google')
    tags = set(bug_label_patterns.GetBugLabelsForTest(test_key))
    subscriptions = list(anomaly_entity.subscriptions)
    tags.update([l for s in subscriptions for l in s.bug_labels])
    bug_components = set(c for s in subscriptions for c in s.bug_components)
    for tag in tags:
      if tag.startswith('Cr-'):
        bug_components.add(tag.replace('Cr-', '').replace('-', '>'))
      else:
        bug_labels.add(tag)

    dct['bug_components'] = list(bug_components)
    dct['bug_labels'] = list(bug_labels)

    desc = descriptor.Descriptor.FromTestPathSync(test_path)
    dct['descriptor'] = {
        'testSuite': desc.test_suite,
        'measurement': desc.measurement,
        'bot': desc.bot,
        'testCase': desc.test_case,
        'statistic': desc.statistic,
    }
    dct['pinpoint_bisects'] = anomaly_entity.pinpoint_bisects
  else:
    test_path_parts = test_path.split('/')
    dct['absolute_delta'] = '%s' % anomaly_entity.GetDisplayAbsoluteChanged()
    dct['bisect_status'] = bisect_status
    dct['bot'] = test_path_parts[1]
    dct['date'] = str(anomaly_entity.timestamp.date())
    dct['display_end'] = anomaly_entity.display_end
    dct['display_start'] = anomaly_entity.display_start
    dct['master'] = test_path_parts[0]
    dct['percent_changed'] = '%s' % anomaly_entity.GetDisplayPercentChanged()
    dct['ref_test'] = anomaly_entity.GetRefTestPath()
    dct['test'] = '/'.join(test_path_parts[3:])
    dct['testsuite'] = test_path_parts[2]
    dct['timestamp'] = anomaly_entity.timestamp.isoformat()
    dct['type'] = 'anomaly'

  return dct


def _GetBisectStatusDict(anomalies):
  """Returns a dictionary of bug ID to bisect status string."""
  bug_id_list = {a.bug_id for a in anomalies if a.bug_id and a.bug_id > 0}
  bugs = ndb.get_multi(ndb.Key('Bug', b) for b in bug_id_list)
  return {b.key.id(): b.latest_bisect_status for b in bugs if b}
