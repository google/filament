# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=too-many-lines

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from unittest import mock
import datetime
from flask import Flask
import logging
import unittest
import webtest

from google.appengine.ext import ndb

from dashboard import alert_groups
from dashboard import sheriff_config_client
from dashboard.common import namespaced_stored_object
from dashboard.common import sandwich_allowlist
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import alert_group
from dashboard.models import alert_group_workflow
from dashboard.models import anomaly
from dashboard.models import graph_data
from dashboard.models import subscription
from dashboard.services import crrev_service
from dashboard.services import pinpoint_service
from dashboard.services import workflow_service

_SERVICE_ACCOUNT_EMAIL = 'service-account@chromium.org'

flask_app = Flask(__name__)


@flask_app.route('/alert_groups_update')
def AlertGroupsGet():
  return alert_groups.AlertGroupsGet()


@mock.patch.object(utils, 'ServiceAccountEmail',
                   lambda: _SERVICE_ACCOUNT_EMAIL)
class GroupReportTestBase(testing_common.TestCase):
  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    self.fake_issue_tracker = testing_common.FakeIssueTrackerService()
    self.fake_issue_tracker.comments.append({
        'id': 1,
        'author': _SERVICE_ACCOUNT_EMAIL,
        'updates': {
            'status': 'WontFix',
        },
    })
    self.mock_get_sheriff_client = mock.MagicMock()
    self.fake_revision_info = testing_common.FakeRevisionInfoClient(
        infos={}, revisions={})

  def setUp(self):
    super().setUp()
    self.maxDiff = None
    self.testapp = webtest.TestApp(flask_app)
    namespaced_stored_object.Set('repositories', {
        'chromium': {
            'repository_url': 'git://chromium'
        },
    })

  def _CallHandler(self):
    result = self.testapp.get('/alert_groups_update')
    self.ExecuteDeferredTasks('update-alert-group-queue')
    return result

  def _FindDuplicateGroupsMock(self, key_string):
    key = ndb.Key('AlertGroup', key_string)
    query = alert_group.AlertGroup.query(
        alert_group.AlertGroup.active == True,
        alert_group.AlertGroup.canonical_group == key)
    duplicated_groups = query.fetch()
    duplicated_keys = [g.key.string_id() for g in duplicated_groups]
    return duplicated_keys

  def _FindCanonicalGroupMock(self, key_string, merged_into,
                              merged_issue_project):
    key = ndb.Key('AlertGroup', key_string)
    query = alert_group.AlertGroup.query(
        alert_group.AlertGroup.active == True,
        alert_group.AlertGroup.bug.project == merged_issue_project,
        alert_group.AlertGroup.bug.bug_id == merged_into)
    query_result = query.fetch(limit=1)
    if not query_result:
      return None

    canonical_group = query_result[0]
    visited = set()
    while canonical_group.canonical_group:
      visited.add(canonical_group.key)
      next_group_key = canonical_group.canonical_group
      # Visited check is just precaution.
      # If it is true - the system previously failed to prevent loop creation.
      if next_group_key == key or next_group_key in visited:
        return None
      canonical_group = next_group_key.get()
    return {'key': canonical_group.key.string_id()}

  def _GetAllActiveAlertGroupsMock(self, group_type: int):
    all_groups = alert_group.AlertGroup.GetAll(group_type=group_type)
    all_keys = [g.key.id() for g in all_groups]
    ungrouped_group = alert_group.AlertGroup.Get(
        group_name='Ungrouped',
        group_type=alert_group.AlertGroup.Type.reserved)
    if len(ungrouped_group) > 0:
      all_keys.append(ungrouped_group[0].key.id())
    return all_keys

  def _PostUngroupedAlertsMock(self, group_type):
    groups = alert_group.AlertGroup.GetAll(group_type=group_type)

    def FindGroup(group):
      for g in groups:
        if group.IsOverlapping(g):
          return g.key
      groups.append(group)
      return None

    reserved = alert_group.AlertGroup.Type.reserved
    ungrouped_list = alert_group.AlertGroup.Get('Ungrouped', reserved)
    if not ungrouped_list:
      alert_group.AlertGroup(
          name='Ungrouped', group_type=reserved, active=True).put()
      return {}
    ungrouped = ungrouped_list[0]
    ungrouped_anomalies = ndb.get_multi(ungrouped.anomalies)

    parity_results = {}
    for anomaly_entity in ungrouped_anomalies:
      new_count = 0
      new_alert_groups = []
      all_groups = alert_group.AlertGroup.GenerateAllGroupsForAnomaly(
          anomaly_entity)
      for g in all_groups:
        found_group = FindGroup(g)
        if found_group:
          new_alert_groups.append(found_group)
        else:
          new_group = g.put()
          new_alert_groups.append(new_group)
          new_count += 1
      anomaly_entity.groups = new_alert_groups
      parity_results[anomaly_entity.key.id()] = {
          'new_groups': [g.id() for g in new_alert_groups],
          'existing_groups': []
      }
    logging.info('Persisting %i anomalies', len(ungrouped_anomalies))
    ndb.put_multi(ungrouped_anomalies)
    return parity_results

  def _PatchPerfIssueService(self, function_name, mock_function):
    perf_issue_post_patcher = mock.patch(function_name, mock_function)
    perf_issue_post_patcher.start()
    self.addCleanup(perf_issue_post_patcher.stop)

  def _SetUpMocks(self, mock_get_sheriff_client):
    sheriff = subscription.Subscription(name='sheriff',
                                        auto_triage_enable=True)
    mock_get_sheriff_client().Match.return_value = ([sheriff], None)
    self.PatchObject(crrev_service, 'GetNumbering',
                     lambda *args, **kargs: {'git_sha': 'abcd'})

    new_job = mock.MagicMock(return_value={'jobId': '123456'})
    self.PatchObject(pinpoint_service, 'NewJob', new_job)
    self.PatchObject(alert_group_workflow, 'revision_info_client',
                     self.fake_revision_info)
    self.PatchObject(alert_group, 'NONOVERLAP_THRESHOLD', 100)

    self._PatchPerfIssueService(
        'dashboard.services.perf_issue_service_client.GetIssue',
        self.fake_issue_tracker.GetIssue)
    self._PatchPerfIssueService(
        'dashboard.services.perf_issue_service_client.GetIssueComments',
        self.fake_issue_tracker.GetIssueComments)
    self._PatchPerfIssueService(
        'dashboard.services.perf_issue_service_client.PostIssue',
        self.fake_issue_tracker.NewBug)
    self._PatchPerfIssueService(
        'dashboard.services.perf_issue_service_client.PostIssueComment',
        self.fake_issue_tracker.AddBugComment)
    self._PatchPerfIssueService(
        'dashboard.services.perf_issue_service_client.GetDuplicateGroupKeys',
        self._FindDuplicateGroupsMock)
    self._PatchPerfIssueService(
        'dashboard.services.perf_issue_service_client.GetCanonicalGroupByIssue',
        self._FindCanonicalGroupMock)
    self._PatchPerfIssueService(
        'dashboard.services.perf_issue_service_client.GetAllActiveAlertGroups',
        self._GetAllActiveAlertGroupsMock)
    self._PatchPerfIssueService(
        'dashboard.services.perf_issue_service_client.PostUngroupedAlerts',
        self._PostUngroupedAlertsMock)

  def _AddAnomaly(self, **kargs):
    t = graph_data.TestMetadata(
        id='master/bot/test_suite/measurement/test_case',)
    t.improvement_direction = anomaly.UP
    t.put()

    default = {
        'test': 'master/bot/test_suite/measurement/test_case',
        'start_revision': 1,
        'end_revision': 100,
        'is_improvement': False,
        'median_before_anomaly': 1.1,
        'median_after_anomaly': 1.3,
        'ownership': {
            'component': 'Foo>Bar',
            'emails': ['x@google.com', 'y@google.com'],
        },
    }
    default.update(kargs)
    default['test'] = utils.TestKey(default['test'])
    graph_data.TestMetadata(
        key=default['test'],
        unescaped_story_name='story',
    ).put()
    a = anomaly.Anomaly(**default)
    clt = sheriff_config_client.GetSheriffConfigClient()
    subscriptions, _ = clt.Match(a)
    groups_for_anomaly = alert_group.AlertGroup.GetGroupsForAnomaly(
        a, subscriptions)
    a.groups = groups_for_anomaly
    return a.put()


@mock.patch.object(utils, 'ServiceAccountEmail',
                   lambda: _SERVICE_ACCOUNT_EMAIL)
@mock.patch('dashboard.sheriff_config_client.GetSheriffConfigClient')
@mock.patch.object(sandwich_allowlist, 'CheckAllowlist',
                   testing_common.CheckSandwichAllowlist)
class GroupReportTest(GroupReportTestBase):
  def testNoGroup(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    # Put an anomaly before Ungrouped is created
    self._AddAnomaly()

  def testCreatingUngrouped(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self.assertIs(
        len(
            alert_group.AlertGroup.Get(
                'Ungrouped',
                alert_group.AlertGroup.Type.reserved,
            )), 0)
    response = self._CallHandler()
    self.assertEqual(response.status_code, 200)
    self.assertIs(
        len(
            alert_group.AlertGroup.Get(
                'Ungrouped',
                alert_group.AlertGroup.Type.reserved,
            )), 1)

  def testCreatingGroup(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    # Ungrouped is created in first run
    self._CallHandler()
    # Put an anomaly after Ungrouped is created
    a1 = self._AddAnomaly()
    # Anomaly is associated with Ungrouped and AlertGroup Created
    self._CallHandler()
    # Anomaly is associated with its AlertGroup
    self._CallHandler()
    self.assertEqual(len(a1.get().groups), 1)
    self.assertEqual(a1.get().groups[0].get().name, 'test_suite')

  def testMultipleAltertsGroupingDifferentDomain_BeforeGroupCreated(
      self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self.testapp.get('/alert_groups_update')
    self.ExecuteDeferredTasks('update-alert-group-queue')
    # Add anomalies
    a1 = self._AddAnomaly()
    a2 = self._AddAnomaly(start_revision=50, end_revision=150)
    a3 = self._AddAnomaly(test='other/bot/test_suite/measurement/test_case')
    a4 = self._AddAnomaly(median_before_anomaly=0)
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    groups = {
        g.domain: g
        for g in alert_group.AlertGroup.Get(
            'test_suite', alert_group.AlertGroup.Type.test_suite)
    }
    self.assertCountEqual(groups['master'].anomalies, [a1, a2, a4])
    self.assertCountEqual(groups['other'].anomalies, [a3])

  def testMultipleAltertsGroupingDifferentDomain_AfterGroupCreated(
      self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self.testapp.get('/alert_groups_update')
    self.ExecuteDeferredTasks('update-alert-group-queue')
    # Add anomalies
    a1 = self._AddAnomaly()
    a2 = self._AddAnomaly(start_revision=50, end_revision=150)
    a4 = self._AddAnomaly(median_before_anomaly=0)
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Add anomalies in other domain
    a3 = self._AddAnomaly(test='other/bot/test_suite/measurement/test_case')
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    groups = {
        g.domain: g
        for g in alert_group.AlertGroup.Get(
            'test_suite', alert_group.AlertGroup.Type.test_suite)
    }
    self.assertCountEqual(groups['master'].anomalies, [a1, a2, a4])
    self.assertCountEqual(groups['other'].anomalies, [a3])

  def testMultipleAltertsGroupingDifferentBot(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self.testapp.get('/alert_groups_update')
    self.ExecuteDeferredTasks('update-alert-group-queue')
    # Add anomalies
    a1 = self._AddAnomaly()
    a2 = self._AddAnomaly(start_revision=50, end_revision=150)
    a3 = self._AddAnomaly(test='master/other/test_suite/measurement/test_case')
    a4 = self._AddAnomaly(median_before_anomaly=0)
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertCountEqual(group.anomalies, [a1, a2, a3, a4])

  def testMultipleAltertsGroupingDifferentSuite(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self.testapp.get('/alert_groups_update')
    self.ExecuteDeferredTasks('update-alert-group-queue')
    # Add anomalies
    a1 = self._AddAnomaly()
    a2 = self._AddAnomaly(start_revision=50, end_revision=150)
    a3 = self._AddAnomaly(test='master/bot/other/measurement/test_case')
    a4 = self._AddAnomaly(median_before_anomaly=0)
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertCountEqual(group.anomalies, [a1, a2, a4])
    group = alert_group.AlertGroup.Get(
        'other',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertCountEqual(group.anomalies, [a3])

  def testMultipleAltertsGroupingOverrideSuite(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self.testapp.get('/alert_groups_update')
    self.ExecuteDeferredTasks('update-alert-group-queue')
    # Add anomalies
    a1 = self._AddAnomaly(test='master/bot/test_suite/measurement/test_case', )
    a2 = self._AddAnomaly(
        test='master/bot/test_suite/measurement/test_case',
        start_revision=50,
        end_revision=150,
    )
    self._AddAnomaly(
        test='master/bot/other/measurement/test_case',
        alert_grouping=['test_suite', 'test_suite_other1'],
    )
    self._AddAnomaly(
        test='master/bot/test_suite/measurement/test_case',
        median_before_anomaly=0,
        alert_grouping=['test_suite_other1', 'test_suite_other2'],
    )
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertCountEqual(group.anomalies, [a1, a2])

  def testMultipleAltertsGroupingMultipleSheriff(self,
                                                 mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    mock_get_sheriff_client().Match.return_value = ([
        subscription.Subscription(name='sheriff1',
                                  auto_triage_enable=True,
                                  auto_bisect_enable=True),
        subscription.Subscription(name='sheriff2',
                                  auto_triage_enable=True,
                                  auto_bisect_enable=True),
    ], None)
    self.testapp.get('/alert_groups_update')
    self.ExecuteDeferredTasks('update-alert-group-queue')
    # Add anomaly
    a1 = self._AddAnomaly()
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Add another anomaly with part of the subscription
    mock_get_sheriff_client().Match.return_value = ([
        subscription.Subscription(name='sheriff1',
                                  auto_triage_enable=True,
                                  auto_bisect_enable=True),
    ], None)
    a2 = self._AddAnomaly()
    # Update Group to associate alerts
    self._CallHandler()
    # Add another anomaly with different subscription
    mock_get_sheriff_client().Match.return_value = ([
        subscription.Subscription(name='sheriff2',
                                  auto_triage_enable=True,
                                  auto_bisect_enable=True),
        subscription.Subscription(name='sheriff3',
                                  auto_triage_enable=True,
                                  auto_bisect_enable=True),
    ], None)
    a3 = self._AddAnomaly()
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()

    groups = {
        g.subscription_name: g
        for g in alert_group.AlertGroup.Get(
            'test_suite', alert_group.AlertGroup.Type.test_suite)
    }
    self.assertCountEqual(
        list(groups.keys()), ['sheriff1', 'sheriff2', 'sheriff3'])
    self.assertCountEqual(groups['sheriff1'].anomalies, [a1, a2])
    self.assertCountEqual(groups['sheriff2'].anomalies, [a1, a3])
    self.assertCountEqual(groups['sheriff3'].anomalies, [a3])

  def testMultipleAltertsGroupingPointRange(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self.testapp.get('/alert_groups_update')
    self.ExecuteDeferredTasks('update-alert-group-queue')
    # Add anomalies
    a1 = self._AddAnomaly(start_revision=100, end_revision=100)
    a2 = self._AddAnomaly(start_revision=100, end_revision=100)
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertCountEqual(group.anomalies, [a1, a2])

  def testArchiveAltertsGroup(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self._CallHandler()
    # Add anomalies
    self._AddAnomaly()
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Set Update timestamp to 10 days ago
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.updated = datetime.datetime.utcnow() - datetime.timedelta(days=10)
    group.put()
    # Archive Group
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
        active=False,
    )[0]
    self.assertEqual(group.name, 'test_suite')

  def testArchiveAltertsGroupIssueClosed(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self.fake_issue_tracker.issue['state'] = 'open'
    self._CallHandler()
    # Add anomalies
    self._AddAnomaly()
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Set Create timestamp to 2 hours ago
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
    group.put()
    # Create Issue
    self._CallHandler()
    # Out of active window
    group.updated = datetime.datetime.utcnow() - datetime.timedelta(days=10)
    # Archive Group
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
        active=False,
    )[0]
    self.assertEqual(group.name, 'test_suite')

  @mock.patch('dashboard.common.utils.ShouldDelayIssueReporting',
              mock.MagicMock(return_value=False))
  @unittest.expectedFailure
  def testTriageAltertsGroup_Sandwiched(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    mock_get_sheriff_client().Match.return_value = ([
        subscription.Subscription(
            name='sheriff', auto_triage_enable=True, auto_bisect_enable=True)
    ], None)
    self._CallHandler()
    # Add anomalies
    a = self._AddAnomaly()
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Set Create timestamp to 2 hours ago
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
    group.put()
    # Submit issue
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertEqual(group.status, alert_group.AlertGroup.Status.triaged)
    self.assertEqual(self.fake_issue_tracker.new_bug_kwargs['components'], [])
    self.assertEqual(
        sorted(self.fake_issue_tracker.new_bug_kwargs['labels']),
        sorted([
            'Pri-2', 'Restrict-View-Google', 'Type-Bug-Regression',
            'Chromeperf-Auto-Triaged'
        ]))
    self.assertRegex(self.fake_issue_tracker.new_bug_kwargs['description'],
                     r'Top 1 affected measurements in bot:')
    self.assertEqual(a.get().bug_id, 12345)
    self.assertEqual(group.bug.bug_id, 12345)
    # Make sure we don't file the issue again for this alert group.
    self.fake_issue_tracker.new_bug_args = None
    self.fake_issue_tracker.new_bug_kwargs = None
    self._CallHandler()
    self.assertIsNone(self.fake_issue_tracker.new_bug_args)
    self.assertIsNone(self.fake_issue_tracker.new_bug_kwargs)

  @mock.patch('dashboard.common.utils.ShouldDelayIssueReporting',
              mock.MagicMock(return_value=True))
  @unittest.expectedFailure
  def testTriageAltertsGroup_Sandwiched_DelayReport(self,
                                                    mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    mock_get_sheriff_client().Match.return_value = ([
        subscription.Subscription(
            name='sheriff', auto_triage_enable=True, auto_bisect_enable=True)
    ], None)
    self._CallHandler()
    # Add anomalies
    a = self._AddAnomaly()
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Set Create timestamp to 2 hours ago
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
    group.put()
    # Submit issue
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertEqual(group.status, alert_group.AlertGroup.Status.triaged)
    self.assertEqual(self.fake_issue_tracker.new_bug_kwargs['components'],
                     ['Speed>Regressions'])
    self.assertEqual(
        sorted(self.fake_issue_tracker.new_bug_kwargs['labels']),
        sorted([
            'Pri-2', 'Restrict-View-Google', 'Type-Bug-Regression',
            'Chromeperf-Auto-Triaged', 'Chromeperf-Delay-Reporting'
        ]))
    self.assertRegex(self.fake_issue_tracker.new_bug_kwargs['description'],
                     r'Top 1 affected measurements in bot:')
    self.assertEqual(a.get().bug_id, 12345)
    self.assertEqual(group.bug.bug_id, 12345)
    # Make sure we don't file the issue again for this alert group.
    self.fake_issue_tracker.new_bug_args = None
    self.fake_issue_tracker.new_bug_kwargs = None
    self._CallHandler()
    self.assertIsNone(self.fake_issue_tracker.new_bug_args)
    self.assertIsNone(self.fake_issue_tracker.new_bug_kwargs)

  def testTriageAltertsGroup_NotSandwiched(self, mock_get_sheriff_client):
    blocked_test_name = 'master/bot/blocked-test_suite/measurement/test_case'
    self._SetUpMocks(mock_get_sheriff_client)
    self._CallHandler()
    # Add anomalies
    a = self._AddAnomaly(test=blocked_test_name)
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Set Create timestamp to 2 hours ago
    group = alert_group.AlertGroup.Get(
        'blocked-test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
    group.put()
    # Submit issue
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'blocked-test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertEqual(group.status, alert_group.AlertGroup.Status.triaged)
    self.assertCountEqual(self.fake_issue_tracker.new_bug_kwargs['components'],
                          ['Foo>Bar'])
    self.assertCountEqual(self.fake_issue_tracker.new_bug_kwargs['labels'], [
        'Pri-2', 'Restrict-View-Google', 'Type-Bug-Regression',
        'Chromeperf-Auto-Triaged'
    ])
    self.assertRegex(self.fake_issue_tracker.new_bug_kwargs['description'],
                     r'Top 1 affected measurements in bot:')
    self.assertEqual(a.get().bug_id, 12345)
    self.assertEqual(group.bug.bug_id, 12345)
    # Make sure we don't file the issue again for this alert group.
    self.fake_issue_tracker.new_bug_args = None
    self.fake_issue_tracker.new_bug_kwargs = None
    self._CallHandler()
    self.assertIsNone(self.fake_issue_tracker.new_bug_args)
    self.assertIsNone(self.fake_issue_tracker.new_bug_kwargs)

  # TODO(dberris): Re-enable this when we start supporting multiple benchmarks
  # in the same alert group in the future.
  @unittest.expectedFailure
  def testTriageAltertsGroup_MultipleBenchmarks(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self._CallHandler()
    # Add anomalies
    a = self._AddAnomaly()
    _ = self._AddAnomaly(
        test='master/bot/other_test_suite/measurement/test_case')
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Set Create timestamp to 2 hours ago
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
    group.put()
    # Submit issue
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertEqual(group.status, alert_group.AlertGroup.Status.triaged)
    self.assertCountEqual(self.fake_issue_tracker.new_bug_kwargs['components'],
                          ['Foo>Bar'])
    self.assertCountEqual(self.fake_issue_tracker.new_bug_kwargs['labels'], [
        'Pri-2', 'Restrict-View-Google', 'Type-Bug-Regression',
        'Chromeperf-Auto-Triaged'
    ])
    logging.debug('Rendered:\n%s', self.fake_issue_tracker.new_bug_args[1])
    self.assertRegex(self.fake_issue_tracker.new_bug_args[1],
                     r'Top 4 affected measurements in bot:')
    self.assertRegex(self.fake_issue_tracker.new_bug_args[1],
                     r'Top 1 affected in test_suite:')
    self.assertRegex(self.fake_issue_tracker.new_bug_args[1],
                     r'Top 1 affected in other_test_suite:')
    self.assertEqual(a.get().bug_id, 12345)
    self.assertEqual(group.bug.bug_id, 12345)
    # Make sure we don't file the issue again for this alert group.
    self.fake_issue_tracker.new_bug_args = None
    self.fake_issue_tracker.new_bug_kwargs = None
    self._CallHandler()
    self.assertIsNone(self.fake_issue_tracker.new_bug_args)
    self.assertIsNone(self.fake_issue_tracker.new_bug_kwargs)

  @mock.patch('dashboard.common.utils.ShouldDelayIssueReporting',
              mock.MagicMock(return_value=False))
  def testTriageAltertsGroupNoOwners_Sandwiched(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    mock_get_sheriff_client().Match.return_value = ([
        subscription.Subscription(
            name='sheriff', auto_triage_enable=True, auto_bisect_enable=True)
    ], None)
    self._CallHandler()
    # Add anomalies
    a = self._AddAnomaly(ownership={
        'component': 'Foo>Bar',
        'emails': None,
    })
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Set Create timestamp to 2 hours ago
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
    group.put()
    # Submit issue
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertEqual(group.status, alert_group.AlertGroup.Status.triaged)
    self.assertEqual(self.fake_issue_tracker.new_bug_kwargs['components'], [])
    self.assertEqual(
        sorted(self.fake_issue_tracker.new_bug_kwargs['labels']),
        sorted([
            'Pri-2', 'Restrict-View-Google', 'Type-Bug-Regression',
            'Chromeperf-Auto-Triaged'
        ]))
    self.assertEqual(a.get().bug_id, 12345)

  @mock.patch('dashboard.common.utils.ShouldDelayIssueReporting',
              mock.MagicMock(return_value=True))
  def testTriageAltertsGroupNoOwners_Sandwiched_DelayReport(
      self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    mock_get_sheriff_client().Match.return_value = ([
        subscription.Subscription(
            name='sheriff', auto_triage_enable=True, auto_bisect_enable=True)
    ], None)
    self._CallHandler()
    # Add anomalies
    a = self._AddAnomaly(ownership={
        'component': 'Foo>Bar',
        'emails': None,
    })
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Set Create timestamp to 2 hours ago
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
    group.put()
    # Submit issue
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertEqual(group.status, alert_group.AlertGroup.Status.triaged)
    self.assertEqual(self.fake_issue_tracker.new_bug_kwargs['components'],
                     ['Speed>Regressions'])
    self.assertEqual(
        sorted(self.fake_issue_tracker.new_bug_kwargs['labels']),
        sorted([
            'Pri-2', 'Restrict-View-Google', 'Type-Bug-Regression',
            'Chromeperf-Auto-Triaged', 'Chromeperf-Delay-Reporting'
        ]))
    self.assertEqual(a.get().bug_id, 12345)

  def testTriageAltertsGroupNoOwners_NotSandwiched(self,
                                                   mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self._CallHandler()
    blocked_test_name = 'master/bot/blocked-test_suite/measurement/test_case'
    # Add anomalies
    a = self._AddAnomaly(
        test=blocked_test_name,
        ownership={
            'component': 'Foo>Bar',
            'emails': None,
        })
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Set Create timestamp to 2 hours ago
    group = alert_group.AlertGroup.Get(
        'blocked-test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
    group.put()
    # Submit issue
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'blocked-test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertEqual(group.status, alert_group.AlertGroup.Status.triaged)
    self.assertEqual(self.fake_issue_tracker.new_bug_kwargs['components'],
                     ['Foo>Bar'])
    self.assertEqual(
        sorted(self.fake_issue_tracker.new_bug_kwargs['labels']),
        sorted([
            'Pri-2', 'Restrict-View-Google', 'Type-Bug-Regression',
            'Chromeperf-Auto-Triaged'
        ]))
    self.assertEqual(a.get().bug_id, 12345)

  def testAddAlertsAfterTriage_Sandwiched(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self._CallHandler()
    # Add anomalies
    a = self._AddAnomaly()
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Set Create timestamp to 2 hours ago
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
    group.put()
    # Submit issue
    self._CallHandler()

    # Add anomalies
    anomalies = [
        self._AddAnomaly(),
        self._AddAnomaly(median_before_anomaly=0),
    ]
    mock_get_sheriff_client().Match.return_value = ([
        subscription.Subscription(
            name='sheriff', auto_triage_enable=True, auto_bisect_enable=True)
    ], None)

    self._CallHandler()
    for a in anomalies:
      self.assertEqual(a.get().bug_id, 12345)
    self.assertEqual(self.fake_issue_tracker.add_comment_args[0], 12345)
    self.assertEqual(self.fake_issue_tracker.add_comment_kwargs['components'],
                     [])

  def testAddAlertsAfterTriage_NotSandwiched(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self._CallHandler()
    blocked_test_name = 'master/blocked-bot/blocked-test_suite/measurement/test_case'
    # Add anomalies
    a = self._AddAnomaly(test=blocked_test_name)
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Set Create timestamp to 2 hours ago
    group = alert_group.AlertGroup.Get(
        'blocked-test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
    group.put()
    # Submit issue
    self._CallHandler()

    # Add anomalies
    anomalies = [
        self._AddAnomaly(test=blocked_test_name),
        self._AddAnomaly(test=blocked_test_name, median_before_anomaly=0),
    ]
    self._CallHandler()
    for a in anomalies:
      self.assertEqual(a.get().bug_id, 12345)
    self.assertEqual(self.fake_issue_tracker.add_comment_args[0], 12345)
    self.assertEqual(self.fake_issue_tracker.add_comment_kwargs['components'],
                     ['Foo>Bar'])

  def testMultipleAltertsNonoverlapThreshold(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    self._CallHandler()
    perf_test = 'ChromiumPerf/bot/test_suite/measurement/test_case'

    # Anomalies without range overlap.
    a1 = self._AddAnomaly(start_revision=10, end_revision=40, test=perf_test)
    a2 = self._AddAnomaly(start_revision=50, end_revision=150, test=perf_test)
    a4 = self._AddAnomaly(start_revision=200, end_revision=300, test=perf_test)
    self._CallHandler()

    # Anomaly that overlaps with first 2 alert groups.
    a5 = self._AddAnomaly(start_revision=5, end_revision=100, test=perf_test)

    # Anomaly that exceeds nonoverlap threshold of all existing alert groups.
    a6 = self._AddAnomaly(start_revision=5, end_revision=305, test=perf_test)
    self._CallHandler()

    # Anomaly that binds to a6's group.
    a7 = self._AddAnomaly(start_revision=10, end_revision=300, test=perf_test)
    self._CallHandler()
    self._CallHandler()

    groups = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )

    anomaly_groups = [group.anomalies for group in groups]
    expected_anomaly_groups = [[a1, a5], [a2, a5], [a4], [a6, a7]]

    self.assertCountEqual(anomaly_groups, expected_anomaly_groups)


@mock.patch.object(utils, 'ServiceAccountEmail',
                   lambda: _SERVICE_ACCOUNT_EMAIL)
@mock.patch('dashboard.sheriff_config_client.GetSheriffConfigClient')
@mock.patch.object(sandwich_allowlist, 'CheckAllowlist',
                   testing_common.CheckSandwichAllowlist)
class RecoveredAlertsTests(GroupReportTestBase):
  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    self.anomalies = []

  def InitAfterMocks(self):
    # First create the 'Ungrouped' AlertGroup.
    self._CallHandler()

    # Then create the alert group which has a regression and recovered
    # regression.
    self.anomalies = [
        self._AddAnomaly(),
        self._AddAnomaly(recovered=True, start_revision=50, end_revision=150),
    ]
    self._CallHandler()

    # Then we update the group to associate alerts.
    self._CallHandler()

    # Set Create timestamp to 2 hours ago, so that the next time the handler is
    # called, we'd trigger the update processing.
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
    group.put()

  def testNoRecovered(self, mock_get_sheriff_client):
    # Ensure that we only include the non-recovered regressions in the filing.
    self._SetUpMocks(mock_get_sheriff_client)
    self.InitAfterMocks()
    self._CallHandler()
    logging.debug('Rendered:\n%s',
                  self.fake_issue_tracker.new_bug_kwargs['description'])
    self.assertRegex(self.fake_issue_tracker.new_bug_kwargs['description'],
                     r'Top 1 affected measurements in bot:')

  def testClosesIssueOnAllRecovered(self, mock_get_sheriff_client):
    # Ensure that we close the issue if all regressions in the group have been
    # marked 'recovered'.
    self._SetUpMocks(mock_get_sheriff_client)
    self.InitAfterMocks()
    self._CallHandler()
    logging.debug('Rendered:\n%s',
                  self.fake_issue_tracker.new_bug_kwargs['description'])
    self.assertRegex(self.fake_issue_tracker.new_bug_kwargs['description'],
                     r'Top 1 affected measurements in bot:')
    # Mark one of the anomalies recovered.
    recovered_anomaly = self.anomalies[0].get()
    recovered_anomaly.recovered = True
    recovered_anomaly.put()
    self._CallHandler()
    self.assertEqual(self.fake_issue_tracker.issue['state'], 'closed')
    self.assertRegex(
        self.fake_issue_tracker.add_comment_kwargs['comment'],
        r'All regressions for this issue have been marked recovered; closing.')

  def testReopensClosedIssuesWithNewRegressions(self, mock_get_sheriff_client):
    # pylint: disable=no-value-for-parameter
    self.testClosesIssueOnAllRecovered()
    self._SetUpMocks(mock_get_sheriff_client)
    mock_get_sheriff_client().Match.return_value = ([
        subscription.Subscription(name='sheriff',
                                  auto_triage_enable=True,
                                  auto_bisect_enable=True)
    ], None)
    # Then we add a new anomaly which should cause the issue to be reopened.
    self._AddAnomaly(start_revision=50,
                     end_revision=75,
                     test='master/bot/test_suite/measurement/other_test_case')
    self._CallHandler()
    logging.debug('Rendered:\n%s',
                  self.fake_issue_tracker.add_comment_kwargs['comment'])
    self.assertEqual(self.fake_issue_tracker.issue["state"], 'open')
    self.assertRegex(
        self.fake_issue_tracker.add_comment_kwargs['comment'],
        r'Reopened due to new regressions detected for this alert group:')
    self.assertRegex(self.fake_issue_tracker.add_comment_kwargs['comment'],
                     r'test_suite/measurement/other_test_case')

  def testManualClosedIssuesWithNewRegressions(self, mock_get_sheriff_client):
    # pylint: disable=no-value-for-parameter
    self.testClosesIssueOnAllRecovered()
    self._SetUpMocks(mock_get_sheriff_client)
    mock_get_sheriff_client().Match.return_value = ([
        subscription.Subscription(name='sheriff',
                                  auto_triage_enable=True,
                                  auto_bisect_enable=True)
    ], None)
    self.fake_issue_tracker.comments.append({
        'id': 2,
        'author': "sheriff@chromium.org",
        'updates': {
            'status': 'WontFix',
        },
    })
    # Then we add a new anomaly which should cause the issue to be reopened.
    self._AddAnomaly(start_revision=50,
                     end_revision=75,
                     test='master/bot/test_suite/measurement/other_test_case')
    self._CallHandler()
    logging.debug('Rendered:\n%s',
                  self.fake_issue_tracker.add_comment_kwargs['comment'])
    self.assertEqual(self.fake_issue_tracker.issue["state"], 'closed')
    self.assertRegex(self.fake_issue_tracker.add_comment_kwargs['comment'],
                     r'test_suite/measurement/other_test_case')

  @mock.patch.object(workflow_service, 'CreateExecution', return_value='fake-execution-id')
  def testStartAutoBisection_Sandwiched_DoNotBisectYet(self,
                                                       mock_workflow_service,
                                                       mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    mock_get_sheriff_client().Match.return_value = ([
        subscription.Subscription(
            name='sheriff', auto_triage_enable=True, auto_bisect_enable=True)
    ], None)
    self.assertIsNotNone(mock_workflow_service)
    self._CallHandler()
    # Add anomalies
    self._AddAnomaly()
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Set Create timestamp to 2 hours ago
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
    group.put()
    # Submit issue
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    # Should not jump straight to bisecting because it should first run a
    # verification workflow to verify the reported regression.
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertEqual(group.bisection_ids, [])
    self.assertEqual(group.status, alert_group.AlertGroup.Status.sandwiched)

  def testStartAutoBisection_NotSandwiched(self, mock_get_sheriff_client):
    self._SetUpMocks(mock_get_sheriff_client)
    mock_get_sheriff_client().Match.return_value = ([
        subscription.Subscription(
            name='blocked-sheriff',
            auto_triage_enable=True,
            auto_bisect_enable=True)
    ], None)
    self._CallHandler()
    # Add anomalies
    self._AddAnomaly()
    # Create Group
    self._CallHandler()
    # Update Group to associate alerts
    self._CallHandler()
    # Set Create timestamp to 2 hours ago
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
    group.put()
    # Submit issue
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    # Start bisection
    self._CallHandler()
    group = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )[0]
    self.assertEqual(group.bisection_ids, ['123456'])


@mock.patch.object(utils, 'ServiceAccountEmail',
                   lambda: _SERVICE_ACCOUNT_EMAIL)
class NonChromiumAutoTriage(GroupReportTestBase):
  def testFileIssue_InChromiumExplicitly(self):
    self.mock_get_sheriff_client.Match.return_value = ([
        subscription.Subscription(name='sheriff',
                                  auto_triage_enable=True,
                                  monorail_project_id='chromium')
    ], None)
    self.PatchObject(alert_group.sheriff_config_client,
                     'GetSheriffConfigClient',
                     lambda: self.mock_get_sheriff_client)
    self._SetUpMocks(self.mock_get_sheriff_client)
    self._CallHandler()
    a = self._AddAnomaly()
    self._CallHandler()
    grouped_anomaly = a.get()
    self.assertEqual(grouped_anomaly.project_id, 'chromium')

  def testAlertGroups_OnePerProject(self):
    self.mock_get_sheriff_client.Match.return_value = ([
        subscription.Subscription(name='chromium sheriff',
                                  auto_triage_enable=True,
                                  monorail_project_id='chromium'),
        subscription.Subscription(name='v8 sheriff',
                                  auto_triage_enable=True,
                                  monorail_project_id='v8')
    ], None)
    self.PatchObject(alert_group.sheriff_config_client,
                     'GetSheriffConfigClient',
                     lambda: self.mock_get_sheriff_client)
    self._SetUpMocks(self.mock_get_sheriff_client)

    # First create the 'Ungrouped' AlertGroup.
    self._CallHandler()

    # Then create an anomaly.
    self._AddAnomaly()
    self._CallHandler()

    # Ensure that we have two different groups on different projects.
    groups = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )
    self.assertEqual(2, len(groups))
    self.assertCountEqual(['chromium', 'v8'], [g.project_id for g in groups])
    for group in groups:
      group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
      group.put()

    # And that we've filed two issues.
    self._CallHandler()
    self.assertCountEqual([{
        'method': 'NewBug',
        'args': (),
        'kwargs': {
            'title': mock.ANY,
            'description': mock.ANY,
            'project': 'v8',
            'cc': [],
            'labels': mock.ANY,
            'components': mock.ANY,
        },
    }, {
        'method': 'NewBug',
        'args': (),
        'kwargs': {
            'title': mock.ANY,
            'description': mock.ANY,
            'project': 'chromium',
            'cc': [],
            'labels': mock.ANY,
            'components': mock.ANY,
        },
    }], self.fake_issue_tracker.calls)

  def testAlertGroups_NonChromium(self):
    self.mock_get_sheriff_client.Match.return_value = ([
        subscription.Subscription(name='non-chromium sheriff',
                                  auto_triage_enable=True,
                                  monorail_project_id='non-chromium')
    ], None)
    self.PatchObject(alert_group.sheriff_config_client,
                     'GetSheriffConfigClient',
                     lambda: self.mock_get_sheriff_client)
    self._SetUpMocks(self.mock_get_sheriff_client)
    self._CallHandler()
    a = self._AddAnomaly()
    self._CallHandler()
    groups = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )
    self.assertEqual(1, len(groups))
    self.assertEqual(['non-chromium'], [g.project_id for g in groups])
    for group in groups:
      group.created = datetime.datetime.utcnow() - datetime.timedelta(hours=2)
      group.put()
    self._CallHandler()
    self.assertCountEqual([{
        'method': 'NewBug',
        'args': (),
        'kwargs': {
            'title': mock.ANY,
            'description': mock.ANY,
            'project': 'non-chromium',
            'cc': [],
            'labels': mock.ANY,
            'components': mock.ANY,
        }
    }], self.fake_issue_tracker.calls)
    a = a.get()
    self.assertEqual(a.project_id, 'non-chromium')

    stored_issue = self.fake_issue_tracker.GetIssue(a.bug_id, 'non-chromium')
    logging.debug('bug_id = %s', a.bug_id)
    self.assertIsNotNone(stored_issue)

    # Now let's ensure that when new anomalies come in, that we're grouping
    # them into the same group for non-chromium alerts.
    self._AddAnomaly(start_revision=2)
    self._CallHandler()
    groups = alert_group.AlertGroup.Get(
        'test_suite',
        alert_group.AlertGroup.Type.test_suite,
    )
    self.assertEqual(1, len(groups))
    self.assertEqual(groups[0].project_id, 'non-chromium')
