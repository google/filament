# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import Flask
from unittest import mock
import six
import unittest
import webtest

from dashboard import associate_alerts
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.models.subscription import Subscription
from dashboard.services import perf_issue_service_client


flask_app = Flask(__name__)


@flask_app.route('/associate_alerts', methods=['GET', 'POST'])
def AssociateAlertsHandlerPost():
  return associate_alerts.AssociateAlertsHandlerPost()


class AssociateAlertsTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.testapp = webtest.TestApp(flask_app)
    testing_common.SetSheriffDomains(['chromium.org'])
    self.SetCurrentUser('foo@chromium.org', is_admin=True)

  def _AddTests(self):
    """Adds sample Tests and returns a list of their keys."""
    testing_common.AddTests(
        ['ChromiumGPU'], ['linux-release'],
        {'scrolling-benchmark': {
            'first_paint': {},
            'mean_frame_time': {},
        }})
    return list(
        map(utils.TestKey, [
            'ChromiumGPU/linux-release/scrolling-benchmark/first_paint',
            'ChromiumGPU/linux-release/scrolling-benchmark/mean_frame_time',
        ]))

  def _AddAnomalies(self):
    """Adds sample Anomaly data and returns a dict of revision to key."""
    subscription = Subscription(
        name='Chromium Perf Sheriff', notification_email='sullivan@google.com')
    test_keys = self._AddTests()
    key_map = {}

    # Add anomalies to the two tests alternately.
    for end_rev in range(10000, 10120, 10):
      test_key = test_keys[0] if end_rev % 20 == 0 else test_keys[1]
      anomaly_key = anomaly.Anomaly(
          start_revision=(end_rev - 5),
          end_revision=end_rev,
          test=test_key,
          median_before_anomaly=100,
          median_after_anomaly=200,
          subscriptions=[subscription],
          subscription_names=[subscription.name],
      ).put()
      key_map[end_rev] = six.ensure_str(anomaly_key.urlsafe())

    # Add an anomaly that overlaps.
    anomaly_key = anomaly.Anomaly(
        start_revision=9990,
        end_revision=9996,
        test=test_keys[0],
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
    ).put()
    key_map[9996] = six.ensure_str(anomaly_key.urlsafe())

    # Add an anomaly that overlaps and has bug ID.
    anomaly_key = anomaly.Anomaly(
        start_revision=9990,
        end_revision=9997,
        test=test_keys[0],
        median_before_anomaly=100,
        median_after_anomaly=200,
        bug_id=12345,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
    ).put()
    key_map[9997] = six.ensure_str(anomaly_key.urlsafe())
    return key_map

  def testGet_NoKeys_ShowsError(self):
    response = self.testapp.get('/associate_alerts')
    self.assertIn(b'<div class="error">', response.body)

  def testGet_SameAsPost(self):
    get_response = self.testapp.get('/associate_alerts')
    post_response = self.testapp.post('/associate_alerts')
    self.assertEqual(get_response.body, post_response.body)

  def testGet_InvalidBugId_ShowsError(self):
    key_map = self._AddAnomalies()
    response = self.testapp.get('/associate_alerts?keys=%s&bug_id=foo' %
                                key_map[9996])
    self.assertIn(b'<div class="error">', response.body)
    self.assertIn(b'Invalid bug ID', response.body)

  # Mocks fetching bugs from issue tracker.
  @mock.patch('dashboard.common.utils.ServiceAccountHttp', mock.MagicMock())
  @mock.patch.object(
      perf_issue_service_client, 'GetIssues',
      mock.MagicMock(return_value=[
          {
              'id': 12345,
              'summary': '5% regression in bot/suite/x at 10000:20000',
              'state': 'open',
              'status': 'New',
              'author': {
                  'name': 'exam...@google.com'
              },
          },
          {
              'id': 13579,
              'summary': '1% regression in bot/suite/y at 10000:20000',
              'state': 'closed',
              'status': 'WontFix',
              'author': {
                  'name': 'exam...@google.com'
              },
          },
      ]))
  def testGet_NoBugId_ShowsDialog(self):
    # When a GET request is made with some anomaly keys but no bug ID,
    # A HTML form is shown for the user to input a bug number.
    key_map = self._AddAnomalies()
    response = self.testapp.get('/associate_alerts?keys=%s' % key_map[10000])

    # The response contains a table of recent bugs and a form.
    self.assertIn(b'12345', response.body)
    self.assertIn(b'13579', response.body)
    self.assertIn(b'<form', response.body)

  def testGet_WithBugId_AlertIsAssociatedWithBugId(self):
    # When the bug ID is given and the alerts overlap, then the Anomaly
    # entities are updated and there is a response indicating success.
    key_map = self._AddAnomalies()
    response = self.testapp.get(
        '/associate_alerts?keys=%s,%s&bug_id=12345&project_id=test_project' %
        (key_map[9996], key_map[10000]))

    # The response page should have a bug number.
    self.assertIn(b'12345', response.body)
    # The Anomaly entities should be updated.
    for anomaly_entity in anomaly.Anomaly.query().fetch():
      if anomaly_entity.end_revision in (10000, 9996):
        self.assertEqual(12345, anomaly_entity.bug_id)
        self.assertEqual('test_project', anomaly_entity.project_id)
      elif anomaly_entity.end_revision != 9997:
        self.assertIsNone(anomaly_entity.bug_id)
        self.assertEqual('chromium', anomaly_entity.project_id)

  def testGet_WithBugId_AlertIsAssociatedWithBugIdAndNoProject(self):
    # When the bug ID is given and the alerts overlap, then the Anomaly
    # entities are updated and there is a response indicating success.
    key_map = self._AddAnomalies()
    response = self.testapp.get('/associate_alerts?keys=%s,%s&bug_id=12345' %
                                (key_map[9996], key_map[10000]))

    # The response page should have a bug number.
    self.assertIn(b'12345', response.body)
    # The Anomaly entities should be updated.
    for anomaly_entity in anomaly.Anomaly.query().fetch():
      if anomaly_entity.end_revision in (10000, 9996):
        self.assertEqual(12345, anomaly_entity.bug_id)
        self.assertEqual('chromium', anomaly_entity.project_id)
      elif anomaly_entity.end_revision != 9997:
        self.assertIsNone(anomaly_entity.bug_id)
        self.assertEqual('chromium', anomaly_entity.project_id)

  def testGet_TargetBugHasNoAlerts_DoesNotAskForConfirmation(self):
    # Associating alert with bug ID that has no alerts is always OK.
    key_map = self._AddAnomalies()
    response = self.testapp.get('/associate_alerts?keys=%s,%s&bug_id=578' %
                                (key_map[9996], key_map[10000]))

    # The response page should have a bug number.
    self.assertIn(b'578', response.body)
    # The Anomaly entities should be updated.
    self.assertEqual(
        578,
        anomaly.Anomaly.query(
            anomaly.Anomaly.end_revision == 9996).get().bug_id)
    self.assertEqual(
        578,
        anomaly.Anomaly.query(
            anomaly.Anomaly.end_revision == 10000).get().bug_id)

  def testGet_NonOverlappingAlerts_AsksForConfirmation(self):
    # Associating alert with bug ID that contains non-overlapping revision
    # ranges should show a confirmation page.
    key_map = self._AddAnomalies()
    response = self.testapp.get('/associate_alerts?keys=%s,%s&bug_id=12345' %
                                (key_map[10000], key_map[10010]))

    # The response page should show confirmation page.
    self.assertIn(b'Do you want to continue?', response.body)
    # The Anomaly entities should not be updated.
    for anomaly_entity in anomaly.Anomaly.query().fetch():
      if anomaly_entity.end_revision != 9997:
        self.assertIsNone(anomaly_entity.bug_id)
        self.assertEqual('chromium', anomaly_entity.project_id)

  def testGet_WithConfirm_AssociatesWithNewBugId(self):
    # Associating alert with bug ID and with confirmed non-overlapping revision
    # range should update alert with bug ID.
    key_map = self._AddAnomalies()
    response = self.testapp.get(
        '/associate_alerts?confirm=true&keys=%s,%s&bug_id=12345&'
        'project_id=test_project' % (key_map[10000], key_map[10010]))

    # The response page should have the bug number.
    self.assertIn(b'12345', response.body)
    # The Anomaly entities should be updated.
    for anomaly_entity in anomaly.Anomaly.query().fetch():
      if anomaly_entity.end_revision in (10000, 10010):
        self.assertEqual(12345, anomaly_entity.bug_id)
        self.assertEqual('test_project', anomaly_entity.project_id)
      elif anomaly_entity.end_revision != 9997:
        self.assertIsNone(anomaly_entity.bug_id)
        self.assertEqual('chromium', anomaly_entity.project_id)

  def testRevisionRangeFromSummary(self):
    # If the summary is in the expected format, a pair is returned.
    self.assertEqual((10000, 10500),
                     associate_alerts._RevisionRangeFromSummary(
                         '1% regression in bot/my_suite/test at 10000:10500'))

    # Otherwise None is returned.
    self.assertIsNone(
        associate_alerts._RevisionRangeFromSummary(
            'Regression in rev ranges 12345 to 20000'))

  def testRangesOverlap_NonOverlapping_ReturnsFalse(self):
    self.assertFalse(associate_alerts._RangesOverlap((1, 5), (6, 9)))
    self.assertFalse(associate_alerts._RangesOverlap((6, 9), (1, 5)))

  def testRangesOverlap_NoneGiven_ReturnsFalse(self):
    self.assertFalse(associate_alerts._RangesOverlap((1, 5), None))
    self.assertFalse(associate_alerts._RangesOverlap(None, (1, 5)))
    self.assertFalse(associate_alerts._RangesOverlap(None, None))

  def testRangesOverlap_OneIncludesOther_ReturnsTrue(self):
    # True if one range envelopes the other.
    self.assertTrue(associate_alerts._RangesOverlap((1, 9), (2, 5)))
    self.assertTrue(associate_alerts._RangesOverlap((2, 5), (1, 9)))

  def testRangesOverlap_PartlyOverlap_ReturnsTrue(self):
    self.assertTrue(associate_alerts._RangesOverlap((1, 6), (5, 9)))
    self.assertTrue(associate_alerts._RangesOverlap((5, 9), (1, 6)))

  def testRangesOverlap_CommonBoundary_ReturnsTrue(self):
    self.assertTrue(associate_alerts._RangesOverlap((1, 6), (6, 9)))
    self.assertTrue(associate_alerts._RangesOverlap((6, 9), (1, 6)))


if __name__ == '__main__':
  unittest.main()
