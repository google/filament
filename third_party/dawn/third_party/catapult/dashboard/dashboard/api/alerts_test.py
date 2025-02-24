# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
from flask import Flask
import json
import unittest

from dashboard.api import alerts
from dashboard.api import api_auth
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.models import report_template
from dashboard.models.subscription import Subscription

flask_app = Flask(__name__)


@flask_app.route('/api/alerts', methods=['POST', 'OPTIONS'])
def ApiAlertsPost():
  return alerts.AlertsPost()


class AlertsGeneralTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.SetUpFlaskApp(flask_app)
    self.SetCurrentClientIdOAuth(api_auth.OAUTH_CLIENT_ID_ALLOWLIST[0])

  def _Post(self, **params):
    return json.loads(self.Post('/api/alerts', params).body)

  def _CreateAnomaly(self,
                     internal_only=False,
                     timestamp=None,
                     bug_id=None,
                     sheriff_name=None,
                     test='master/bot/test_suite/measurement/test_case',
                     start_revision=0,
                     end_revision=100,
                     display_start=0,
                     display_end=100,
                     median_before_anomaly=100,
                     median_after_anomaly=200,
                     is_improvement=False,
                     recovered=False):
    entity = anomaly.Anomaly()
    entity.internal_only = internal_only
    if timestamp:
      entity.timestamp = timestamp
    entity.bug_id = bug_id
    if sheriff_name:
      entity.subscriptions = [
          Subscription(
              name=sheriff_name,
              notification_email='sullivan@google.com',
          )
      ]
      entity.subscription_names = [sheriff_name]
    if test:
      entity.test = utils.TestKey(test)
    entity.start_revision = start_revision
    entity.end_revision = end_revision
    entity.display_start = display_start
    entity.display_end = display_end
    entity.median_before_anomaly = median_before_anomaly
    entity.median_after_anomaly = median_after_anomaly
    entity.is_improvement = is_improvement
    entity.recovered = recovered
    return entity.put().urlsafe()

  def testCountLimit(self):
    self._CreateAnomaly()
    self._CreateAnomaly()
    response = self._Post(limit=1, count_limit=10)
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(2, response['count'])

  def testAllExternal(self):
    self._CreateAnomaly()
    self._CreateAnomaly(internal_only=True)
    response = self._Post()
    self.assertEqual(1, len(response['anomalies']))

  def testKey(self):
    response = self._Post(key=self._CreateAnomaly())
    self.assertEqual(1, len(response['anomalies']))

  def testKeyInternal_Internal(self):
    self.SetCurrentUserOAuth(testing_common.INTERNAL_USER)
    response = self._Post(key=self._CreateAnomaly(internal_only=True))
    self.assertEqual(1, len(response['anomalies']))

  def testKeyInternal_External(self):
    response = self._Post(key=self._CreateAnomaly(internal_only=True))
    self.assertEqual(0, len(response['anomalies']))

  def testBot(self):
    self._CreateAnomaly()
    self._CreateAnomaly(test='adept/android/lodging/assessment/story')
    response = self._Post(bot='android')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual('android', response['anomalies'][0]['bot'])

  def testReport(self):
    self._CreateAnomaly()
    self._CreateAnomaly(test='adept/android/lodging/assessment/story')
    report_template.ReportTemplate(
        name='foo',
        id=42,
        template={
            'rows': [{
                'testSuites': ['lodging'],
                'measurement': 'assessment',
                'bots': ['adept:android'],
                'testCases': ['story']
            }]
        }).put()
    response = self._Post(report=42)
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual('android', response['anomalies'][0]['bot'])

  def testMaster(self):
    self._CreateAnomaly()
    self._CreateAnomaly(test='adept/android/lodging/assessment/story')
    response = self._Post(master='adept')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual('adept', response['anomalies'][0]['master'])

  def testTestSuite(self):
    self._CreateAnomaly()
    self._CreateAnomaly(test='adept/android/lodging/assessment/story')
    response = self._Post(test_suite='lodging')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual('lodging', response['anomalies'][0]['testsuite'])

  def testTest(self):
    self._CreateAnomaly()
    self._CreateAnomaly(test='adept/android/lodging/assessment/story')
    response = self._Post(test='adept/android/lodging/assessment/story')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual('assessment/story', response['anomalies'][0]['test'])

  def testBugId(self):
    self._CreateAnomaly()
    self._CreateAnomaly(bug_id=42)
    response = self._Post(bug_id=42)
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(42, response['anomalies'][0]['bug_id'])

    response = self._Post(bug_id='')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(None, response['anomalies'][0]['bug_id'])

  def testIsImprovement(self):
    self._CreateAnomaly()
    self._CreateAnomaly(is_improvement=True)
    response = self._Post(is_improvement='true')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(True, response['anomalies'][0]['improvement'])

    response = self._Post(is_improvement='false')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(False, response['anomalies'][0]['improvement'])

  def testIsImprovement_Invalid(self):
    self._CreateAnomaly()
    self._CreateAnomaly(is_improvement=True)
    with self.assertRaises(Exception):
      self._Post(is_improvement='invalid')

  def testRecovered(self):
    self._CreateAnomaly()
    self._CreateAnomaly(recovered=True)
    response = self._Post(recovered='true')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(True, response['anomalies'][0]['recovered'])

    response = self._Post(recovered='false')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(False, response['anomalies'][0]['recovered'])

  def testRecovered_Invalid(self):
    self._CreateAnomaly()
    self._CreateAnomaly(recovered=True)
    with self.assertRaises(Exception):
      self._Post(recovered='invalid')

  def testLimit(self):
    self._CreateAnomaly()
    self._CreateAnomaly()
    response = self._Post(limit=1)
    self.assertEqual(1, len(response['anomalies']))

  def testSheriff(self):
    self._CreateAnomaly(sheriff_name='Chromium Perf Sheriff', start_revision=42)
    self._CreateAnomaly(sheriff_name='WebRTC Perf Sheriff')
    response = self._Post(sheriff='Chromium Perf Sheriff')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(42, response['anomalies'][0]['start_revision'])

  def testMaxStartRevision(self):
    self._CreateAnomaly()
    self._CreateAnomaly(start_revision=2)
    response = self._Post(max_start_revision=1)
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(0, response['anomalies'][0]['start_revision'])

  def testMinStartRevision(self):
    self._CreateAnomaly()
    self._CreateAnomaly(start_revision=2)
    response = self._Post(min_start_revision=1)
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(2, response['anomalies'][0]['start_revision'])

  def testMaxEndRevision(self):
    self._CreateAnomaly()
    self._CreateAnomaly(end_revision=200)
    response = self._Post(max_end_revision=150)
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(100, response['anomalies'][0]['end_revision'])

  def testMinEndRevision(self):
    self._CreateAnomaly()
    self._CreateAnomaly(end_revision=200)
    response = self._Post(min_end_revision=150)
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(200, response['anomalies'][0]['end_revision'])

  def testMaxTimestamp(self):
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(59))
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(61))
    response = self._Post(max_timestamp='1970-1-1T0:1:0.000001')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual('1970-01-01T00:00:59',
                     response['anomalies'][0]['timestamp'])

  def testMinTimestamp(self):
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(59))
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(61))
    response = self._Post(min_timestamp='1970-1-1T0:1:0')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual('1970-01-01T00:01:01',
                     response['anomalies'][0]['timestamp'])

  def testAllInequalityFilters(self):
    matching_start_revision = 15
    matching_end_revision = 35
    matching_timestamp = datetime.datetime.utcfromtimestamp(120)
    self._CreateAnomaly(
        start_revision=9,
        end_revision=matching_end_revision,
        timestamp=matching_timestamp)
    self._CreateAnomaly(
        start_revision=21,
        end_revision=matching_end_revision,
        timestamp=matching_timestamp)
    self._CreateAnomaly(
        start_revision=matching_start_revision,
        end_revision=29,
        timestamp=matching_timestamp)
    self._CreateAnomaly(
        start_revision=matching_start_revision,
        end_revision=41,
        timestamp=matching_timestamp)
    self._CreateAnomaly(
        start_revision=matching_start_revision,
        end_revision=matching_end_revision,
        timestamp=datetime.datetime.utcfromtimestamp(181))
    self._CreateAnomaly(
        start_revision=matching_start_revision,
        end_revision=matching_end_revision,
        timestamp=datetime.datetime.utcfromtimestamp(59))
    self._CreateAnomaly(
        start_revision=matching_start_revision,
        end_revision=matching_end_revision,
        timestamp=matching_timestamp)
    response = self._Post(
        min_start_revision=10,
        max_start_revision=20,
        min_end_revision=30,
        max_end_revision=40,
        min_timestamp='1970-1-1T0:1:0',
        max_timestamp='1970-1-1T0:3:0')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(matching_start_revision,
                     response['anomalies'][0]['start_revision'])
    self.assertEqual(matching_end_revision,
                     response['anomalies'][0]['end_revision'])
    self.assertEqual(matching_timestamp.isoformat(),
                     response['anomalies'][0]['timestamp'])

  def testUntilFound(self):
    # inequality_property defaults to the first of 'start_revision',
    # 'end_revision', 'timestamp' that is filtered.
    # Filter by start_revision and timestamp so that timestamp will be filtered
    # post-hoc. Set limit so that the first few queries return alerts that match
    # the start_revision filter but not the post_filter, so that
    # QueryAnomaliesUntilFound must automatically chase cursors until it finds
    # some results.
    matching_timestamp = datetime.datetime.utcfromtimestamp(60)
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(100))
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(90))
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(80))
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(70))
    self._CreateAnomaly(timestamp=matching_timestamp)
    response = self._Post(
        limit=1, min_start_revision=0, max_timestamp='1970-1-1T0:1:0')
    self.assertEqual(1, len(response['anomalies']))
    self.assertEqual(matching_timestamp.isoformat(),
                     response['anomalies'][0]['timestamp'])


if __name__ == '__main__':
  unittest.main()
