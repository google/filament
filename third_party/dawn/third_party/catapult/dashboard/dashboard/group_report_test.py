# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import Flask
import itertools
import json
from unittest import mock
import unittest

import six
import webtest

from google.appengine.ext import ndb

from dashboard import group_report
from dashboard import short_uri
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import alert_group
from dashboard.models import anomaly
from dashboard.models import bug_data
from dashboard.models import page_state
from dashboard.models.subscription import Subscription
from dashboard.services import perf_issue_service_client

flask_app = Flask(__name__)


@flask_app.route('/group_report', methods=['GET'])
def GroupReportGet():
  return group_report.GroupReportGet()


@flask_app.route('/group_report', methods=['POST'])
def GroupReportPost():
  return group_report.GroupReportPost()


class GroupReportTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.testapp = webtest.TestApp(flask_app)

  def _AddAnomalyEntities(self,
                          revision_ranges,
                          test_key,
                          subscriptions,
                          bug_id=None,
                          project_id=None,
                          group_id=None):
    """Adds a group of Anomaly entities to the datastore."""
    urlsafe_keys = []
    keys = []
    for start_rev, end_rev in revision_ranges:
      subscription_names = [s.name for s in subscriptions]
      anomaly_key = anomaly.Anomaly(
          start_revision=start_rev,
          end_revision=end_rev,
          test=test_key,
          bug_id=bug_id,
          project_id=project_id,
          subscription_names=subscription_names,
          subscriptions=subscriptions,
          median_before_anomaly=100,
          median_after_anomaly=200).put()
      urlsafe_keys.append(six.ensure_str(anomaly_key.urlsafe()))
      keys.append(anomaly_key)
    if group_id:
      alert_group.AlertGroup(
          id=group_id,
          anomalies=keys,
      ).put()
    return urlsafe_keys

  def _AddTests(self):
    """Adds sample TestMetadata entities and returns their keys."""
    testing_common.AddTests(
        ['ChromiumGPU'], ['linux-release'],
        {'scrolling-benchmark': {
            'first_paint': {},
            'mean_frame_time': {},
        }})
    keys = [
        utils.TestKey(
            'ChromiumGPU/linux-release/scrolling-benchmark/first_paint'),
        utils.TestKey(
            'ChromiumGPU/linux-release/scrolling-benchmark/mean_frame_time'),
    ]
    # By default, all TestMetadata entities have an improvement_direction of
    # UNKNOWN, meaning that neither direction is considered an improvement.
    # Here we set the improvement direction so that some anomalies are
    # considered improvements.
    for test_key in keys:
      test = test_key.get()
      test.improvement_direction = anomaly.DOWN
      test.put()
    return keys

  def _Subscription(self, suffix=""):
    """Adds a Sheriff entity and returns the key."""
    return Subscription(
        name='Chromium Perf Sheriff' + suffix,
        notification_email='sullivan@google.com')

  def testGet(self):
    response = self.testapp.get('/group_report')
    self.assertEqual('text/html', response.content_type)
    self.assertIn(b'Chrome Performance Dashboard', response.body)

  def testPost_WithAnomalyKeys_ShowsSelectedAndOverlapping(self):
    subscriptions = [
        self._Subscription(suffix=" 1"),
        self._Subscription(suffix=" 2"),
    ]
    test_keys = self._AddTests()
    selected_ranges = [(400, 900), (200, 700)]
    overlapping_ranges = [(300, 500), (500, 600), (600, 800)]
    non_overlapping_ranges = [(100, 200)]
    selected_keys = self._AddAnomalyEntities(selected_ranges, test_keys[0],
                                             subscriptions)
    self._AddAnomalyEntities(overlapping_ranges, test_keys[0], subscriptions)
    self._AddAnomalyEntities(non_overlapping_ranges, test_keys[0],
                             subscriptions)

    response = self.testapp.post('/group_report?keys=%s' %
                                 ','.join(selected_keys))
    alert_list = self.GetJsonValue(response, 'alert_list')

    # Confirm the first N keys are the selected keys.
    first_keys = [
        alert['key']
        for alert in itertools.islice(alert_list, len(selected_keys))
    ]
    self.assertSetEqual(set(selected_keys), set(first_keys))

    # Expect selected alerts + overlapping alerts,
    # but not the non-overlapping alert.
    self.assertEqual(5, len(alert_list))

  def testPost_WithInvalidSidParameter_ShowsError(self):
    response = self.testapp.post('/group_report?sid=foobar')
    error = self.GetJsonValue(response, 'error')
    self.assertIn('No anomalies specified', error)

  def testPost_WithValidSidParameter(self):
    subscription = self._Subscription()
    test_keys = self._AddTests()
    selected_ranges = [(400, 900), (200, 700)]
    selected_keys = self._AddAnomalyEntities(selected_ranges, test_keys[0],
                                             [subscription])

    json_keys = six.ensure_binary(json.dumps(selected_keys))
    state_id = short_uri.GenerateHash(','.join(selected_keys))
    page_state.PageState(id=state_id, value=json_keys).put()

    response = self.testapp.post('/group_report?sid=%s' % state_id)
    alert_list = self.GetJsonValue(response, 'alert_list')

    # Confirm the first N keys are the selected keys.
    first_keys = [
        alert['key']
        for alert in itertools.islice(alert_list, len(selected_keys))
    ]
    self.assertSetEqual(set(selected_keys), set(first_keys))
    self.assertEqual(2, len(alert_list))

  def testPost_WithKeyOfNonExistentAlert_ShowsError(self):
    key = ndb.Key('Anomaly', 123)
    response = self.testapp.post('/group_report?keys=%s' %
                                 six.ensure_str(key.urlsafe()))
    error = self.GetJsonValue(response, 'error')
    self.assertEqual(
        'No Anomaly found for key %s.' % six.ensure_str(key.urlsafe()), error)

  def testPost_WithInvalidKeyParameter_ShowsError(self):
    response = self.testapp.post('/group_report?keys=foobar')
    error = self.GetJsonValue(response, 'error')
    self.assertIn('Invalid Anomaly key', error)

  def testPost_WithRevParameter(self):
    # If the rev parameter is given, then all alerts whose revision range
    # includes the given revision should be included.
    subscription = self._Subscription()
    test_keys = self._AddTests()
    self._AddAnomalyEntities([(190, 210), (200, 300), (100, 200), (400, 500)],
                             test_keys[0], [subscription])
    response = self.testapp.post('/group_report?rev=200')
    alert_list = self.GetJsonValue(response, 'alert_list')
    self.assertEqual(3, len(alert_list))

  def testPost_WithInvalidRevParameter_ShowsError(self):
    response = self.testapp.post('/group_report?rev=foo')
    error = self.GetJsonValue(response, 'error')
    self.assertEqual('Invalid rev "foo".', error)

  def testPost_WithBugIdParameter(self):
    subscription = self._Subscription()
    test_keys = self._AddTests()
    bug_data.Bug.New(project='chromium', bug_id=123).put()
    self._AddAnomalyEntities([(200, 300), (100, 200), (400, 500)],
                             test_keys[0], [subscription],
                             bug_id=123,
                             project_id='test_project')
    self._AddAnomalyEntities([(150, 250)], test_keys[0], [subscription])
    response = self.testapp.post(
        '/group_report?bug_id=123&project_id=test_project')
    alert_list = self.GetJsonValue(response, 'alert_list')
    self.assertEqual(3, len(alert_list))

  def testPost_WithProjectIdMissing(self):
    subscription = self._Subscription()
    test_keys = self._AddTests()
    bug_data.Bug.New(project='chromium', bug_id=123).put()
    self._AddAnomalyEntities([(200, 300), (100, 200), (400, 500)],
                             test_keys[0], [subscription],
                             bug_id=123,
                             project_id='chromium')
    self._AddAnomalyEntities([(150, 250)], test_keys[0], [subscription])
    response = self.testapp.post('/group_report?bug_id=123')
    alert_list = self.GetJsonValue(response, 'alert_list')
    self.assertEqual(3, len(alert_list))

  def testPost_WithInvalidBugIdParameter_ShowsError(self):
    response = self.testapp.post('/group_report?bug_id=foo')
    alert_list = self.GetJsonValue(response, 'alert_list')
    self.assertIsNone(alert_list)
    error = self.GetJsonValue(response, 'error')
    self.assertEqual('Invalid bug ID "chromium:foo".', error)

  @mock.patch.object(perf_issue_service_client, 'GetAnomaliesByAlertGroupID',
                     mock.MagicMock(return_value=[1, 2, 3]))
  def testPost_WithGroupIdParameter(self):
    subscription = self._Subscription()
    test_keys = self._AddTests()
    self._AddAnomalyEntities([(200, 300), (100, 200), (400, 500)],
                             test_keys[0], [subscription],
                             group_id="123")
    self._AddAnomalyEntities([(150, 250)], test_keys[0], [subscription])
    response = self.testapp.post('/group_report?group_id=123')
    alert_list = self.GetJsonValue(response, 'alert_list')
    self.assertEqual(3, len(alert_list))

  @mock.patch.object(perf_issue_service_client, 'GetAnomaliesByAlertGroupID',
                     mock.MagicMock(return_value=[1, 2, 3, '1-2-3']))
  def testPost_WithGroupIdParameterWithNonIntegerAnomalyId(self):
    subscription = self._Subscription()
    test_keys = self._AddTests()
    self._AddAnomalyEntities([(200, 300), (100, 200), (400, 500), (600, 700)],
                             test_keys[0], [subscription],
                             group_id="123")
    self._AddAnomalyEntities([(150, 250)], test_keys[0], [subscription])
    response = self.testapp.post('/group_report?group_id=123')
    alert_list = self.GetJsonValue(response, 'alert_list')
    self.assertEqual(3, len(alert_list))

  def testPost_WithInvalidGroupIdParameter(self):
    response = self.testapp.post('/group_report?group_id=foo')
    alert_list = self.GetJsonValue(response, 'alert_list')
    self.assertIsNone(alert_list)
    error = self.GetJsonValue(response, 'error')
    self.assertEqual('Invalid AlertGroup ID "foo".', error)


if __name__ == '__main__':
  unittest.main()
