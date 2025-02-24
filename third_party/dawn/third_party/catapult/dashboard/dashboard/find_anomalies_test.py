# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import sys
import unittest

from unittest import mock

from google.appengine.ext import ndb

from dashboard import find_anomalies
from dashboard import find_change_points
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import alert_group
from dashboard.models import anomaly
from dashboard.models import graph_data
from dashboard.models import histogram
from dashboard.models.subscription import Subscription
from dashboard.models.subscription import VISIBILITY
from dashboard.models.subscription import AnomalyConfig
from dashboard.sheriff_config_client import SheriffConfigClient
from tracing.value.diagnostics import reserved_infos

# pylint: disable=too-many-lines

# Sample time series.
_TEST_ROW_DATA = [
    (241105, 2136.7),
    (241116, 2140.3),
    (241151, 2149.1),
    (241154, 2147.2),
    (241156, 2130.6),
    (241160, 2136.2),
    (241188, 2146.7),
    (241201, 2141.8),
    (241226, 2140.6),
    (241247, 2128.1),
    (241249, 2134.2),
    (241254, 2130.0),
    (241262, 2136.0),
    (241268, 2142.6),
    (241271, 2149.1),
    (241282, 2156.6),
    (241294, 2125.3),
    (241298, 2155.5),
    (241303, 2148.5),
    (241317, 2146.2),
    (241323, 2123.3),
    (241330, 2121.5),
    (241342, 2141.2),
    (241355, 2145.2),
    (241371, 2136.3),
    (241386, 2144.0),
    (241405, 2138.1),
    (241420, 2147.6),
    (241432, 2140.7),
    (241441, 2132.2),
    (241452, 2138.2),
    (241455, 2139.3),
    (241471, 2134.0),
    (241488, 2137.2),
    (241503, 2152.5),
    (241520, 2136.3),
    (241524, 2139.3),
    (241529, 2143.5),
    (241532, 2145.5),
    (241535, 2147.0),
    (241537, 2484.1),
    (241546, 2480.8),
    (241553, 2481.5),
    (241559, 2476.8),
    (241566, 2474.0),
    (241577, 2482.8),
    (241579, 2484.8),
    (241582, 2490.5),
    (241584, 2483.1),
    (241609, 2478.3),
    (241620, 2478.1),
    (241645, 2490.8),
    (241653, 2477.7),
    (241666, 2485.3),
    (241697, 2473.8),
    (241716, 2472.1),
    (241735, 2472.5),
    (241757, 2474.7),
    (241766, 2496.7),
    (241782, 2484.1),
]


def _MakeSampleChangePoint(x_value, median_before, median_after):
  """Makes a sample find_change_points.ChangePoint for use in these tests."""
  # The only thing that matters in these tests is the revision number
  # and the values before and after.
  return find_change_points.ChangePoint(
      x_value=x_value,
      median_before=median_before,
      median_after=median_after,
      window_start=1,
      window_end=8,
      size_before=None,
      size_after=None,
      relative_change=None,
      std_dev_before=None,
      t_statistic=None,
      degrees_of_freedom=None,
      p_value=None,
      extended_start=x_value,
      extended_end=x_value,
  )


class EndRevisionMatcher:
  """Custom matcher to test if an anomaly matches a given end rev."""

  def __init__(self, end_revision):
    """Initializes with the end time to check."""
    self._end_revision = end_revision

  def __eq__(self, rhs):
    """Checks to see if RHS has the same end time."""
    return self._end_revision == rhs.end_revision

  def __repr__(self):
    """Shows a readable revision which can be printed when assert fails."""
    return '<IsEndRevision %d>' % self._end_revision

  def __hash__(self):
    return hash(self._end_revision)


class ModelMatcher:
  """Custom matcher to check if two ndb entity names match."""

  def __init__(self, name):
    """Initializes with the name of the entity."""
    self._name = name

  def __eq__(self, rhs):
    """Checks to see if RHS has the same name."""
    return (rhs.key.string_id() if rhs.key else rhs.name) == self._name

  def __repr__(self):
    """Shows a readable revision which can be printed when assert fails."""
    return '<IsModel %s>' % self._name

  def __hash__(self):
    return hash(self._name)


@ndb.tasklet
def _MockTasklet(*_):
  raise ndb.Return(None)


@mock.patch.object(SheriffConfigClient, '__init__',
                   mock.MagicMock(return_value=None))
class ProcessAlertsTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.SetCurrentUser('foo@bar.com', is_admin=True)

  def _AddDataForTests(self, stats=None, masters=None):
    if not masters:
      masters = ['ChromiumGPU']
    testing_common.AddTests(masters, ['linux-release'], {
        'scrolling_benchmark': {
            'ref': {},
        },
    })

    for m in masters:
      ref = utils.TestKey('%s/linux-release/scrolling_benchmark/ref' % m).get()
      ref.units = 'ms'
      for i in range(9000, 10070, 5):
        # Internal-only data should be found.
        test_container_key = utils.GetTestContainerKey(ref.key)
        r = graph_data.Row(
            id=i + 1,
            value=float(i * 3),
            parent=test_container_key,
            internal_only=True)
        if stats:
          for s in stats:
            setattr(r, s, i)
        r.put()

  def _DataSeries(self):
    return [(r.revision, r, r.value) for r in list(graph_data.Row.query())]

  def _GetGroupsForAnomalyMock(self, anomaly_entity):
    group_keys = alert_group.AlertGroup.GetGroupsForAnomaly(
        anomaly_entity, None)
    group_ids = [k.string_id() for k in group_keys]
    return group_ids

  @mock.patch.object(find_anomalies.find_change_points, 'FindChangePoints',
                     mock.MagicMock(return_value=[
                         _MakeSampleChangePoint(10011, 50, 100),
                         _MakeSampleChangePoint(10041, 200, 100),
                         _MakeSampleChangePoint(10061, 0, 100),
                     ]))
  @mock.patch.object(find_anomalies.email_sheriff, 'EmailSheriff')
  def testProcessTest(self, mock_email_sheriff):
    perf_comment_post_patcher = mock.patch(
        'dashboard.services.perf_issue_service_client.GetAlertGroupsForAnomaly',
        self._GetGroupsForAnomalyMock)
    perf_comment_post_patcher.start()
    self.addCleanup(perf_comment_post_patcher.stop)

    self._AddDataForTests()
    test_path = 'ChromiumGPU/linux-release/scrolling_benchmark/ref'
    test = utils.TestKey(test_path).get()
    test.UpdateSheriff()
    test.put()

    alert_group_key1 = alert_group.AlertGroup(
        id='group_id_1',
        name='scrolling_benchmark',
        domain='ChromiumGPU',
        subscription_name='sheriff1',
        status=alert_group.AlertGroup.Status.untriaged,
        active=True,
        revision=alert_group.RevisionRange(
            repository='chromium', start=10000, end=10070),
    ).put()
    alert_group_key2 = alert_group.AlertGroup(
        id='group_id_2',
        name='scrolling_benchmark',
        domain='ChromiumGPU',
        subscription_name='sheriff2',
        status=alert_group.AlertGroup.Status.untriaged,
        active=True,
        revision=alert_group.RevisionRange(
            repository='chromium', start=10000, end=10070),
    ).put()

    s1 = Subscription(name='sheriff1', visibility=VISIBILITY.PUBLIC)
    s2 = Subscription(name='sheriff2', visibility=VISIBILITY.PUBLIC)
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s1, s2], None))) as m:
      find_anomalies.ProcessTests([test.key])
      self.assertEqual(m.call_args_list[0], mock.call(test.key.id()))
    self.ExecuteDeferredTasks('default')

    expected_calls = [
        mock.call(
            [ModelMatcher('sheriff1')],
            ModelMatcher('ChromiumGPU/linux-release/scrolling_benchmark/ref'),
            EndRevisionMatcher(10011)),
        mock.call(
            [ModelMatcher('sheriff1')],
            ModelMatcher('ChromiumGPU/linux-release/scrolling_benchmark/ref'),
            EndRevisionMatcher(10041)),
        mock.call(
            [ModelMatcher('sheriff1')],
            ModelMatcher('ChromiumGPU/linux-release/scrolling_benchmark/ref'),
            EndRevisionMatcher(10061)),
        mock.call(
            [ModelMatcher('sheriff2')],
            ModelMatcher('ChromiumGPU/linux-release/scrolling_benchmark/ref'),
            EndRevisionMatcher(10011)),
        mock.call(
            [ModelMatcher('sheriff2')],
            ModelMatcher('ChromiumGPU/linux-release/scrolling_benchmark/ref'),
            EndRevisionMatcher(10041)),
        mock.call(
            [ModelMatcher('sheriff2')],
            ModelMatcher('ChromiumGPU/linux-release/scrolling_benchmark/ref'),
            EndRevisionMatcher(10061)),
    ]
    self.assertListEqual(expected_calls, mock_email_sheriff.call_args_list)

    anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(len(anomalies), 6)
    for a in anomalies:
      self.assertEqual(a.groups, [alert_group_key1, alert_group_key2])

    def AnomalyExists(anomalies, test, percent_changed, direction,
                      start_revision, end_revision, subscription_names,
                      internal_only, units, absolute_delta, statistic):
      for a in anomalies:
        if (a.test == test and a.percent_changed == percent_changed
            and a.direction == direction and a.start_revision == start_revision
            and a.end_revision == end_revision
            and a.subscription_names == subscription_names
            and a.internal_only == internal_only and a.units == units
            and a.absolute_delta == absolute_delta
            and a.statistic == statistic):
          return True
      return False

    self.assertTrue(
        AnomalyExists(
            anomalies,
            test.key,
            percent_changed=100,
            direction=anomaly.UP,
            start_revision=10007,
            end_revision=10011,
            subscription_names=['sheriff1'],
            internal_only=False,
            units='ms',
            absolute_delta=50,
            statistic='avg'))
    self.assertTrue(
        AnomalyExists(
            anomalies,
            test.key,
            percent_changed=100,
            direction=anomaly.UP,
            start_revision=10007,
            end_revision=10011,
            subscription_names=['sheriff2'],
            internal_only=False,
            units='ms',
            absolute_delta=50,
            statistic='avg'))
    self.assertTrue(
        AnomalyExists(
            anomalies,
            test.key,
            percent_changed=-50,
            direction=anomaly.DOWN,
            start_revision=10037,
            end_revision=10041,
            subscription_names=['sheriff1'],
            internal_only=False,
            units='ms',
            absolute_delta=-100,
            statistic='avg'))
    self.assertTrue(
        AnomalyExists(
            anomalies,
            test.key,
            percent_changed=-50,
            direction=anomaly.DOWN,
            start_revision=10037,
            end_revision=10041,
            subscription_names=['sheriff2'],
            internal_only=False,
            units='ms',
            absolute_delta=-100,
            statistic='avg'))
    self.assertTrue(
        AnomalyExists(
            anomalies,
            test.key,
            percent_changed=sys.float_info.max,
            direction=anomaly.UP,
            start_revision=10057,
            end_revision=10061,
            internal_only=False,
            units='ms',
            subscription_names=['sheriff1'],
            absolute_delta=100,
            statistic='avg'))
    self.assertTrue(
        AnomalyExists(
            anomalies,
            test.key,
            percent_changed=sys.float_info.max,
            direction=anomaly.UP,
            start_revision=10057,
            end_revision=10061,
            internal_only=False,
            units='ms',
            subscription_names=['sheriff2'],
            absolute_delta=100,
            statistic='avg'))

    # This is here just to verify that AnomalyExists returns False sometimes.
    self.assertFalse(
        AnomalyExists(
            anomalies,
            test.key,
            percent_changed=100,
            direction=anomaly.DOWN,
            start_revision=10037,
            end_revision=10041,
            subscription_names=['sheriff1', 'sheriff2'],
            internal_only=False,
            units='ms',
            absolute_delta=500,
            statistic='avg'))

  @mock.patch.object(find_anomalies, '_ProcessTestStat')
  def testProcessTest_SkipsClankInternal(self, mock_process_stat):
    mock_process_stat.side_effect = _MockTasklet

    self._AddDataForTests(masters=['ClankInternal'])
    test_path = 'ClankInternal/linux-release/scrolling_benchmark/ref'
    test = utils.TestKey(test_path).get()

    a = anomaly.Anomaly(
        test=test.key,
        start_revision=10061,
        end_revision=10062,
        statistic='avg')
    a.put()

    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([], None))) as m:
      find_anomalies.ProcessTests([test.key])
      self.assertEqual(m.call_args_list, [])
    self.ExecuteDeferredTasks('default')

    self.assertFalse(mock_process_stat.called)

  @mock.patch.object(find_anomalies, '_ProcessTestStat')
  def testProcessTest_UsesLastAlert_Avg(self, mock_process_stat):
    mock_process_stat.side_effect = _MockTasklet

    self._AddDataForTests()
    test_path = 'ChromiumGPU/linux-release/scrolling_benchmark/ref'
    test = utils.TestKey(test_path).get()

    a = anomaly.Anomaly(
        test=test.key,
        start_revision=10061,
        end_revision=10062,
        statistic='avg')
    a.put()

    test.UpdateSheriff()
    test.put()

    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([], None))):
      find_anomalies.ProcessTests([test.key])
    self.ExecuteDeferredTasks('default')

    query = graph_data.Row.query(
        projection=['revision', 'timestamp', 'value', 'swarming_bot_id'])
    query = query.filter(graph_data.Row.revision > 10062)
    query = query.filter(
        graph_data.Row.parent_test == utils.OldStyleTestKey(test.key))
    row_data = query.fetch()
    rows = [(r.revision, r, r.value) for r in row_data]
    mock_process_stat.assert_called_with(mock.ANY, mock.ANY, rows, None)

    anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(len(anomalies), 1)

  @mock.patch.object(find_anomalies, '_ProcessTestStat')
  def testProcessTest_SkipsLastAlert_NotAvg(self, mock_process_stat):
    self._AddDataForTests(stats=('count',))
    test_path = 'ChromiumGPU/linux-release/scrolling_benchmark/ref'
    test = utils.TestKey(test_path).get()

    a = anomaly.Anomaly(
        test=test.key,
        start_revision=10061,
        end_revision=10062,
        statistic='count')
    a.put()

    test.UpdateSheriff()
    test.put()

    @ndb.tasklet
    def _AssertParams(test_entity, stat, rows, ref_rows):
      del test_entity
      del stat
      del ref_rows
      assert rows[0][0] < a.end_revision

    mock_process_stat.side_effect = _AssertParams
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([], None))):
      find_anomalies.ProcessTests([test.key])
    self.ExecuteDeferredTasks('default')

  @mock.patch.object(
      find_anomalies.find_change_points, 'FindChangePoints',
      mock.MagicMock(return_value=[_MakeSampleChangePoint(10011, 100, 50)]))
  def testProcessTest_ImprovementMarkedAsImprovement(self):
    self._AddDataForTests()
    test = utils.TestKey(
        'ChromiumGPU/linux-release/scrolling_benchmark/ref').get()
    test.improvement_direction = anomaly.DOWN
    test.UpdateSheriff()
    test.put()
    s = Subscription(name='sheriff', visibility=VISIBILITY.PUBLIC)
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s], None))) as m:
      find_anomalies.ProcessTests([test.key])
      self.assertEqual(m.call_args_list, [mock.call(test.key.id())])
    anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(len(anomalies), 1)
    self.assertTrue(anomalies[0].is_improvement)

  @mock.patch('logging.error')
  def testProcessTest_NoSheriff_ErrorLogged(self, mock_logging_error):
    self._AddDataForTests()
    ref = utils.TestKey(
        'ChromiumGPU/linux-release/scrolling_benchmark/ref').get()
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([], None))):
      find_anomalies.ProcessTests([ref.key])
    mock_logging_error.assert_called_with('No subscription for %s',
                                          ref.key.string_id())

  @mock.patch.object(find_anomalies.find_change_points, 'FindChangePoints',
                     mock.MagicMock(return_value=[
                         _MakeSampleChangePoint(10026, 55.2, 57.8),
                         _MakeSampleChangePoint(10041, 45.2, 37.8),
                     ]))
  @mock.patch.object(find_anomalies.email_sheriff, 'EmailSheriff')
  def testProcessTest_FiltersOutImprovements(self, mock_email_sheriff):
    self._AddDataForTests()
    test = utils.TestKey(
        'ChromiumGPU/linux-release/scrolling_benchmark/ref').get()
    test.improvement_direction = anomaly.UP
    test.UpdateSheriff()
    test.put()
    s = Subscription(name='sheriff', visibility=VISIBILITY.PUBLIC)
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s], None))) as m:
      find_anomalies.ProcessTests([test.key])
      self.assertEqual(m.call_args_list, [mock.call(test.key.id())])
    self.ExecuteDeferredTasks('default')
    mock_email_sheriff.assert_called_once_with(
        [ModelMatcher('sheriff')],
        ModelMatcher('ChromiumGPU/linux-release/scrolling_benchmark/ref'),
        EndRevisionMatcher(10041))

  @mock.patch.object(find_anomalies.find_change_points, 'FindChangePoints',
                     mock.MagicMock(return_value=[
                         _MakeSampleChangePoint(10011, 50, 100),
                     ]))
  @mock.patch.object(find_anomalies.email_sheriff, 'EmailSheriff')
  def testProcessTest_InternalOnlyTest(self, mock_email_sheriff):
    self._AddDataForTests()
    test = utils.TestKey(
        'ChromiumGPU/linux-release/scrolling_benchmark/ref').get()
    test.internal_only = True
    test.UpdateSheriff()
    test.put()

    s = Subscription(name='sheriff', visibility=VISIBILITY.PUBLIC)
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s], None))) as m:
      find_anomalies.ProcessTests([test.key])
      self.assertEqual(m.call_args_list, [mock.call(test.key.id())])
    self.ExecuteDeferredTasks('default')
    expected_calls = [
        mock.call(
            [ModelMatcher('sheriff')],
            ModelMatcher('ChromiumGPU/linux-release/scrolling_benchmark/ref'),
            EndRevisionMatcher(10011))
    ]
    self.assertEqual(expected_calls, mock_email_sheriff.call_args_list)

    anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(len(anomalies), 1)
    self.assertEqual(test.key, anomalies[0].test)
    self.assertEqual(100, anomalies[0].percent_changed)
    self.assertEqual(anomaly.UP, anomalies[0].direction)
    self.assertEqual(10007, anomalies[0].start_revision)
    self.assertEqual(10011, anomalies[0].end_revision)
    self.assertTrue(anomalies[0].internal_only)

  def testProcessTest_CreatesAnAnomaly_RefMovesToo_BenchmarkDuration(self):
    testing_common.AddTests(['ChromiumGPU'], ['linux-release'], {
        'foo': {
            'benchmark_duration': {
                'ref': {}
            }
        },
    })
    ref = utils.TestKey(
        'ChromiumGPU/linux-release/foo/benchmark_duration/ref').get()
    non_ref = utils.TestKey(
        'ChromiumGPU/linux-release/foo/benchmark_duration').get()
    test_container_key = utils.GetTestContainerKey(ref.key)
    test_container_key_non_ref = utils.GetTestContainerKey(non_ref.key)
    for row in _TEST_ROW_DATA:
      graph_data.Row(id=row[0], value=row[1], parent=test_container_key).put()
      graph_data.Row(
          id=row[0], value=row[1], parent=test_container_key_non_ref).put()
    ref.UpdateSheriff()
    ref.put()
    s = Subscription(name='sheriff', visibility=VISIBILITY.PUBLIC)
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s], None))) as m:
      find_anomalies.ProcessTests([ref.key])
      self.assertEqual(m.call_args_list, [mock.call(ref.key.id())])
    new_anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(1, len(new_anomalies))

  def testProcessTest_AnomaliesMatchRefSeries_NoAlertCreated(self):
    # Tests that a Anomaly entity is not created if both the test and its
    # corresponding ref build series have the same data.
    testing_common.AddTests(['ChromiumGPU'], ['linux-release'], {
        'scrolling_benchmark': {
            'ref': {}
        },
    })
    ref = utils.TestKey(
        'ChromiumGPU/linux-release/scrolling_benchmark/ref').get()
    non_ref = utils.TestKey(
        'ChromiumGPU/linux-release/scrolling_benchmark').get()
    test_container_key = utils.GetTestContainerKey(ref.key)
    test_container_key_non_ref = utils.GetTestContainerKey(non_ref.key)
    for row in _TEST_ROW_DATA:
      graph_data.Row(id=row[0], value=row[1], parent=test_container_key).put()
      graph_data.Row(
          id=row[0], value=row[1], parent=test_container_key_non_ref).put()
    ref.UpdateSheriff()
    ref.put()
    non_ref.UpdateSheriff()
    non_ref.put()
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([], None))):
      find_anomalies.ProcessTests([non_ref.key])
    new_anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(0, len(new_anomalies))

  def testProcessTest_AnomalyDoesNotMatchRefSeries_AlertCreated(self):
    # Tests that an Anomaly entity is created when non-ref series goes up, but
    # the ref series stays flat.
    testing_common.AddTests(['ChromiumGPU'], ['linux-release'], {
        'scrolling_benchmark': {
            'ref': {}
        },
    })
    ref = utils.TestKey(
        'ChromiumGPU/linux-release/scrolling_benchmark/ref').get()
    non_ref = utils.TestKey(
        'ChromiumGPU/linux-release/scrolling_benchmark').get()
    test_container_key = utils.GetTestContainerKey(ref.key)
    test_container_key_non_ref = utils.GetTestContainerKey(non_ref.key)
    for row in _TEST_ROW_DATA:
      graph_data.Row(id=row[0], value=2125.375, parent=test_container_key).put()
      graph_data.Row(
          id=row[0], value=row[1], parent=test_container_key_non_ref).put()
    ref.UpdateSheriff()
    ref.put()
    non_ref.UpdateSheriff()
    non_ref.put()
    s = Subscription(name='sheriff', visibility=VISIBILITY.PUBLIC)
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s], None))) as m:
      find_anomalies.ProcessTests([non_ref.key])
      self.assertEqual(m.call_args_list, [mock.call(non_ref.key.id())])
    new_anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(len(new_anomalies), 1)

  def testProcessTest_CreatesAnAnomaly(self):
    testing_common.AddTests(['ChromiumGPU'], ['linux-release'], {
        'scrolling_benchmark': {
            'ref': {}
        },
    })
    ref = utils.TestKey(
        'ChromiumGPU/linux-release/scrolling_benchmark/ref').get()
    test_container_key = utils.GetTestContainerKey(ref.key)
    for row in _TEST_ROW_DATA:
      graph_data.Row(id=row[0], value=row[1], parent=test_container_key).put()
    ref.UpdateSheriff()
    ref.put()
    s = Subscription(name='sheriff', visibility=VISIBILITY.PUBLIC)
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s], None))) as m:
      find_anomalies.ProcessTests([ref.key])
      self.assertEqual(m.call_args_list, [mock.call(ref.key.id())])
    new_anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(1, len(new_anomalies))
    self.assertEqual(anomaly.UP, new_anomalies[0].direction)
    self.assertEqual(241533, new_anomalies[0].start_revision)
    self.assertEqual(241546, new_anomalies[0].end_revision)

  def testProcessTest_BotIdBeforeAndAfterNotNoneAndDiffer_ignoreAnomaly(self):
    testing_common.AddTests(['ChromiumGPU'], ['linux-release'], {
        'scrolling_benchmark': {
            'ref': {}
        },
    })
    ref = utils.TestKey(
        'ChromiumGPU/linux-release/scrolling_benchmark/ref').get()
    test_container_key = utils.GetTestContainerKey(ref.key)
    for idx, row in enumerate(_TEST_ROW_DATA):
      graph_data.Row(
          id=row[0],
          value=row[1],
          parent=test_container_key,
          swarming_bot_id='device-id-%d' % idx).put()
    ref.UpdateSheriff()
    ref.put()
    s = Subscription(name='sheriff', visibility=VISIBILITY.PUBLIC)
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s], None))) as m:
      find_anomalies.ProcessTests([ref.key])
      self.assertEqual(m.call_args_list, [mock.call(ref.key.id())])
    new_anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(0, len(new_anomalies))

  def testProcessTest_RefineAnomalyPlacement_OffByOneBefore(self):
    testing_common.AddTests(
        ['ChromiumPerf'], ['linux-perf'],
        {'blink_perf.layout': {
            'nested-percent-height-tables': {}
        }})
    test = utils.TestKey(
        'ChromiumPerf/linux-perf/blink_perf.layout/nested-percent-height-tables'
    ).get()
    test_container_key = utils.GetTestContainerKey(test.key)
    sample_data = [
        (728446, 480.2504),
        (728462, 487.685),
        (728469, 486.6389),
        (728480, 477.6597),
        (728492, 471.2238),
        (728512, 480.4379),
        (728539, 464.5573),
        (728594, 489.0594),
        (728644, 484.4796),
        (728714, 489.5986),
        (728751, 489.474),
        (728788, 481.9336),
        (728835, 484.089),
        (728869, 485.4287),
        (728883, 476.8234),
        (728907, 487.4736),
        (728938, 490.601),
        (728986, 483.5039),
        (729021, 485.176),
        (729066, 484.5855),
        (729105, 483.9114),
        (729119, 483.559),
        (729161, 477.6875),
        (729201, 484.9668),
        (729240, 480.7091),
        (729270, 484.5506),
        (729292, 495.1445),
        (729309, 479.9111),
        (729329, 479.8815),
        (729391, 487.5683),
        (729430, 476.7355),
        (729478, 487.7251),
        (729525, 493.1012),
        (729568, 497.7565),
        (729608, 499.6481),
        (729642, 496.1591),
        (729658, 493.4581),
        (729687, 486.1097),
        (729706, 478.036),
        (729730, 480.4222),  # In crbug/1041688 this was the original placement.
        (729764, 421.0342),  # We instead should be setting it here.
        (729795, 428.0284),
        (729846, 433.8261),
        (729883, 429.49),
        (729920, 436.3342),
        (729975, 434.3996),
        (730011, 428.3672),
        (730054, 436.309),
        (730094, 435.3792),
        (730128, 433.0537),
    ]
    for row in sample_data:
      graph_data.Row(id=row[0], value=row[1], parent=test_container_key).put()
    test.UpdateSheriff()
    test.put()
    s = Subscription(name='sheriff', visibility=VISIBILITY.PUBLIC)
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s], None))) as m:
      find_anomalies.ProcessTests([test.key])
      self.assertEqual(m.call_args_list, [mock.call(test.test_path)])
    new_anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(1, len(new_anomalies))
    self.assertEqual(anomaly.DOWN, new_anomalies[0].direction)
    self.assertEqual(729731, new_anomalies[0].start_revision)
    self.assertEqual(729764, new_anomalies[0].end_revision)

  def testProcessTest_RefineAnomalyPlacement_OffByOneStable(self):
    testing_common.AddTests(
        ['ChromiumPerf'], ['linux-perf'], {
            'memory.desktop': {
                ('memory:chrome:all_processes:'
                 'reported_by_chrome:v8:effective_size_avg'): {}
            }
        })
    test = utils.TestKey(
        ('ChromiumPerf/linux-perf/memory.desktop/'
         'memory:chrome:all_processes:reported_by_chrome:v8:effective_size_avg'
        )).get()
    test_container_key = utils.GetTestContainerKey(test.key)
    sample_data = [
        (733480, 1381203.0),
        (733494, 1381220.0),
        (733504, 1381212.0),
        (733524, 1381220.0),
        (733538, 1381211.0),
        (733544, 1381212.0),
        (733549, 1381220.0),
        (733563, 1381220.0),
        (733581, 1381220.0),
        (733597, 1381212.0),
        (733611, 1381228.0),
        (733641, 1381212.0),
        (733675, 1381204.0),
        (733721, 1381212.0),
        (733766, 1381211.0),
        (733804, 1381204.0),
        (733835, 1381219.0),
        (733865, 1381211.0),
        (733885, 1381219.0),
        (733908, 1381204.0),
        (733920, 1381211.0),
        (733937, 1381220.0),
        (734091, 1381211.0),
        (734133, 1381219.0),
        (734181, 1381204.0),
        (734211, 1381720.0),
        (734248, 1381712.0),
        (734277, 1381696.0),
        (734311, 1381704.0),
        (734341, 1381703.0),
        (734372, 1381704.0),
        (734405, 1381703.0),
        (734431, 1381711.0),
        (734456, 1381720.0),
        (734487, 1381703.0),
        (734521, 1381704.0),
        (734554, 1381726.0),
        (734598, 1381704.0),
        (734630, 1381703.0),  # In crbug/1041688 this is where it was placed.
        (734673, 1529888.0),  # This is where it should be.
        (734705, 1529888.0),
        (734739, 1529860.0),
        (734770, 1529860.0),
        (734793, 1529888.0),
        (734829, 1529860.0),
    ]
    for row in sample_data:
      graph_data.Row(id=row[0], value=row[1], parent=test_container_key).put()
    test.UpdateSheriff()
    test.put()
    s = Subscription(name='sheriff', visibility=VISIBILITY.PUBLIC)
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s], None))) as m:
      find_anomalies.ProcessTests([test.key])
      self.assertEqual(m.call_args_list, [mock.call(test.test_path)])
    new_anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(1, len(new_anomalies))
    self.assertEqual(anomaly.UP, new_anomalies[0].direction)
    self.assertEqual(734631, new_anomalies[0].start_revision)
    self.assertEqual(734673, new_anomalies[0].end_revision)

  def testProcessTest_RefineAnomalyPlacement_MinSize0Max2Elements(self):
    testing_common.AddTests(['ChromiumPerf'], ['linux-perf'],
                            {'sizes': {
                                'method_count': {}
                            }})
    test = utils.TestKey(('ChromiumPerf/linux-perf/sizes/method_count')).get()
    test_container_key = utils.GetTestContainerKey(test.key)
    test.UpdateSheriff()
    test.put()
    sample_data = [
        (6990, 100),
        (6991, 100),
        (6992, 100),
        (6993, 100),
        (6994, 100),
        (6995, 100),
        (6996, 100),
        (6997, 100),
        (6998, 100),
        (6999, 100),
        (7000, 100),
        (7001, 155),
        (7002, 155),
        (7003, 155),
    ]
    for row in sample_data:
      graph_data.Row(id=row[0], value=row[1], parent=test_container_key).put()
    test.UpdateSheriff()
    test.put()
    s = Subscription(
        name='sheriff',
        visibility=VISIBILITY.PUBLIC,
        anomaly_configs=[
            AnomalyConfig(
                max_window_size=10,
                min_absolute_change=50,
                min_relative_change=0,
                min_segment_size=0)
        ])
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s], None))) as m:
      find_anomalies.ProcessTests([test.key])
      self.assertEqual(m.call_args_list, [mock.call(test.test_path)])
    new_anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(1, len(new_anomalies))
    self.assertEqual(anomaly.UP, new_anomalies[0].direction)
    self.assertEqual(7001, new_anomalies[0].start_revision)
    self.assertEqual(7001, new_anomalies[0].end_revision)

  def testProcessTest_MultipleChangePoints(self):
    testing_common.AddTests(
        ['ChromiumPerf'], ['linux-perf'],
        {'blink_perf.layout': {
            'nested-percent-height-tables': {}
        }})
    test = utils.TestKey(
        'ChromiumPerf/linux-perf/blink_perf.layout/nested-percent-height-tables'
    ).get()
    test_container_key = utils.GetTestContainerKey(test.key)
    sample_data = [
        (804863, 13830765),
        (804867, 16667862),
        (804879, 13929296),
        (804891, 13823876),
        (804896, 13908794),
        (804900, 13899281),
        (804907, 14901462),
        (804921, 13890597),
        (804935, 13969113),
        (804946, 13996520),
        (804957, 13913104),
        (805143, 16770364),
        (805175, 14858529),
        (805179, 14013942),
        (805185, 14857516),
        (805195, 14895168),
        (805196, 14944037),
        (805205, 13919484),
        (805211, 15736581),
        (805231, 14730142),
        (805236, 13892102),
        (805247, 14808876),
        (805253, 14903648),
        (805262, 13896626),
        (805276, 15797878),
        (805281, 14542593),
        (805285, 15733168),
        (805290, 13882841),
        (805302, 15727394),
        (805314, 15758058),
        (805333, 16074960),
        (805345, 16142162),
        (805359, 16138912),
        (805384, 17914289),
        (805412, 18368834),
        (805428, 18055197),
        (805457, 19673614),
        (805482, 19705606),
        (805502, 19609089),
        (805509, 19576745),
        (805531, 19600059),
        (805550, 19702969),
        (805564, 19660953),
        (805584, 19830273),
        (805600, 19800662),
        (805606, 19493150),
        (805620, 19700545),
        (805624, 19623731),
        (805628, 19683921),
        (805634, 19660001),
    ]
    for row in sample_data:
      graph_data.Row(id=row[0], value=row[1], parent=test_container_key).put()
    test.UpdateSheriff()
    test.put()
    s = Subscription(name='sheriff', visibility=VISIBILITY.PUBLIC)
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s], None))) as m:
      find_anomalies.ProcessTests([test.key])
      self.assertEqual(m.call_args_list, [mock.call(test.test_path)])
    new_anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(2, len(new_anomalies))
    self.assertEqual(anomaly.UP, new_anomalies[0].direction)
    self.assertEqual(805429, new_anomalies[0].start_revision)
    self.assertEqual(805457, new_anomalies[0].end_revision)
    self.assertEqual(805315, new_anomalies[1].start_revision)
    self.assertEqual(805428, new_anomalies[1].end_revision)

  def testProcessTest__RefineAnomalyPlacement_BalancedEstimator1(self):
    testing_common.AddTests(
        ['ChromiumPerf'], ['linux-perf'],
        {'blink_perf.layout': {
            'nested-percent-height-tables': {}
        }})
    test = utils.TestKey(
        'ChromiumPerf/linux-perf/blink_perf.layout/nested-percent-height-tables'
    ).get()
    test_container_key = utils.GetTestContainerKey(test.key)
    sample_data = [
        (818289, 2009771),
        (818290, 1966080),
        (818291, 1966080),
        (818293, 1966080),
        (818294, 2053461),
        (818296, 2009771),
        (818298, 1966080),
        (818301, 2009771),
        (818303, 2009771),
        (818305, 2009771),
        (818306, 2009771),
        (818307, 1966080),
        (818308, 2009771),
        (818309, 2009771),
        (818310, 1966080),
        (818311, 2009771),
        (818312, 1966080),
        (818317, 1966080),
        (818318, 1966080),
        (818320, 2053461),
        (818322, 2009771),
        (818326, 1966080),
        (818331, 1966080),
        (818335, 1966080),
        (818340, 2009771),
        (818347, 2009771),
        (818350, 1966080),
        (818353, 1966080),
        (818354, 2009771),
        (818361, 2009771),
        (818362, 1966080),
        (818374, 2009771),
        (818379, 2009771),
        (818382, 2053461),
        (818389, 2009771),
        (818402, 1966080),
        (818409, 2009771),
        (818416, 1966080),
        (818420, 1966080),
        (818430, 2009771),
        (818440, 2228224),
        (818450, 2228224),
        (818461, 2228224),
        (818469, 2228224),
        (818481, 2228224),
        (818498, 2271915),
        (818514, 2228224),
        (818531, 2271915),
        (818571, 2271915),
        (818583, 2271915),
    ]
    for row in sample_data:
      graph_data.Row(id=row[0], value=row[1], parent=test_container_key).put()
    test.UpdateSheriff()
    test.put()
    s = Subscription(name='sheriff', visibility=VISIBILITY.PUBLIC)
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s], None))) as m:
      find_anomalies.ProcessTests([test.key])
      self.assertEqual(m.call_args_list, [mock.call(test.test_path)])
    new_anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(1, len(new_anomalies))
    self.assertEqual(anomaly.UP, new_anomalies[0].direction)
    self.assertEqual(818431, new_anomalies[0].start_revision)
    self.assertEqual(818440, new_anomalies[0].end_revision)

  def testProcessTest__RefineAnomalyPlacement_BalancedEstimator2(self):
    testing_common.AddTests(
        ['ChromiumPerf'], ['linux-perf'],
        {'blink_perf.layout': {
            'nested-percent-height-tables': {}
        }})
    test = utils.TestKey(
        'ChromiumPerf/linux-perf/blink_perf.layout/nested-percent-height-tables'
    ).get()
    test_container_key = utils.GetTestContainerKey(test.key)
    sample_data = [
        (793468, 136.5382),
        (793486, 137.7192),
        (793495, 137.4038),
        (793504, 137.4919),
        (793505, 137.4465),
        (793518, 136.9279),
        (793525, 137.3501),
        (793528, 136.9622),
        (793543, 137.1027),
        (793550, 137.7351),
        (793555, 137.1511),
        (793559, 137.2094),
        (793560, 136.5192),
        (793565, 138.1536),
        (793580, 137.4172),
        (793590, 136.8746),
        (793601, 137.5016),
        (793609, 137.0773),
        (793625, 137.4702),
        (793646, 135.9019),
        (793657, 137.2827),
        (793702, 136.5978),
        (793712, 136.0732),
        (793721, 132.1820),
        (793742, 122.1631),
        (793760, 136.3152),
        (793774, 136.9616),
        (793788, 136.8438),
        (794016, 136.3022),
        (794024, 136.3495),
        (794027, 136.3145),
        (794036, 136.5502),
        (794043, 136.3861),
        (794051, 136.2035),
        (794059, 136.2348),
        (794066, 136.2594),
        (794074, 135.9686),
        (794088, 136.7375),
        (794107, 136.5570),
        (794132, 129.9924),  # This one is a potential change point - but weak
        (794143, 135.8275),
        (794154, 107.2502),  # This is a better change point
        (794158, 108.3948),
        (794160, 107.3564),
        (794196, 107.9707),
        (794236, 111.3168),
        (794268, 108.7905),
        (794281, 111.1065),
        (794319, 109.7699),
        (794320, 109.8082),
    ]
    for row in sample_data:
      graph_data.Row(id=row[0], value=row[1], parent=test_container_key).put()
    test.UpdateSheriff()
    test.put()
    s = Subscription(name='sheriff', visibility=VISIBILITY.PUBLIC)
    with mock.patch.object(SheriffConfigClient, 'Match',
                           mock.MagicMock(return_value=([s], None))) as m:
      find_anomalies.ProcessTests([test.key])
      self.assertEqual(m.call_args_list, [mock.call(test.test_path)])
    new_anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(1, len(new_anomalies))
    self.assertEqual(anomaly.DOWN, new_anomalies[0].direction)
    self.assertEqual(794144, new_anomalies[0].start_revision)
    self.assertEqual(794154, new_anomalies[0].end_revision)

  def testProcessTest__RefineAnomalyPlacement_OnePassEDivisive(self):
    testing_common.AddTests(
        ['ChromiumPerf'], ['linux-perf'],
        {'blink_perf.layout': {
            'nested-percent-height-tables': {}
        }})
    test = utils.TestKey(
        'ChromiumPerf/linux-perf/blink_perf.layout/nested-percent-height-tables'
    ).get()
    test_container_key = utils.GetTestContainerKey(test.key)
    # 1608562683 will be anomaly if we run E-Divisive from 1608525044.
    sample_data = [
        (1608404024, 246272),
        (1608407660, 249344),
        (1608417360, 246272),
        (1608422547, 246784),
        (1608434678, 248832),
        (1608440108, 248320),
        (1608442260, 250880),
        (1608452306, 248832),
        (1608457404, 247296),
        (1608459374, 247296),
        (1608463502, 249344),
        (1608469894, 247296),
        (1608471945, 247296),
        (1608477313, 246272),
        (1608481014, 248832),
        (1608484511, 247296),
        (1608486532, 246784),
        (1608488082, 248832),
        (1608491972, 246784),
        (1608493895, 248832),
        (1608495366, 248320),
        (1608498927, 252416),
        (1608501293, 246784),
        (1608505924, 246272),
        (1608507885, 246784),
        (1608509593, 250368),
        (1608512971, 246784),
        (1608515075, 246272),
        (1608519889, 247296),
        (1608521956, 254464),
        (1608525044, 247296),
        (1608526992, 244736),
        (1608528640, 245760),
        (1608530391, 246784),
        (1608531986, 245760),
        (1608533763, 245760),
        (1608538109, 246272),
        (1608539988, 246784),
        (1608545280, 251392),
        (1608547200, 251026),
        (1608550736, 248320),
        (1608552820, 248832),
        (1608554780, 251392),
        (1608560589, 247296),
        (1608562683, 251904),
        (1608564319, 268800),
        (1608566089, 263168),
        (1608567823, 266240),
        (1608569370, 266752),
        (1608570921, 264192),
    ]
    for row in sample_data:
      graph_data.Row(id=row[0], value=row[1], parent=test_container_key).put()
    test.UpdateSheriff()
    test.put()
    new_anomalies = anomaly.Anomaly.query().fetch()
    self.assertEqual(0, len(new_anomalies))

  def testMakeAnomalyEntity_NoRefBuild(self):
    testing_common.AddTests(['ChromiumPerf'], ['linux'], {
        'page_cycler_v2': {
            'cnn': {},
            'yahoo': {},
            'nytimes': {},
        },
    })
    test = utils.TestKey('ChromiumPerf/linux/page_cycler_v2').get()
    testing_common.AddRows(test.test_path, [100, 200, 300, 400])

    alert = find_anomalies._MakeAnomalyEntity(
        _MakeSampleChangePoint(10011, 50, 100), test, 'avg', self._DataSeries(),
        {}, Subscription()).get_result()
    self.assertIsNone(alert.ref_test)

  def testMakeAnomalyEntity_RefBuildSlash(self):
    testing_common.AddTests(['ChromiumPerf'], ['linux'], {
        'page_cycler_v2': {
            'ref': {},
            'cnn': {},
            'yahoo': {},
            'nytimes': {},
        },
    })
    test = utils.TestKey('ChromiumPerf/linux/page_cycler_v2').get()
    testing_common.AddRows(test.test_path, [100, 200, 300, 400])

    alert = find_anomalies._MakeAnomalyEntity(
        _MakeSampleChangePoint(10011, 50, 100), test, 'avg', self._DataSeries(),
        {}, Subscription()).get_result()
    self.assertEqual(alert.ref_test.string_id(),
                     'ChromiumPerf/linux/page_cycler_v2/ref')

  def testMakeAnomalyEntity_RefBuildUnderscore(self):
    testing_common.AddTests(['ChromiumPerf'], ['linux'], {
        'page_cycler_v2': {
            'cnn': {},
            'cnn_ref': {},
            'yahoo': {},
            'nytimes': {},
        },
    })
    test = utils.TestKey('ChromiumPerf/linux/page_cycler_v2/cnn').get()
    testing_common.AddRows(test.test_path, [100, 200, 300, 400])

    alert = find_anomalies._MakeAnomalyEntity(
        _MakeSampleChangePoint(10011, 50, 100), test, 'avg', self._DataSeries(),
        {}, Subscription()).get_result()
    self.assertEqual(alert.ref_test.string_id(),
                     'ChromiumPerf/linux/page_cycler_v2/cnn_ref')
    self.assertIsNone(alert.display_start)
    self.assertIsNone(alert.display_end)

  def testMakeAnomalyEntity_RevisionRanges(self):
    testing_common.AddTests(['ClankInternal'], ['linux'], {
        'page_cycler_v2': {
            'cnn': {},
            'cnn_ref': {},
            'yahoo': {},
            'nytimes': {},
        },
    })
    test = utils.TestKey('ClankInternal/linux/page_cycler_v2/cnn').get()
    testing_common.AddRows(test.test_path, [100, 200, 300, 400])
    for row in graph_data.Row.query():
      # Different enough to ensure it is picked up properly.
      row.r_commit_pos = int(row.value) + 2
      row.put()

    alert = find_anomalies._MakeAnomalyEntity(
        _MakeSampleChangePoint(300, 50, 100), test, 'avg', self._DataSeries(),
        {}, Subscription()).get_result()
    self.assertEqual(alert.display_start, 203)
    self.assertEqual(alert.display_end, 302)

  def testMakeAnomalyEntity_AddsOwnership(self):
    data_samples = [{
        'type': 'GenericSet',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb826',
        'values': ['alice@chromium.org', 'bob@chromium.org']
    }, {
        'type': 'GenericSet',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb827',
        'values': ['abc']
    }, {
        'type': 'GenericSet',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb828',
        'values': ['This is an info blurb.']
    }]

    test_key = utils.TestKey('ChromiumPerf/linux/page_cycler_v2/cnn')
    testing_common.AddTests(['ChromiumPerf'], ['linux'], {
        'page_cycler_v2': {
            'cnn': {},
            'cnn_ref': {},
            'yahoo': {},
            'nytimes': {},
        },
    })
    test = test_key.get()
    testing_common.AddRows(test.test_path, [100, 200, 300, 400])

    suite_key = utils.TestKey('ChromiumPerf/linux/page_cycler_v2')
    entity = histogram.SparseDiagnostic(
        data=data_samples[0],
        test=suite_key,
        start_revision=1,
        end_revision=sys.maxsize,
        id=data_samples[0]['guid'],
        name=reserved_infos.OWNERS.name)
    entity.put()

    entity = histogram.SparseDiagnostic(
        data=data_samples[1],
        test=suite_key,
        start_revision=1,
        end_revision=sys.maxsize,
        id=data_samples[1]['guid'],
        name=reserved_infos.BUG_COMPONENTS.name)
    entity.put()

    entity = histogram.SparseDiagnostic(
        data=data_samples[2],
        test=suite_key,
        start_revision=1,
        end_revision=sys.maxsize,
        id=data_samples[2]['guid'],
        name=reserved_infos.INFO_BLURB.name)
    entity.put()

    alert = find_anomalies._MakeAnomalyEntity(
        _MakeSampleChangePoint(10011, 50, 100), test, 'avg', self._DataSeries(),
        {}, Subscription()).get_result()

    self.assertEqual(alert.ownership['component'], 'abc')
    self.assertListEqual(alert.ownership['emails'],
                         ['alice@chromium.org', 'bob@chromium.org'])
    self.assertEqual(alert.ownership['info_blurb'], 'This is an info blurb.')

  def testMakeAnomalyEntity_AlertGrouping(self):
    data_sample = {
        'type': 'GenericSet',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb826',
        'values': ['group123', 'group234']
    }

    testing_common.AddTests(['ChromiumPerf'], ['linux'], {
        'page_cycler_v2': {
            'cnn': {},
            'cnn_ref': {},
            'yahoo': {},
            'nytimes': {},
        },
    })
    test = utils.TestKey('ChromiumPerf/linux/page_cycler_v2/cnn').get()
    testing_common.AddRows(test.test_path, [100, 200, 300, 400])

    suite_key = utils.TestKey('ChromiumPerf/linux/page_cycler_v2')
    entity = histogram.SparseDiagnostic(
        data=data_sample,
        test=suite_key,
        start_revision=1,
        end_revision=sys.maxsize,
        id=data_sample['guid'],
        name=reserved_infos.ALERT_GROUPING.name)
    entity.put()
    entity.put()
    alert = find_anomalies._MakeAnomalyEntity(
        _MakeSampleChangePoint(10011, 50, 100), test, 'avg', self._DataSeries(),
        {}, Subscription()).get_result()
    self.assertEqual(alert.alert_grouping, ['group123', 'group234'])


if __name__ == '__main__':
  unittest.main()
