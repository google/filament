# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
import unittest

from google.appengine.ext import ndb

from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.models.subscription import Subscription


class AnomalyTest(testing_common.TestCase):
  """Test case for some functions in anomaly."""

  def testGetTestMetadataKey_Test(self):
    a = anomaly.Anomaly(
        test=ndb.Key('Master', 'm', 'Bot', 'b', 'Test', 't', 'Test', 't'))
    k = a.GetTestMetadataKey()
    self.assertEqual('TestMetadata', k.kind())
    self.assertEqual('m/b/t/t', k.id())
    self.assertEqual('m/b/t/t', utils.TestPath(k))

  def testGetTestMetadataKey_TestMetadata(self):
    a = anomaly.Anomaly(test=utils.TestKey('a/b/c/d'))
    k = a.GetTestMetadataKey()
    self.assertEqual('TestMetadata', k.kind())
    self.assertEqual('a/b/c/d', k.id())
    self.assertEqual('a/b/c/d', utils.TestPath(k))

  def testGetTestMetadataKey_None(self):
    a = anomaly.Anomaly()
    k = a.GetTestMetadataKey()
    self.assertIsNone(k)

  def testGetAnomaliesForTest(self):
    old_style_key1 = utils.OldStyleTestKey('master/bot/test1/metric')
    new_style_key1 = utils.TestMetadataKey('master/bot/test1/metric')
    old_style_key2 = utils.OldStyleTestKey('master/bot/test2/metric')
    new_style_key2 = utils.TestMetadataKey('master/bot/test2/metric')
    anomaly.Anomaly(id="old_1", test=old_style_key1).put()
    anomaly.Anomaly(id="old_1a", test=old_style_key1).put()
    anomaly.Anomaly(id="old_2", test=old_style_key2).put()
    anomaly.Anomaly(id="new_1", test=new_style_key1).put()
    anomaly.Anomaly(id="new_2", test=new_style_key2).put()
    anomaly.Anomaly(id="new_2a", test=new_style_key2).put()
    key1_alerts, _, _ = anomaly.Anomaly.QueryAsync(
        test=new_style_key1).get_result()
    self.assertEqual(['new_1', 'old_1', 'old_1a'],
                     [a.key.id() for a in key1_alerts])
    key2_alerts, _, _ = anomaly.Anomaly.QueryAsync(
        test=old_style_key2).get_result()
    self.assertEqual(['new_2', 'new_2a', 'old_2'],
                     [a.key.id() for a in key2_alerts])
    key2_alerts_limit, _, _ = anomaly.Anomaly.QueryAsync(
        test=old_style_key2, limit=2).get_result()
    self.assertEqual(['new_2', 'new_2a'],
                     [a.key.id() for a in key2_alerts_limit])

  def testComputedTestProperties(self):
    anomaly.Anomaly(
        id="foo", test=utils.TestKey('master/bot/benchmark/metric/page')).put()
    a = ndb.Key('Anomaly', 'foo').get()
    self.assertEqual(a.master_name, 'master')
    self.assertEqual(a.bot_name, 'bot')
    self.assertEqual(a.benchmark_name, 'benchmark')

  def _CreateAnomaly(self,
                     timestamp=None,
                     bug_id=None,
                     project_id=None,
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
    if timestamp:
      entity.timestamp = timestamp
    entity.bug_id = bug_id
    entity.project_id = project_id
    if sheriff_name:
      entity.subscription_names.append(sheriff_name)
      entity.subscriptions.append(
          Subscription(
              name=sheriff_name, notification_email='sullivan@google.com'))
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

  def testKey(self):
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        key=self._CreateAnomaly()).get_result()
    self.assertEqual(1, len(anomalies))

  def testBot(self):
    self._CreateAnomaly()
    self._CreateAnomaly(test='adept/android/lodging/assessment/story')
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        bot_name='android').get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual('android', anomalies[0].bot_name)

  def testMaster(self):
    self._CreateAnomaly()
    self._CreateAnomaly(test='adept/android/lodging/assessment/story')
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        master_name='adept').get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual('adept', anomalies[0].master_name)

  def testTestSuite(self):
    self._CreateAnomaly()
    self._CreateAnomaly(test='adept/android/lodging/assessment/story')
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        test_suite_name='lodging').get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual('adept/android/lodging/assessment/story',
                     anomalies[0].test.id())

  def testTest(self):
    self._CreateAnomaly()
    self._CreateAnomaly(test='adept/android/lodging/assessment/story')
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        test='adept/android/lodging/assessment/story').get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual('adept/android/lodging/assessment/story',
                     anomalies[0].test.id())

  def testTestKeys(self):
    self._CreateAnomaly()
    test_path = 'adept/android/lodging/assessment/story'
    self._CreateAnomaly(test=test_path)
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        test_keys=[utils.TestMetadataKey(test_path)]).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(test_path, anomalies[0].test.id())

  def testBugId(self):
    self._CreateAnomaly()
    self._CreateAnomaly(bug_id=42)
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(bug_id=42).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(42, anomalies[0].bug_id)

    anomalies, _, _ = anomaly.Anomaly.QueryAsync(bug_id='').get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(None, anomalies[0].bug_id)

    anomalies, _, _ = anomaly.Anomaly.QueryAsync(bug_id='*').get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(42, anomalies[0].bug_id)

  def testProjectId(self):
    self._CreateAnomaly(project_id='')
    self._CreateAnomaly(project_id='test_project')
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(project_id='').get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual('', anomalies[0].project_id)

    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        project_id='chromium').get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual('', anomalies[0].project_id)

    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        project_id='test_project').get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual('test_project', anomalies[0].project_id)

  def testIsImprovement(self):
    self._CreateAnomaly()
    self._CreateAnomaly(is_improvement=True)
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        is_improvement=True).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(True, anomalies[0].is_improvement)

    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        is_improvement=False).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(False, anomalies[0].is_improvement)

  def testRecovered(self):
    self._CreateAnomaly()
    self._CreateAnomaly(recovered=True)
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(recovered=True).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(True, anomalies[0].recovered)

    anomalies, _, _ = anomaly.Anomaly.QueryAsync(recovered=False).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(False, anomalies[0].recovered)

  def testLimit(self):
    self._CreateAnomaly()
    self._CreateAnomaly()
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(limit=1).get_result()
    self.assertEqual(1, len(anomalies))

  def testSheriff(self):
    self._CreateAnomaly(sheriff_name='Chromium Perf Sheriff', start_revision=42)
    self._CreateAnomaly(sheriff_name='WebRTC Perf Sheriff')
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        subscriptions=['Chromium Perf Sheriff']).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(42, anomalies[0].start_revision)

  def testMaxStartRevision(self):
    self._CreateAnomaly()
    self._CreateAnomaly(start_revision=2)
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        max_start_revision=1).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(0, anomalies[0].start_revision)

  def testMinStartRevision(self):
    self._CreateAnomaly()
    self._CreateAnomaly(start_revision=2)
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        min_start_revision=1).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(2, anomalies[0].start_revision)

  def testMaxEndRevision(self):
    self._CreateAnomaly()
    self._CreateAnomaly(end_revision=200)
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        max_end_revision=150).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(100, anomalies[0].end_revision)

  def testMinEndRevision(self):
    self._CreateAnomaly()
    self._CreateAnomaly(end_revision=200)
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        min_end_revision=150).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(200, anomalies[0].end_revision)

  def testMinAndMaxRevisionAreSame(self):
    self._CreateAnomaly()
    self._CreateAnomaly(end_revision=200)
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        min_end_revision=200, max_start_revision=200).get_result()
    self.assertGreaterEqual(1, len(anomalies))
    self.assertEqual(200, anomalies[0].end_revision)

  def testMaxTimestamp(self):
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(59))
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(61))
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        max_timestamp=datetime.datetime.utcfromtimestamp(60)).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(
        datetime.datetime.utcfromtimestamp(59), anomalies[0].timestamp)

  def testMinTimestamp(self):
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(59))
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(61))
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        min_timestamp=datetime.datetime.utcfromtimestamp(60)).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(
        datetime.datetime.utcfromtimestamp(61), anomalies[0].timestamp)

  def testInequalityWithTestKeys(self):
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(59))
    self._CreateAnomaly(timestamp=datetime.datetime.utcfromtimestamp(61))
    self._CreateAnomaly(
        timestamp=datetime.datetime.utcfromtimestamp(61),
        test='master/bot/test_suite/measurement/test_case2')
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        test='master/bot/test_suite/measurement/test_case2',
        min_timestamp=datetime.datetime.utcfromtimestamp(60)).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(
        datetime.datetime.utcfromtimestamp(61), anomalies[0].timestamp)

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
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        min_start_revision=10,
        max_start_revision=20,
        min_end_revision=30,
        max_end_revision=40,
        min_timestamp=datetime.datetime.utcfromtimestamp(60),
        max_timestamp=datetime.datetime.utcfromtimestamp(180)).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(matching_start_revision, anomalies[0].start_revision)
    self.assertEqual(matching_end_revision, anomalies[0].end_revision)
    self.assertEqual(matching_timestamp, anomalies[0].timestamp)

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
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        limit=1, min_start_revision=0,
        max_timestamp=matching_timestamp).get_result()
    self.assertEqual(1, len(anomalies))
    self.assertEqual(matching_timestamp, anomalies[0].timestamp)


if __name__ == '__main__':
  unittest.main()
