# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import Flask
import unittest
import webtest

from dashboard import migrate_test_names
from dashboard import migrate_test_names_tasks
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.models import graph_data
from dashboard.models import histogram
from tracing.value.diagnostics import generic_set

# Masters, bots and test names to add to the mock datastore.
_MOCK_DATA = [['ChromiumPerf'], ['win7', 'mac'], {
    'SunSpider': {
        'Total': {
            't': {},
            't_ref': {},
            't_extwr': {},
        },
        '3d-cube': {
            't': {}
        },
    },
    'moz': {
        'read_op_b': {
            'r_op_b': {}
        },
    },
}]

flask_app = Flask(__name__)


@flask_app.route('/migrate_test_names', methods=['GET'])
def MigrateTestNamesGet():
  return migrate_test_names.MigrateTestNamesGet()


@flask_app.route('/migrate_test_names', methods=['POST'])
def MigrateTestNamesPost():
  return migrate_test_names.MigrateTestNamesPost()


@flask_app.route('/migrate_test_names_tasks', methods=['POST'])
def MigrateTestNamesTasksPost():
  return migrate_test_names_tasks.MigrateTestNamesTasksPost()


class MigrateTestNamesTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.testapp = webtest.TestApp(flask_app)
    # Make sure puts get split up into multiple calls.
    migrate_test_names_tasks._MAX_DATASTORE_PUTS_PER_PUT_MULTI_CALL = 30
    self.SetCurrentUser('internal@foo.bar')
    testing_common.SetIsInternalUser('internal@foo.bar', True)
    self.SetUserGroupMembership('internal@foo.bar',
                                migrate_test_names._ACCESS_GROUP_NAME, True)

  def _AddMockData(self):
    """Adds sample TestMetadata, Row, and Anomaly entities."""
    testing_common.AddTests(*_MOCK_DATA)

    # Add 50 Row entities to one of the tests.
    # Also add 2 Anomaly entities.
    test_path = 'ChromiumPerf/mac/SunSpider/Total/t'
    test_key = utils.TestKey(test_path)
    test_container_key = utils.GetTestContainerKey(test_key)
    for rev in range(15000, 15100, 2):
      graph_data.Row(id=rev, parent=test_container_key, value=(rev * 2)).put()
      if rev % 50 == 0:
        data = generic_set.GenericSet(['foo_%s' % rev])
        data = data.AsDict()
        anomaly.Anomaly(
            start_revision=(rev - 2),
            end_revision=rev,
            median_before_anomaly=100,
            median_after_anomaly=50,
            test=test_key).put()
        histogram.SparseDiagnostic(
            test=test_key,
            start_revision=rev - 50,
            end_revision=rev - 1,
            data=data).put()
        histogram.Histogram(test=test_key).put()

  def _CheckRows(self, test_path, multiplier=2):
    """Checks the rows match the expected sample data for a given test.

    The expected revisions that should be present are based on the sample data
    added in _AddMockData.

    Args:
      test_path: Test path of the test to get rows for.
      multiplier: Number to multiply with revision to get expected value.
    """
    rows = graph_data.Row.query(
        graph_data.Row.parent_test == utils.OldStyleTestKey(test_path)).fetch()
    self.assertEqual(50, len(rows))
    self.assertEqual(15000, rows[0].revision)
    self.assertEqual(15000 * multiplier, rows[0].value)
    self.assertEqual(15098, rows[49].revision)
    self.assertEqual(15098 * multiplier, rows[49].value)

    t = utils.TestKey(test_path).get()
    self.assertTrue(t.has_rows)

  def _CheckAnomalies(self, test_path, r1=15000, r2=15050):
    """Checks whether the anomalies match the ones added in _AddMockData.

    Args:
      test_path: The test path for the TestMetadata which the Anomalies are on.
      r1: Expected end revision of first Anomaly.
      r2: Expected end revision of second Anomaly.
    """
    key = utils.TestKey(test_path)
    anomalies = anomaly.Anomaly.query(anomaly.Anomaly.test == key).fetch()
    self.assertEqual(2, len(anomalies))
    self.assertEqual(r1, anomalies[0].end_revision)
    self.assertEqual(r2, anomalies[1].end_revision)

  def _CheckHistogramData(self, test_path):
    diagnostics = histogram.SparseDiagnostic.query(
        histogram.SparseDiagnostic.test == utils.TestKey(test_path)).fetch()
    self.assertEqual(2, len(diagnostics))

    histograms = histogram.SparseDiagnostic.query(
        histogram.Histogram.test == utils.TestKey(test_path)).fetch()
    self.assertEqual(2, len(histograms))

  def _CheckTests(self, expected_tests):
    """Checks whether the current TestMetadata entities match the expected list.

    Args:
      expected_tests: List of test paths without the master/bot part.
    """
    for master in _MOCK_DATA[0]:
      for bot in _MOCK_DATA[1]:
        expected = ['%s/%s/%s' % (master, bot, t) for t in expected_tests]
        tests = graph_data.TestMetadata.query(
            graph_data.TestMetadata.master_name == master,
            graph_data.TestMetadata.bot_name == bot).fetch()
        actual = [t.test_path for t in tests]
        self.assertEqual(expected, actual)

  def testPost_MigrateTraceLevelTest(self):
    self._AddMockData()
    self.testapp.post('/migrate_test_names', {
        'old_pattern': '*/*/*/*/t',
        'new_pattern': '*/*/*/*/time',
    })
    self.ExecuteTaskQueueTasks('/migrate_test_names_tasks',
                               migrate_test_names_tasks._TASK_QUEUE_NAME)
    expected_tests = [
        'SunSpider',
        'SunSpider/3d-cube',
        'SunSpider/3d-cube/time',
        'SunSpider/Total',
        'SunSpider/Total/t_extwr',
        'SunSpider/Total/t_ref',
        'SunSpider/Total/time',
        'moz',
        'moz/read_op_b',
        'moz/read_op_b/r_op_b',
    ]
    self._CheckTests(expected_tests)

  def testPost_MigrateSkippingIdentityMigrations(self):
    self._AddMockData()
    # Matches both t_ref and t_extwr, but only t_extwr gets migrated and
    # t_ref is left untouched (otherwise t_ref would be deleted!).
    self.testapp.post('/migrate_test_names', {
        'old_pattern': '*/*/*/*/t_*',
        'new_pattern': '*/*/*/*/t_ref',
    })
    self.ExecuteTaskQueueTasks('/migrate_test_names_tasks',
                               migrate_test_names_tasks._TASK_QUEUE_NAME)
    expected_tests = [
        'SunSpider',
        'SunSpider/3d-cube',
        'SunSpider/3d-cube/t',
        'SunSpider/Total',
        'SunSpider/Total/t',
        'SunSpider/Total/t_ref',
        'moz',
        'moz/read_op_b',
        'moz/read_op_b/r_op_b',
    ]
    self._CheckTests(expected_tests)

  def testPost_RenameTraceWithPartialWildCardsInNewPattern_Fails(self):
    # If there's a wildcard in a part of the new pattern, it should
    # just be a single wildcard by itself, and it just means "copy
    # over whatever was in the old test path". Wildcards mixed with
    # substrings should be rejected.
    self._AddMockData()
    self.testapp.post(
        '/migrate_test_names', {
            'old_pattern': '*/*/Sun*/*/t',
            'new_pattern': '*/*/Sun*/*/time',
        },
        status=400)
    self.ExecuteTaskQueueTasks('/migrate_test_names_tasks',
                               migrate_test_names_tasks._TASK_QUEUE_NAME)
    # Nothing was renamed since there was an error.
    expected_tests = [
        'SunSpider',
        'SunSpider/3d-cube',
        'SunSpider/3d-cube/t',
        'SunSpider/Total',
        'SunSpider/Total/t',
        'SunSpider/Total/t_extwr',
        'SunSpider/Total/t_ref',
        'moz',
        'moz/read_op_b',
        'moz/read_op_b/r_op_b',
    ]
    self._CheckTests(expected_tests)

  def testPost_MigrateChartLevelTest(self):
    self._AddMockData()

    self.testapp.post(
        '/migrate_test_names', {
            'old_pattern': '*/*/SunSpider/Total',
            'new_pattern': '*/*/SunSpider/OverallScore',
        })
    self.ExecuteTaskQueueTasks('/migrate_test_names_tasks',
                               migrate_test_names_tasks._TASK_QUEUE_NAME)

    self._CheckRows('ChromiumPerf/mac/SunSpider/OverallScore/t')
    self._CheckAnomalies('ChromiumPerf/mac/SunSpider/OverallScore/t')
    self._CheckHistogramData('ChromiumPerf/mac/SunSpider/OverallScore/t')
    expected_tests = [
        'SunSpider',
        'SunSpider/3d-cube',
        'SunSpider/3d-cube/t',
        'SunSpider/OverallScore',
        'SunSpider/OverallScore/t',
        'SunSpider/OverallScore/t_extwr',
        'SunSpider/OverallScore/t_ref',
        'moz',
        'moz/read_op_b',
        'moz/read_op_b/r_op_b',
    ]
    self._CheckTests(expected_tests)

  def testPost_MigrateSuiteLevelTest(self):
    self._AddMockData()

    self.testapp.post('/migrate_test_names', {
        'old_pattern': '*/*/SunSpider',
        'new_pattern': '*/*/SunSpider1.0',
    })
    self.ExecuteTaskQueueTasks('/migrate_test_names_tasks',
                               migrate_test_names_tasks._TASK_QUEUE_NAME)

    self._CheckRows('ChromiumPerf/mac/SunSpider1.0/Total/t')
    self._CheckAnomalies('ChromiumPerf/mac/SunSpider1.0/Total/t')
    self._CheckHistogramData('ChromiumPerf/mac/SunSpider1.0/Total/t')
    expected_tests = [
        'SunSpider1.0',
        'SunSpider1.0/3d-cube',
        'SunSpider1.0/3d-cube/t',
        'SunSpider1.0/Total',
        'SunSpider1.0/Total/t',
        'SunSpider1.0/Total/t_extwr',
        'SunSpider1.0/Total/t_ref',
        'moz',
        'moz/read_op_b',
        'moz/read_op_b/r_op_b',
    ]
    self._CheckTests(expected_tests)

  def testPost_MigrateSeriesToChartLevelTest(self):
    self._AddMockData()

    self.testapp.post(
        '/migrate_test_names', {
            'old_pattern': '*/*/SunSpider/Total/t',
            'new_pattern': '*/*/SunSpider/Total',
        })
    self.ExecuteTaskQueueTasks('/migrate_test_names_tasks',
                               migrate_test_names_tasks._TASK_QUEUE_NAME)

    # The Row and Anomaly entities have been moved.
    self._CheckRows('ChromiumPerf/mac/SunSpider/Total')
    self._CheckAnomalies('ChromiumPerf/mac/SunSpider/Total')
    self._CheckHistogramData('ChromiumPerf/mac/SunSpider/Total')

    # There is no SunSpider/Total/time any more.
    expected_tests = [
        'SunSpider',
        'SunSpider/3d-cube',
        'SunSpider/3d-cube/t',
        'SunSpider/Total',
        'SunSpider/Total/t_extwr',
        'SunSpider/Total/t_ref',
        'moz',
        'moz/read_op_b',
        'moz/read_op_b/r_op_b',
    ]
    self._CheckTests(expected_tests)

  def testPost_MigrationFinished_EmailsSheriff(self):
    self._AddMockData()

    # Add a sheriff for one test.
    test_path = 'ChromiumPerf/mac/moz/read_op_b/r_op_b'
    test = utils.TestKey(test_path).get()
    test.put()

    # Add another sheriff for another test.
    test_path = 'ChromiumPerf/win7/moz/read_op_b/r_op_b'
    test = utils.TestKey(test_path).get()
    test.put()

    # Make a request to t migrate a test and then execute tasks on the queue.
    self.testapp.post(
        '/migrate_test_names', {
            'old_pattern': '*/*/moz/read_op_b/r_op_b',
            'new_pattern': '*/*/moz/read_operations_browser',
        })
    self.ExecuteTaskQueueTasks('/migrate_test_names_tasks',
                               migrate_test_names_tasks._TASK_QUEUE_NAME)

    # Check the emails that were sent.
    messages = self.mail_stub.get_sent_messages()
    self.assertEqual(2, len(messages))
    self.assertEqual('gasper-alerts@google.com', messages[0].sender)
    self.assertEqual('browser-perf-engprod@google.com',
                     messages[0].to)
    self.assertEqual('Sheriffed Test Migrated', messages[0].subject)
    body = str(messages[0].html)
    self.assertIn(
        'ChromiumPerf/mac/moz/read_op_b/r_op_b -> ', body)
    self.assertIn('-> <i>ChromiumPerf/mac/moz/read_operations_browser</i>',
                  body)
    self.assertEqual('gasper-alerts@google.com', messages[1].sender)
    self.assertEqual('browser-perf-engprod@google.com',
                     messages[1].to)
    self.assertEqual('Sheriffed Test Migrated', messages[1].subject)
    body = str(messages[1].html)
    self.assertIn(
        'ChromiumPerf/win7/moz/read_op_b/r_op_b ->', body)
    self.assertIn('-> <i>ChromiumPerf/win7/moz/read_operations_browser</i>',
                  body)

  def testGetNewTestPath_WithAsterisks(self):
    self.assertEqual(
        'A/b/c/X',
        migrate_test_names_tasks._ValidateAndGetNewTestPath('A/b/c/d',
            '*/*/*/X'))
    self.assertEqual(
        'A/b/c/d',
        migrate_test_names_tasks._ValidateAndGetNewTestPath('A/b/c/d',
            '*/*/*/*'))
    self.assertEqual(
        'A/b/c',
        migrate_test_names_tasks._ValidateAndGetNewTestPath('A/b/c/d',
            '*/*/*'))

  def testGetNewTestPath_WithBrackets(self):
    # Brackets are just used to delete parts of names, no other functionality.
    self.assertEqual(
        'A/b/c/x',
        migrate_test_names_tasks._ValidateAndGetNewTestPath('A/b/c/xxxx',
                                                      '*/*/*/[xxx]'))
    self.assertEqual(
        'A/b/c',
        migrate_test_names_tasks._ValidateAndGetNewTestPath('A/b/c/xxxx',
                                                      '*/*/*/[xxxx]'))
    self.assertEqual(
        'A/b/c/x',
        migrate_test_names_tasks._ValidateAndGetNewTestPath('A/b/c/x',
            '*/*/*/[]'))
    self.assertEqual(
        'A/b/c/d',
        migrate_test_names_tasks._ValidateAndGetNewTestPath('AA/bb/cc/dd',
                                                      '[A]/[b]/[c]/[d]'))

  def testGetNewTestPath_NewPathHasDifferentLength(self):
    self.assertEqual(
        'A/b/c',
        migrate_test_names_tasks._ValidateAndGetNewTestPath('A/b/c/d',
            'A/*/c'))
    self.assertEqual(
        'A/b/c/d',
        migrate_test_names_tasks._ValidateAndGetNewTestPath('A/b/c',
            'A/*/c/d'))
    self.assertRaises(migrate_test_names_tasks.BadInputPatternError,
                      migrate_test_names_tasks._ValidateAndGetNewTestPath,
                      'A/b/c', 'A/b/c/*')

  def testGetNewTestPath_InvalidArgs(self):
    self.assertRaises(AssertionError,
                      migrate_test_names_tasks._ValidateAndGetNewTestPath,
                      'A/b/*/d', 'A/b/c/d')
    self.assertRaises(migrate_test_names_tasks.BadInputPatternError,
                      migrate_test_names_tasks._ValidateAndGetNewTestPath,
                      'A/b/c/d', 'A/b/c/d*')

  def testGet_UnauthorizedAccess(self):
    self.SetUserGroupMembership('internal@foo.bar',
                                migrate_test_names._ACCESS_GROUP_NAME, False)
    response = self.testapp.get('/migrate_test_names')
    self.assertIsNotNone(response)
    self.assertIn('Unauthorized', response)

  def testPost_UnauthorizedAccess(self):
    self.SetUserGroupMembership('internal@foo.bar',
                                migrate_test_names._ACCESS_GROUP_NAME, False)

    # Expect a 401 response for post calls
    response = self.testapp.post('/migrate_test_names', status=401)
    self.assertIsNotNone(response)

  def testGet_no_user(self):
    self.SetCurrentUser("")
    response = self.testapp.get('/migrate_test_names')
    self.assertIsNotNone(response)
    self.assertIn('Unauthorized', response)


if __name__ == '__main__':
  unittest.main()
