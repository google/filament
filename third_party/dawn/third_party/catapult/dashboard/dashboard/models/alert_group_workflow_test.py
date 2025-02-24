# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=too-many-lines

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
import json
from unittest import mock
import six
import unittest
import uuid

from google.appengine.ext import ndb

from dashboard.common import feature_flags
from dashboard.common import namespaced_stored_object
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import alert_group
from dashboard.models import alert_group_workflow
from dashboard.models import anomaly
from dashboard.common import sandwich_allowlist
from dashboard.models import subscription
from dashboard.services import workflow_service

_SERVICE_ACCOUNT_EMAIL = 'service-account@chromium.org'


@mock.patch.object(sandwich_allowlist, 'CheckAllowlist',
                   testing_common.CheckSandwichAllowlist)
class AlertGroupWorkflowTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.maxDiff = None
    self._issue_tracker = testing_common.FakeIssueTrackerService()
    self._sheriff_config = testing_common.FakeSheriffConfigClient()
    self._pinpoint = testing_common.FakePinpoint()
    self._cloud_workflows = testing_common.FakeCloudWorkflows()
    self._crrev = testing_common.FakeCrrev()
    self._gitiles = testing_common.FakeGitiles()
    self._revision_info = testing_common.FakeRevisionInfoClient(
        infos={
            'r_chromium_commit_pos': {
                'name':
                    'Chromium Commit Position',
                'url':
                    'http://test-results.appspot.com/revision_range?start={{R1}}&end={{R2}}',
            },
        },
        revisions={
            'master/bot/test_suite/measurement/test_case': {
                0: {
                    'r_chromium_commit_pos': '0'
                },
                100: {
                    'r_chromium_commit_pos': '100'
                },
            }
        })
    self._service_account = lambda: _SERVICE_ACCOUNT_EMAIL

    perf_issue_patcher = mock.patch(
        'dashboard.services.perf_issue_service_client.GetIssue',
        self._issue_tracker.GetIssue)
    perf_issue_patcher.start()
    self.addCleanup(perf_issue_patcher.stop)

    perf_comments_patcher = mock.patch(
        'dashboard.services.perf_issue_service_client.GetIssueComments',
        self._issue_tracker.GetIssueComments)
    perf_comments_patcher.start()
    self.addCleanup(perf_comments_patcher.stop)

    perf_issue_post_patcher = mock.patch(
        'dashboard.services.perf_issue_service_client.PostIssue',
        self._issue_tracker.NewBug)
    perf_issue_post_patcher.start()
    self.addCleanup(perf_issue_post_patcher.stop)

    perf_comment_post_patcher = mock.patch(
        'dashboard.services.perf_issue_service_client.PostIssueComment',
        self._issue_tracker.AddBugComment)
    perf_comment_post_patcher.start()
    self.addCleanup(perf_comment_post_patcher.stop)

    namespaced_stored_object.Set('repositories', {
        'chromium': {
            'repository_url': 'git://chromium'
        },
    })

    perf_comment_post_patcher = mock.patch(
        'dashboard.services.perf_issue_service_client.GetDuplicateGroupKeys',
        self._FindDuplicateGroupsMock)
    perf_comment_post_patcher.start()
    self.addCleanup(perf_comment_post_patcher.stop)

    perf_comment_post_patcher = mock.patch(
        'dashboard.services.perf_issue_service_client.GetCanonicalGroupByIssue',
        self._FindCanonicalGroupMock)
    perf_comment_post_patcher.start()
    self.addCleanup(perf_comment_post_patcher.stop)
    feature_flags.SANDWICH_VERIFICATION = True

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

  @staticmethod
  def _AddAnomaly(is_summary=False, **kwargs):
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
            'info_blurb': 'This is an info blurb.',
        },
    }
    default.update(kwargs)

    tests = default['test'].split('/')

    def GenerateTestDict(tests):
      if not tests:
        return {}
      return {tests[0]: GenerateTestDict(tests[1:])}

    testing_common.AddTests([tests[0]], [tests[1]], GenerateTestDict(tests[2:]))
    test_key = utils.TestKey(default['test'])
    if not is_summary:
      t = test_key.get()
      t.unescaped_story_name = 'story'
      t.put()

    default['test'] = test_key

    return anomaly.Anomaly(**default).put()

  @staticmethod
  def _AddSignalQualityScore(anomaly_key, signal_score):
    version = 0
    key = ndb.Key(
        'SignalQuality',
        anomaly_key.get().test.string_id(),
        'SignalQualityScore',
        str(version),
    )

    return alert_group_workflow.SignalQualityScore(
        key=key,
        score=signal_score,
        updated_time=datetime.datetime.now(),
    ).put()

  @staticmethod
  def _AddAlertGroup(anomaly_key,
                     subscription_name=None,
                     issue=None,
                     anomalies=None,
                     status=None,
                     project_id=None,
                     bisection_ids=None,
                     canonical_group=None,
                     sandwich_verification_workflow_id=None):
    anomaly_entity = anomaly_key.get()
    group = alert_group.AlertGroup(
        id=str(uuid.uuid4()),
        name=anomaly_entity.benchmark_name,
        subscription_name=subscription_name or 'sheriff',
        status=alert_group.AlertGroup.Status.untriaged,
        project_id=project_id or 'chromium',
        active=True,
        revision=alert_group.RevisionRange(
            repository='chromium',
            start=anomaly_entity.start_revision,
            end=anomaly_entity.end_revision,
        ),
        bisection_ids=bisection_ids or [],
        sandwich_verification_workflow_id=sandwich_verification_workflow_id,
    )
    if issue:
      group.bug = alert_group.BugInfo(
          bug_id=issue.get('id'),
          project=issue.get('projectId', 'chromium'),
      )
      group.project_id = issue.get('projectId', 'chromium')
    if anomalies:
      group.anomalies = anomalies
    if status:
      group.status = status
    if canonical_group:
      group.canonical_group = canonical_group
    return group.put()

  @staticmethod
  # Perform same update on the same group twice because operation will only
  # be triggered when monorail not being updated
  def _UpdateTwice(workflow, update):
    workflow.Process(update=update)
    workflow.Process(update=update)

  def testAddAnomalies_GroupUntriaged(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    added = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(anomalies[0], anomalies=anomalies)
    self._sheriff_config.patterns = {
        '*': [subscription.Subscription(name='sheriff')],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies + added),
            issue={},
        ))

    self.assertEqual(len(group.get().anomalies), 4)
    for a in added:
      self.assertIn(a, group.get().anomalies)

  # A helper function that simulates the process of an alert group passing
  # the Sandiwch Regression Verification phase with a successful repro of
  # the regression, ready to proceed to bisection.
  def _SandwichVerify(self, group, anomalies, opt_decision=True):
    sandwich_verification_workflow_id = self._cloud_workflows.CreateExecution(
        anomalies[0])
    sandwich_workflow = self._cloud_workflows.GetExecution(
        sandwich_verification_workflow_id)
    sandwich_workflow['state'] = workflow_service.EXECUTION_STATE_SUCCEEDED
    sandwich_workflow['result'] = '''{
        "job_id": "pinpoint-job-id-12345",
        "anomaly": "",
        "statistic": "",
        "decision": %s
    }''' % ('true' if opt_decision else 'false')
    g = group.get()
    g.sandwich_verification_workflow_id = sandwich_verification_workflow_id
    g.status = alert_group.AlertGroup.Status.sandwiched
    g.put()

  def _SandwichVerifyFailure(self, group, anomalies):
    sandwich_verification_workflow_id = self._cloud_workflows.CreateExecution(
        anomalies[0])
    sandwich_workflow = self._cloud_workflows.GetExecution(
        sandwich_verification_workflow_id)
    sandwich_workflow['state'] = workflow_service.EXECUTION_STATE_FAILED

    # https://cloud.google.com/workflows/docs/reference/executions/rest/v1beta/projects.locations.workflows.executions#Error
    sandwich_workflow['error'] = {
        'payload': 'payload string',
        'context': 'context string',
        'stackTrace': {}
    }
    g = group.get()
    g.sandwich_verification_workflow_id = sandwich_verification_workflow_id
    g.status = alert_group.AlertGroup.Status.sandwiched
    g.put()

  def testAddAnomalies_GroupTriaged_IssueOpen(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    added = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        anomalies=anomalies,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies + added),
            issue=self._issue_tracker.issue,
        ))

    self.assertEqual(len(group.get().anomalies), 4)
    self.assertEqual(group.get().status, alert_group.AlertGroup.Status.triaged)
    for a in added:
      self.assertIn(a, group.get().anomalies)
      self.assertEqual(group.get().bug.bug_id,
                       self._issue_tracker.add_comment_args[0])
      self.assertIn('Added 2 regressions to the group',
                    self._issue_tracker.add_comment_kwargs['comment'])
      self.assertIn('[4] regressions in test_suite',
                    self._issue_tracker.add_comment_kwargs['title'])
      self.assertIn('sheriff', self._issue_tracker.add_comment_kwargs['title'])
      self.assertFalse(self._issue_tracker.add_comment_kwargs['send_email'])

  def testAddAnomalies_GroupTriaged_IssueClosed(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    added = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        anomalies=anomalies,
        status=alert_group.AlertGroup.Status.closed,
    )
    self._issue_tracker.issue.update({
        'state':
            'closed',
        'comments': [{
            'id': 1,
            'author': _SERVICE_ACCOUNT_EMAIL,
            'updates': {
                'status': 'WontFix'
            },
        }],
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        service_account=self._service_account,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies + added),
            issue=self._issue_tracker.issue,
        ))

    self.assertEqual(len(group.get().anomalies), 4)
    self.assertEqual('closed', self._issue_tracker.issue.get('state'))
    for a in added:
      self.assertIn(a, group.get().anomalies)
      self.assertEqual(group.get().bug.bug_id,
                       self._issue_tracker.add_comment_args[0])
      self.assertIn('Added 2 regressions to the group',
                    self._issue_tracker.add_comment_kwargs['comment'])
      self.assertIn('[4] regressions in test_suite',
                    self._issue_tracker.add_comment_kwargs['title'])
      self.assertIn('sheriff', self._issue_tracker.add_comment_kwargs['title'])
      self.assertFalse(self._issue_tracker.add_comment_kwargs['send_email'])

  def testAddAnomalies_GroupTriaged_IssueClosed_LegacyAccount(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    added = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        anomalies=anomalies,
        status=alert_group.AlertGroup.Status.closed,
    )
    self._issue_tracker.issue.update({
        'state':
            'closed',
        'comments': [{
            'id': 1,
            'author': utils.LEGACY_SERVICE_ACCOUNT,
            'updates': {
                'status': 'WontFix'
            },
        }],
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        service_account=lambda: utils.LEGACY_SERVICE_ACCOUNT,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies + added),
            issue=self._issue_tracker.issue,
        ))

    self.assertEqual(len(group.get().anomalies), 4)
    self.assertEqual('closed', self._issue_tracker.issue.get('state'))
    for a in added:
      self.assertIn(a, group.get().anomalies)
      self.assertEqual(group.get().bug.bug_id,
                       self._issue_tracker.add_comment_args[0])
      self.assertIn('Added 2 regressions to the group',
                    self._issue_tracker.add_comment_kwargs['comment'])
      self.assertIn('[4] regressions in test_suite',
                    self._issue_tracker.add_comment_kwargs['title'])
      self.assertIn('sheriff', self._issue_tracker.add_comment_kwargs['title'])
      self.assertFalse(self._issue_tracker.add_comment_kwargs['send_email'])

  def testAddAnomalies_GroupTriaged_IssueClosed_AutoBisect(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    added = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        anomalies=anomalies,
        status=alert_group.AlertGroup.Status.closed,
    )
    self._issue_tracker.issue.update({
        'state':
            'closed',
        'comments': [{
            'id': 1,
            'author': _SERVICE_ACCOUNT_EMAIL,
            'updates': {
                'status': 'WontFix'
            },
        }],
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        service_account=self._service_account,
    )
    w.Process(
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies + added),
            issue=self._issue_tracker.issue,
        ))

    self.assertEqual(len(group.get().anomalies), 4)
    self.assertEqual('open', self._issue_tracker.issue.get('state'))
    for a in added:
      self.assertIn(a, group.get().anomalies)
      self.assertEqual(group.get().bug.bug_id,
                       self._issue_tracker.add_comment_args[0])
      self.assertIn('Added 2 regressions to the group',
                    self._issue_tracker.add_comment_kwargs['comment'])
    self.assertFalse(self._issue_tracker.add_comment_kwargs['send_email'])

  def testUpdate_GroupTriaged_IssueClosed(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state':
            'closed',
        'comments': [{
            'id': 1,
            'author': _SERVICE_ACCOUNT_EMAIL,
            'updates': {
                'status': 'WontFix'
            },
        }],
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        service_account=self._service_account,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertEqual(group.get().status, alert_group.AlertGroup.Status.closed)

  def testAddAnomalies_GroupTriaged_IssueClosed_Manual(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    added = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        anomalies=anomalies,
        status=alert_group.AlertGroup.Status.closed,
    )
    self._issue_tracker.issue.update({
        'state':
            'closed',
        'comments': [{
            'id': 2,
            'author': "sheriff@chromium.org",
            'updates': {
                'status': 'WontFix'
            },
        }, {
            'id': 1,
            'author': _SERVICE_ACCOUNT_EMAIL,
            'updates': {
                'status': 'WontFix'
            },
        }],
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        service_account=self._service_account,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies + added),
            issue=self._issue_tracker.issue,
        ))

    self.assertEqual(len(group.get().anomalies), 4)
    self.assertEqual('closed', self._issue_tracker.issue.get('state'))
    for a in added:
      self.assertIn(a, group.get().anomalies)
      self.assertEqual(group.get().bug.bug_id,
                       self._issue_tracker.add_comment_args[0])
      self.assertIn('Added 2 regressions to the group',
                    self._issue_tracker.add_comment_kwargs['comment'])
    self.assertFalse(self._issue_tracker.add_comment_kwargs['send_email'])

  def testUpdate_GroupTriaged_IssueClosed_AllTriaged(self):
    anomalies = [
        self._AddAnomaly(recovered=True),
        self._AddAnomaly(recovered=True)
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        anomalies=anomalies,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state':
            'closed',
        'comments': [{
            'id': 1,
            'author': _SERVICE_ACCOUNT_EMAIL,
            'updates': {
                'status': 'WontFix'
            },
        }],
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        service_account=self._service_account,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertEqual(group.get().status, alert_group.AlertGroup.Status.closed)
    self.assertIsNone(self._issue_tracker.add_comment_args)

  def testAddAnomalies_GroupTriaged_CommentsNone(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    added = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        anomalies=anomalies,
        status=alert_group.AlertGroup.Status.closed,
    )
    self._issue_tracker.issue.update({
        'state': 'closed',
        'comments': None,
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        service_account=self._service_account,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies + added),
            issue=self._issue_tracker.issue,
        ))

    self.assertEqual(len(group.get().anomalies), 4)
    self.assertEqual('closed', self._issue_tracker.issue.get('state'))
    for a in added:
      self.assertIn(a, group.get().anomalies)
      self.assertEqual(group.get().bug.bug_id,
                       self._issue_tracker.add_comment_args[0])
      self.assertIn('Added 2 regressions to the group',
                    self._issue_tracker.add_comment_kwargs['comment'])
    self.assertFalse(self._issue_tracker.add_comment_kwargs['send_email'])

  def testUpdate_GroupClosed_IssueOpen(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.closed,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))

    self.assertEqual(group.get().status, alert_group.AlertGroup.Status.triaged)

  def testUpdate_GroupTriaged_AlertsAllRecovered(self):
    anomalies = [
        self._AddAnomaly(recovered=True),
        self._AddAnomaly(recovered=True),
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))

    self.assertEqual('closed', self._issue_tracker.issue.get('state'))

  def testUpdate_GroupTriaged_AlertsPartRecovered(self):
    anomalies = [self._AddAnomaly(recovered=True), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))

    self.assertEqual('open', self._issue_tracker.issue.get('state'))

  def testUpdate_NoAnomaliesFound(self):
    anomalies = [self._AddAnomaly(recovered=True), self._AddAnomaly()]
    group = self._AddAlertGroup(
        self._AddAnomaly(),
        issue=self._issue_tracker.issue,
        anomalies=anomalies,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
    )
    update = alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
        now=datetime.datetime.utcnow(),
        anomalies=[],
        issue=self._issue_tracker.issue,
    )
    w.Process(update=update)

    self.assertEqual(anomalies, group.get().anomalies)
    self.assertEqual('open', self._issue_tracker.issue.get('state'))

  def testTriage_GroupUntriaged(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        status=alert_group.AlertGroup.Status.untriaged,
    )
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        revision_info=self._revision_info,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
        crrev=self._crrev,
        gitiles=self._gitiles
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=None,
        ))
    self.assertIn('[2] regressions',
                  self._issue_tracker.new_bug_kwargs['title'])
    self.assertIn(
        'Chromium Commit Position: http://test-results.appspot.com/revision_range?start=0&end=100',
        self._issue_tracker.new_bug_kwargs['description'])

  # === Delay Reporting ===
  # Delay Reporting will be enabled when auto bisect is enabled.
  # If it is enabled:
  #  the component/cc/labels in subscription will not be added in issue.
  @mock.patch('dashboard.common.utils.ShouldDelayIssueReporting',
              mock.MagicMock(return_value=True))
  def testTriage_GroupUntriaged_DelayReporting_Delayed(self):
    test_subscription = 'AnySub'
    enable_auto_bisect = True
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        status=alert_group.AlertGroup.Status.untriaged,
        subscription_name=test_subscription)
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name=test_subscription,
                bug_components=['test-component'],
                bug_cc_emails=['test-cc'],
                bug_labels=['test-label'],
                auto_triage_enable=True,
                auto_bisect_enable=enable_auto_bisect)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        revision_info=self._revision_info,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
        crrev=self._crrev,
        gitiles=self._gitiles)
    w.Process(
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=None,
        ))
    self.assertIn('[2] regressions',
                  self._issue_tracker.new_bug_kwargs['title'])
    self.assertIn(
        'Chromium Commit Position: http://test-results.appspot.com/revision_range?start=0&end=100',
        self._issue_tracker.new_bug_kwargs['description'])
    self.assertIn(utils.DELAY_REPORTING_PLACEHOLDER,
                  self._issue_tracker.new_bug_kwargs['components'])
    self.assertIn(utils.DELAY_REPORTING_LABEL,
                  self._issue_tracker.new_bug_kwargs['labels'])
    self.assertNotIn('test-component',
                     self._issue_tracker.new_bug_kwargs['components'])
    self.assertNotIn('test-cc', self._issue_tracker.new_bug_kwargs['cc'])
    self.assertNotIn('test-label', self._issue_tracker.new_bug_kwargs['labels'])

  @mock.patch('dashboard.common.utils.ShouldDelayIssueReporting',
              mock.MagicMock(return_value=True))
  def testTriage_GroupUntriaged_DelayReporting_NotDelayed_NoBisect(self):
    test_subscription = 'AnySub-blocked'
    enable_auto_bisect = False
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        status=alert_group.AlertGroup.Status.untriaged,
        subscription_name=test_subscription)
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name=test_subscription,
                bug_components=['test-component'],
                bug_cc_emails=['test-cc'],
                bug_labels=['test-label'],
                auto_triage_enable=True,
                auto_bisect_enable=enable_auto_bisect)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        revision_info=self._revision_info,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
        crrev=self._crrev,
        gitiles=self._gitiles)
    w.Process(
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=None,
        ))
    self.assertIn('[2] regressions',
                  self._issue_tracker.new_bug_kwargs['title'])
    self.assertIn(
        'Chromium Commit Position: http://test-results.appspot.com/revision_range?start=0&end=100',
        self._issue_tracker.new_bug_kwargs['description'])
    self.assertNotIn(utils.DELAY_REPORTING_PLACEHOLDER,
                     self._issue_tracker.new_bug_kwargs['components'])
    self.assertNotIn(utils.DELAY_REPORTING_LABEL,
                     self._issue_tracker.new_bug_kwargs['labels'])
    self.assertIn('Pri-2', self._issue_tracker.new_bug_kwargs['labels'])
    self.assertIn('test-component',
                  self._issue_tracker.new_bug_kwargs['components'])
    self.assertIn('test-cc', self._issue_tracker.new_bug_kwargs['cc'])
    self.assertIn('test-label', self._issue_tracker.new_bug_kwargs['labels'])

  def testTriage_GroupUntriaged_MultiSubscriptions(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        status=alert_group.AlertGroup.Status.untriaged,
    )
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff'),
            subscription.Subscription(
                name='sheriff_not_bind', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        revision_info=self._revision_info,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=None,
        ))
    self.assertIsNone(self._issue_tracker.new_bug_args)

  def testTriage_GroupUntriaged_NonChromiumProject(self):
    anomalies = [self._AddAnomaly()]
    # TODO(dberris): Figure out a way to not have to hack the fake service to
    # seed it with the correct issue in the correct project.
    self._issue_tracker.issues[(
        'v8', self._issue_tracker.bug_id)] = self._issue_tracker.issues[(
            'chromium', self._issue_tracker.bug_id)]
    del self._issue_tracker.issues[('chromium', self._issue_tracker.bug_id)]
    self._issue_tracker.issues[('v8', self._issue_tracker.bug_id)].update({
        'projectId': 'v8',
    })
    group = self._AddAlertGroup(
        anomalies[0],
        status=alert_group.AlertGroup.Status.untriaged,
        project_id='v8')
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                monorail_project_id='v8')
        ],
    }
    self.assertEqual(group.get().project_id, 'v8')
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        revision_info=self._revision_info,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
        crrev=self._crrev,
        gitiles=self._gitiles)
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=None))
    self.assertEqual(group.get().bug.project, 'v8')
    self.assertEqual(anomalies[0].get().project_id, 'v8')

  def testTriage_GroupUntriaged_MultipleRange(self):
    anomalies = [
        self._AddAnomaly(median_before_anomaly=0.2, start_revision=10),
        self._AddAnomaly(median_before_anomaly=0.1)
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        status=alert_group.AlertGroup.Status.untriaged,
    )
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        revision_info=self._revision_info,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
        crrev=self._crrev,
        gitiles=self._gitiles
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=None,
        ))
    self.assertIn('[2] regressions',
                  self._issue_tracker.new_bug_kwargs['title'])
    self.assertIn(
        'Chromium Commit Position: http://test-results.appspot.com/revision_range?start=0&end=100',
        self._issue_tracker.new_bug_kwargs['description'])

  def testTriage_GroupUntriaged_InfAnomaly(self):
    anomalies = [self._AddAnomaly(median_before_anomaly=0), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        status=alert_group.AlertGroup.Status.untriaged,
    )
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        revision_info=self._revision_info,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
        crrev=self._crrev,
        gitiles=self._gitiles
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=None,
        ))
    self.assertIn('inf', self._issue_tracker.new_bug_kwargs['description'])

  def testTriage_GroupTriaged_InfAnomaly(self):
    anomalies = [self._AddAnomaly(median_before_anomaly=0), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertIn('inf', self._issue_tracker.add_comment_kwargs['comment'])
    self.assertFalse(self._issue_tracker.add_comment_kwargs['send_email'])

  def testArchive_GroupUntriaged(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        anomalies=anomalies,
        status=alert_group.AlertGroup.Status.untriaged,
    )
    self._sheriff_config.patterns = {
        '*': [subscription.Subscription(name='sheriff')],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=0),
            triage_delay=datetime.timedelta(hours=0),
        ),
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow() + datetime.timedelta(seconds=1),
            anomalies=ndb.get_multi(anomalies),
            issue=None,
        ))
    self.assertEqual(False, group.get().active)

  def testArchive_GroupTriaged(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        anomalies=anomalies,
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=0),
            triage_delay=datetime.timedelta(hours=0),
        ),
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertEqual(False, group.get().active)

  def testBisect_GroupTriaged_SandwichVerify_Repro(self):
    anomalies = [
        self._AddAnomaly(median_before_anomaly=0.2),
        self._AddAnomaly(median_before_anomaly=0.1),
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    tags = json.loads(self._pinpoint.new_job_request['tags'])
    self.assertEqual(six.ensure_str(anomalies[1].urlsafe()), tags['alert'])

    # Tags must be a dict of key/value string pairs.
    for k, v in tags.items():
      self.assertIsInstance(k, six.string_types)
      self.assertIsInstance(v, six.string_types)

    self.assertEqual(['123456'], group.get().bisection_ids)
    self.assertEqual(['Chromeperf-Auto-Bisected'],
                     self._issue_tracker.add_comment_kwargs['labels'])

  def testBisect_GroupTriaged_SandwichVerify_NoRepro(self):
    anomalies = [
        self._AddAnomaly(median_before_anomaly=0.2),
        self._AddAnomaly(median_before_anomaly=0.1),
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies, False)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertIsNone(self._pinpoint.new_job_request)

    self.assertEqual([], group.get().bisection_ids)
    self.assertEqual(
        ['Chromeperf-Auto-Closed', 'Regression-Verification-No-Repro'],
        self._issue_tracker.add_comment_kwargs['labels'])

  @mock.patch('dashboard.common.utils.ShouldDelayIssueReporting',
              mock.MagicMock(return_value=False))
  @unittest.expectedFailure
  def testSandwich_Allowlist_enabled(self):
    test_cases = [
        'master/blocked-bot/test_suite/measurement/test_case',
        'master/sandwichable-bot/blocked-benchmark/Speedometer2/test_case',
        'master/win-10-perf/test_suite/measurement/test_case',
        'master/mac-m1_mini_2020-perf/jetstream2/JetStream2/test_case'
    ]
    anomalies = []
    for test_case in test_cases:
      anomalies.append(
          self._AddAnomaly(
              test=test_case,
              ownership={'component': 'anomaly>cannot>set>components'}))
    test_subscription = 'Sandwich-Allowed Subscription'
    group = self._AddAlertGroup(anomalies[0],
                                anomalies=anomalies,
                                subscription_name=test_subscription)
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name=test_subscription,
                auto_triage_enable=True,
                auto_merge_enable=True,
                auto_bisect_enable=True,
                bug_components=['sub>should>set>component'],
                bug_labels=['sub-should-set-labels']),
            # This subscription should get ignored by Process, since it doesn't match
            # the AlertGroup's subscription_name property.
            subscription.Subscription(
                name='blocked-sandwich-sub',
                auto_triage_enable=True,
                auto_merge_enable=True,
                auto_bisect_enable=True,
                bug_components=['other>sub>cannot>set>component'],
                bug_labels=['other-sub-can-set-labels'])
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        # This part with triage_delay set to 0 is critical if you want the _UpdateTwice call to
        # result in AlertGroupWorfklow.Process calling _TryTriage (and _FileIssue) to exercise
        # the code path that creates new bugs during auto-triage. This code is *badly* in need of
        # refactoring w/ e.g. a State Machine or Strategy pattern, or preferably just a complete
        # ground-up rewrite.
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
        crrev=self._crrev,
    )
    self._issue_tracker.issue.update({'state': 'open'})

    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue={},
        ))

    self.assertEqual(len(group.get().anomalies), 4)

    feature_flags.SANDWICH_VERIFICATION = True
    allowed_regressions = w._CheckSandwichAllowlist(ndb.get_multi(anomalies))

    self.assertEqual(len(allowed_regressions), 2)
    r1 = allowed_regressions[0]
    self.assertEqual(r1.benchmark_name, 'test_suite')
    self.assertEqual(r1.bot_name, 'win-10-perf')
    r2 = allowed_regressions[1]
    self.assertEqual(r2.benchmark_name, 'jetstream2')
    self.assertEqual(r2.bot_name, 'mac-m1_mini_2020-perf')

    feature_flags.SANDWICH_VERIFICATION = False
    allowed_regressions = w._CheckSandwichAllowlist(ndb.get_multi(anomalies))

    self.assertEqual(len(allowed_regressions), 0)
    self.assertEqual([], sorted(self._issue_tracker.issue.get('components')))
    self.assertIn('other-sub-can-set-labels',
                  self._issue_tracker.issue.get('labels'))
    self.assertIn('sub-should-set-labels',
                  self._issue_tracker.issue.get('labels'))

  def testSandwich_Allowlist_blocked(self):
    # Test blocked subscription
    test_name = '/'.join(
        ["master", 'linux-perf', 'speedometer2', 'dummy', 'metric', 'parts'])
    anomalies = []
    anomalies.append(self._AddAnomaly(test=test_name, is_improvement=True))
    test_subscription = 'blocked subscription'
    group = self._AddAlertGroup(anomalies[0],
                                anomalies=anomalies,
                                subscription_name=test_subscription)
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name=test_subscription,
                auto_triage_enable=True,
                bug_components=['should>not>set>component'])
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue={},
        ))

    self.assertEqual(len(group.get().anomalies), 1)
    feature_flags.SANDWICH_VERIFICATION = True
    allowed_regressions = w._CheckSandwichAllowlist(ndb.get_multi(anomalies))
    self.assertEqual(len(allowed_regressions), 0)

  def testSandwich_CheckAllowList(self):
    res = sandwich_allowlist.CheckAllowlist("Sandwich Verification Test JetStream2",
        "jetstream2", "android-pixel4-perf")
    self.assertTrue(res)

  def testSandwich_TryVerifyRegression_createsSandwichWorkflow(self):
    # Pre-coditions:
    # - feature_flags.SANDWICH_VERIFICATION is True
    # - New anomaly appears
    # - Its subscription is enabled for auto_triage and auto_bisect
    # - The anomaly's AlertGroup status is 'triaged`
    # - The anomaly's benchmark/workload/config is allowed for sandwich verification
    # Post-conditions:
    # - The anomaly's AlertGroup state is 'sandwiched'
    # - A "Sandwich Verification" *cloud* workflow has been requested
    # - The AlertGroup's sandwich_verification_workflow_id is not None
    # - *No* pinpoint bisection job has been started for the alert group
    # - The issue does not have components from the sandwich sheriff config assigned to it
    feature_flags.SANDWICH_VERIFICATION = True

    test_name = '/'.join(
        ["master", 'linux-perf', 'speedometer2', 'dummy', 'metric', 'parts'])
    anomalies = [
        self._AddAnomaly(test=test_name, statistic="made up"),
        self._AddAnomaly(test=test_name, statistic="also made up")
    ]

    group = self._AddAlertGroup(
        anomalies[0],
        subscription_name='Sandwich-Allowed Subscription',
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({'state': 'open'})
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='Sandwich-Allowed Subscription',
                auto_triage_enable=True,
                auto_bisect_enable=True,
                auto_merge_enable=True,
                bug_components=['should>not>set>component'])
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self.assertNotIn('Chromeperf-Auto-BisectOptOut',
                     self._issue_tracker.issue.get('labels'))
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertNotIn('Chromeperf-Auto-BisectOptOut',
                     self._issue_tracker.issue.get('labels'))
    self.assertNotIn('should>not>set>component', self._issue_tracker.issue.get('components'))
    self.assertEqual(w._group.status, alert_group.AlertGroup.Status.sandwiched)
    self.assertIsNotNone(w._group.sandwich_verification_workflow_id)
    self.assertIsNone(self._pinpoint.new_job_request)
    workflow_anomaly = self._cloud_workflows.create_execution_called_with_anomaly
    self.assertIsNotNone(workflow_anomaly)
    self.assertIsNotNone(workflow_anomaly['benchmark'])
    self.assertIsNotNone(workflow_anomaly['bot_name'])
    self.assertIsNotNone(workflow_anomaly['story'])
    self.assertIsNotNone(workflow_anomaly['measurement'])
    self.assertIsNotNone(workflow_anomaly['target'])
    self.assertIsNotNone(workflow_anomaly['start_git_hash'])
    self.assertIsNotNone(workflow_anomaly['end_git_hash'])
    self.assertIsNotNone(workflow_anomaly['project'])

  def testSandwich_autoBisectDisabled_StillAddsIssueComponent(self):
    # Pre-coditions:
    # - feature_flags.SANDWICH_VERIFICATION is True
    # - New anomaly appears
    # - Its subscription is enabled for auto_triage=True and auto_bisect=False
    # - The anomaly's AlertGroup status is 'triaged`
    # - The anomaly's benchmark/workload/config is allowed for sandwich verification
    # Post-conditions:
    # - The anomaly's AlertGroup state is '?'
    # - No "Sandwich Verification" *cloud* workflow has been requested
    # - The AlertGroup's sandwich_verification_workflow_id is None
    # - *No* pinpoint bisection job has been started for the alert group
    # - The issue does have components from the sheriff config assigned to it
    feature_flags.SANDWICH_VERIFICATION = True

    test_name = '/'.join(
        ["master", 'linux-perf', 'speedometer2', 'dummy', 'metric', 'parts'])
    anomalies = [
        self._AddAnomaly(test=test_name, statistic="made up"),
        self._AddAnomaly(test=test_name, statistic="also made up")
    ]

    group = self._AddAlertGroup(
        anomalies[0],
        subscription_name='Sandwich-Allowed Subscription',
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({'state': 'open'})
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='Sandwich-Allowed Subscription',
                auto_triage_enable=True,
                auto_bisect_enable=False,
                auto_merge_enable=True,
                bug_components=['should>set>component'])
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )

    # Not sure this should be the case...
    self.assertNotIn('Chromeperf-Auto-BisectOptOut',
                     self._issue_tracker.issue.get('labels'))
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))

    # Not sure this should be the case...
    self.assertNotIn('Chromeperf-Auto-BisectOptOut',
                     self._issue_tracker.issue.get('labels'))
    self.assertIn('should>set>component',
                  self._issue_tracker.issue.get('components'))
    self.assertNotEqual(w._group.status,
                        alert_group.AlertGroup.Status.sandwiched)
    self.assertIsNone(w._group.sandwich_verification_workflow_id)
    self.assertIsNone(self._pinpoint.new_job_request)
    workflow_anomaly = self._cloud_workflows.create_execution_called_with_anomaly
    self.assertIsNone(workflow_anomaly)

  def testSandwich_TryVerifyRegression_SANDWICH_VERIFICATION_disabled_noSandwichWorkflow(
      self):
    # Pre-coditions:
    # - feature_flags.SANDWICH_VERIFICATION is False
    # - New anomaly appears
    # - It's for a subscription is enabled for auto_triage and auto_bisect
    # - The anomaly's AlertGroup status is 'triaged`
    # Post-conditions:
    # - The anomaly's AlertGroup state is 'triaged'
    # - A "Sandwich Verification" *cloud* workflow has not been requested
    # - The AlertGroup's sandwich_verification_workflow_id is None
    # - A pinpoint bisection job has been started for the alert group
    # - The issue does not have components from the sandwich sheriff config assigned to it
    feature_flags.SANDWICH_VERIFICATION = False

    test_name = '/'.join(
        ["master", 'linux-perf', 'speedometer2', 'dummy', 'metric', 'parts'])
    anomalies = [
        self._AddAnomaly(test=test_name, statistic="made up"),
        self._AddAnomaly(test=test_name, statistic="also made up")
    ]

    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
        subscription_name='Sandwich-Allowed Subscription',
    )
    self._issue_tracker.issue.update({'state': 'open'})
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='Sandwich-Allowed Subscription',
                auto_triage_enable=True,
                auto_bisect_enable=True,
                bug_components=['should>set>component'])
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self.assertNotIn('Chromeperf-Auto-BisectOptOut',
                     self._issue_tracker.issue.get('labels'))
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertNotIn('Chromeperf-Auto-BisectOptOut',
                     self._issue_tracker.issue.get('labels'))
    self.assertIn('should>set>component',
                  self._issue_tracker.issue.get('components'))
    self.assertEqual(w._group.status, alert_group.AlertGroup.Status.bisected)
    self.assertIsNone(w._group.sandwich_verification_workflow_id)
    self.assertIsNotNone(self._pinpoint.new_job_request)
    workflow_anomaly = self._cloud_workflows.create_execution_called_with_anomaly
    self.assertIsNone(workflow_anomaly)
    feature_flags.SANDWICH_VERIFICATION = True

  def testSandwich_RegressionVerificationFailed_SkipBisection(self):
    # Pre-coditions:
    # - feature_flags.SANDWICH_VERIFICATION is True
    # - New anomaly appears
    # - It's for a subscription is enabled for auto_triage and auto_bisect
    # - The anomaly's AlertGroup status is 'sandwiched`
    # - The anomaly's benchmark/workload/config is allowed for sandwich verification
    # - The anomaly's AlertGroup has a sandwich_verification_workflow_id set
    # - Cloud Workflow service has an execution with that workflow id, and its state is FAILED
    # - The workflow execution has no results, just an error
    # Post-conditions:
    # - The anomaly's AlertGroup state is not 'bisected' (fail safe back to default behavior)
    # - A new "Sandwich Verification" *cloud* workflow has *not* been requested
    # - The AlertGroup's sandwich_verification_workflow_id is not changed
    # - The issue tracker has been called to update the bug label Regression-Verification-Failed
    #.  and status Unconfirmed.
    # - The issue does not have components from the sandwich sheriff config assigned to it
    feature_flags.SANDWICH_VERIFICATION = True

    test_name = '/'.join(
        ["master", 'linux-perf', 'speedometer2', 'dummy', 'metric', 'parts'])
    anomalies = [
        self._AddAnomaly(test=test_name, statistic="made up"),
        self._AddAnomaly(test=test_name, statistic="also made up")
    ]

    group = self._AddAlertGroup(
        anomalies[0],
        subscription_name='Sandwich-Allowed Subscription',
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.sandwiched,
    )
    self._issue_tracker.issue.update({'state': 'open'})
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='Sandwich-Allowed Subscription',
                auto_triage_enable=True,
                auto_bisect_enable=True,
                bug_components=['should>not>set>component'])
        ],
    }
    self._SandwichVerifyFailure(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self.assertNotIn('Chromeperf-Auto-BisectOptOut',
                     self._issue_tracker.issue.get('labels'))

    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertEqual([
        'Chromeperf-Auto-Closed', 'Chromeperf-Auto-Triaged', 'M-61', 'Pri-2',
        'Pri-3', 'Regression-Verification-Failed',
        'Restrict-View-Google', 'Type-Bug', 'Type-Bug-Regression'
    ], sorted(self._issue_tracker.issue.get('labels')))
    self.assertNotIn('should>not>set>component', self._issue_tracker.issue.get('components'))
    self.assertIsNotNone(w._group.sandwich_verification_workflow_id)
    self.assertIsNone(self._pinpoint.new_job_request)

    # First is a NewBug call in the test itself.
    self.assertEqual(len(self._issue_tracker.calls), 2)

    self.assertEqual(self._issue_tracker.calls[1]['method'], 'AddBugComment')
    self.assertEqual(len(self._issue_tracker.calls[1]['args']), 2)
    self.assertEqual(self._issue_tracker.calls[1]['args'][0],
                     self._issue_tracker.issue.get('id'))
    self.assertEqual(self._issue_tracker.calls[1]['args'][1], 'chromium')

    self.assertEqual(
        self._issue_tracker.calls[1]['kwargs'], {
            'comment':
                mock.ANY,
            'components': [],
            'status': 'WontFix',
            'labels':
                ['Chromeperf-Auto-Closed', 'Regression-Verification-Failed'],
            'send_email': False,
        })

    self.assertEqual(w._group.status, alert_group.AlertGroup.Status.closed)

  def testSandwich_RegressionVerification_NoRepro_skipBisect(self):
    # Pre-coditions:
    # - feature_flags.SANDWICH_VERIFICATION is True
    # - New anomaly appears
    # - It's for a subscription is enabled for auto_triage and auto_bisect
    # - The anomaly's AlertGroup status is 'sandwiched`
    # - The anomaly's benchmark/workload/config is allowed for sandwich verification
    # - The anomaly's AlertGroup has a sandwich_verification_workflow_id set
    # - Cloud Workflow service has an execution with that workflow id, and its state is SUCCEEDED
    # - The workflow resulted in decision: False, meaning it should skip bisection.
    # Post-conditions:
    # - The anomaly's AlertGroup state is now 'closed'
    # - A new "Sandwich Verification" *cloud* workflow has *not* been requested
    # - The AlertGroup's sandwich_verification_workflow_id is not changed
    # - A pinpoint bisection job has *not* been started for the alert group
    # - The issue tracker has beeb called to update associated issue as closed / WontFix and
    #   labels ['Regression-Verification-No-Repro', 'Chromeperf-Auto-Closed']
    # - The issue does not have components from the sandwich sheriff config assigned to it
    feature_flags.SANDWICH_VERIFICATION = True

    test_name = '/'.join(
        ["master", 'linux-perf', 'speedometer2', 'dummy', 'metric', 'parts'])
    anomalies = [
        self._AddAnomaly(test=test_name, statistic="made up"),
        self._AddAnomaly(test=test_name, statistic="also made up")
    ]

    group = self._AddAlertGroup(
        anomalies[0],
        subscription_name='Sandwich-Allowed Subscription',
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.sandwiched,
    )
    self._issue_tracker.issue.update({'state': 'open'})
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='Sandwich-Allowed Subscription',
                auto_triage_enable=True,
                auto_bisect_enable=True,
                bug_components=['should>not>set>component'])
        ],
    }
    self._SandwichVerify(group, anomalies, False)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self.assertNotIn('Chromeperf-Auto-BisectOptOut',
                     self._issue_tracker.issue.get('labels'))
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertEqual([
        'Chromeperf-Auto-Closed', 'Chromeperf-Auto-Triaged', 'M-61', 'Pri-2',
        'Pri-3', 'Regression-Verification-No-Repro', 'Restrict-View-Google',
        'Type-Bug', 'Type-Bug-Regression'
    ], sorted(self._issue_tracker.issue.get('labels')))
    self.assertEqual([], self._issue_tracker.issue.get('components'))
    self.assertIsNotNone(w._group.sandwich_verification_workflow_id)
    self.assertIsNone(self._pinpoint.new_job_request)

    # First is a NewBug call in the test itself.
    self.assertEqual(len(self._issue_tracker.calls), 2)

    self.assertEqual(self._issue_tracker.calls[1]['method'], 'AddBugComment')
    self.assertEqual(len(self._issue_tracker.calls[1]['args']), 2)
    self.assertEqual(self._issue_tracker.calls[1]['args'][0],
                     self._issue_tracker.issue.get('id'))
    self.assertEqual(self._issue_tracker.calls[1]['args'][1], 'chromium')

    self.assertEqual(
        self._issue_tracker.calls[1]['kwargs'], {
            'comment':
                mock.ANY,
            'status': 'WontFix',
            'labels':
                ['Chromeperf-Auto-Closed', 'Regression-Verification-No-Repro'],
            'send_email': False,
            'components': [],
        })

    self.assertEqual(w._group.status, alert_group.AlertGroup.Status.closed)

  def testSandwich_RegressionVerification_Repro_doBisect(self):
    # Pre-coditions:
    # - feature_flags.SANDWICH_VERIFICATION is True
    # - New anomaly appears
    # - Its subscription is enabled for auto_triage and auto_bisect
    # - The anomaly's AlertGroup status is 'sandwiched`
    # - The anomaly's benchmark/workload/config is allowed for sandwich verification
    # - The anomaly's AlertGroup has a sandwich_verification_workflow_id set
    # - Cloud Workflow service has an execution with that workflow id, and its state is SUCCEEDED
    # - The workflow resulted in decision: True, meaning we should proceed with auto-bisection.
    # Post-conditions:
    # - The anomaly's AlertGroup state is now 'bisected'
    # - A new "Sandwich Verification" *cloud* workflow has *not* been requested
    # - The AlertGroup's sandwich_verification_workflow_id is not changed
    # - A pinpoint bisection job has been started for the alert group
    # - The issue tracker has been called to update the issue with label Chromeperf-Auto-Bisected
    # - The issue does have components from the sandwich sheriff config assigned to it
    # - The bisect tags include "sandwiched: True"
    feature_flags.SANDWICH_VERIFICATION = True

    test_name = '/'.join(
        ["master", 'linux-perf', 'speedometer2', 'dummy', 'metric', 'parts'])

    anomalies = [
        self._AddAnomaly(test=test_name, statistic="made up"),
        self._AddAnomaly(test=test_name, statistic="also made up",
            ownership={'component': 'anomaly>should>not>set>component'})
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        subscription_name='Sandwich-Allowed Subscription',
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.sandwiched,
    )
    self._SandwichVerify(group, anomalies)
    self._issue_tracker.issue.update({'state': 'open'})

    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='Sandwich-Allowed Subscription',
                auto_triage_enable=True,
                auto_bisect_enable=True,
                bug_components=['sub>can>set>component'])
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self.assertNotIn('Chromeperf-Auto-BisectOptOut',
                     self._issue_tracker.issue.get('labels'))
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertNotIn('Chromeperf-Auto-BisectOptOut',
                     self._issue_tracker.issue.get('labels'))

    self.assertEqual(
        self._issue_tracker.issue.get('components'), ['sub>can>set>component'])
    self.assertIsNotNone(w._group.sandwich_verification_workflow_id)
    self.assertIsNotNone(self._pinpoint.new_job_request)
    tags = json.loads(self._pinpoint.new_job_request['tags'])
    self.assertEqual(tags['sandwiched'], 'true')

    # First is a NewBug call in the test itself.
    self.assertEqual(len(self._issue_tracker.calls), 3)

    self.assertEqual(self._issue_tracker.calls[1]['method'], 'AddBugComment')
    self.assertEqual(len(self._issue_tracker.calls[1]['args']), 2)
    self.assertEqual(self._issue_tracker.calls[1]['args'][0],
                     self._issue_tracker.issue.get('id'))
    self.assertEqual(self._issue_tracker.calls[1]['args'][1], 'chromium')

    self.assertEqual(
        self._issue_tracker.calls[1]['kwargs'], {
            'comment': mock.ANY,
            'labels': ['Regression-Verification-Repro'],
            'send_email': False,
            'status': 'Untriaged',
            'components': ['sub>can>set>component'],
        })
    self.assertEqual(w._group.status, alert_group.AlertGroup.Status.bisected)

  def testSandwich_RegressionVerification_Repro_doBisect_DelayReport(self):
    # Pre-coditions:
    # - feature_flags.SANDWICH_VERIFICATION is True
    # - New anomaly appears
    # - Its subscription is enabled for auto_triage and auto_bisect
    # - The anomaly's AlertGroup status is 'sandwiched`
    # - The anomaly's benchmark/workload/config is allowed for sandwich verification
    # - The anomaly's AlertGroup has a sandwich_verification_workflow_id set
    # - Cloud Workflow service has an execution with that workflow id, and its state is SUCCEEDED
    # - The workflow resulted in decision: True, meaning we should proceed with auto-bisection.
    # Post-conditions:
    # - The anomaly's AlertGroup state is now 'bisected'
    # - A new "Sandwich Verification" *cloud* workflow has *not* been requested
    # - The AlertGroup's sandwich_verification_workflow_id is not changed
    # - A pinpoint bisection job has been started for the alert group
    # - The Chromeperf-Delay-Reporting label is in the issue.
    # - The issue tracker has been called to update the issue with label Chromeperf-Auto-Bisected
    # - The issue does not have components from the sandwich sheriff config assigned to it
    # - The bisect tags include "sandwiched: True"
    feature_flags.SANDWICH_VERIFICATION = True

    test_name = '/'.join(
        ["master", 'linux-perf', 'speedometer2', 'dummy', 'metric', 'parts'])

    anomalies = [
        self._AddAnomaly(test=test_name, statistic="made up"),
        self._AddAnomaly(
            test=test_name,
            statistic="also made up",
            ownership={'component': 'anomaly>should>not>set>component'})
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        subscription_name='Sandwich-Allowed Subscription',
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.sandwiched,
    )
    self._SandwichVerify(group, anomalies)
    self._issue_tracker.issue.update({
        'state': 'open',
        'labels': [utils.DELAY_REPORTING_LABEL]
    })

    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='Sandwich-Allowed Subscription',
                auto_triage_enable=True,
                auto_bisect_enable=True,
                bug_components=['should>not>set>component'])
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self.assertNotIn('Chromeperf-Auto-BisectOptOut',
                     self._issue_tracker.issue.get('labels'))
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertNotIn('Chromeperf-Auto-BisectOptOut',
                     self._issue_tracker.issue.get('labels'))

    self.assertNotIn('should>not>set>component',
                     self._issue_tracker.issue.get('components'))
    self.assertIsNotNone(w._group.sandwich_verification_workflow_id)
    self.assertIsNotNone(self._pinpoint.new_job_request)
    tags = json.loads(self._pinpoint.new_job_request['tags'])
    self.assertEqual(tags['sandwiched'], 'true')

    # First is a NewBug call in the test itself.
    self.assertEqual(len(self._issue_tracker.calls), 3)

    self.assertEqual(self._issue_tracker.calls[1]['method'], 'AddBugComment')
    self.assertEqual(len(self._issue_tracker.calls[1]['args']), 2)
    self.assertEqual(self._issue_tracker.calls[1]['args'][0],
                     self._issue_tracker.issue.get('id'))
    self.assertEqual(self._issue_tracker.calls[1]['args'][1], 'chromium')

    self.assertEqual(
        self._issue_tracker.calls[1]['kwargs'], {
            'comment': mock.ANY,
            'labels': ['Regression-Verification-Repro'],
            'send_email': False,
            'status': 'Untriaged',
            'components': [],
        })
    self.assertEqual(w._group.status, alert_group.AlertGroup.Status.bisected)

  def testSandwich_TryVerifyRegression_BisectOptOut_skipBisect(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        subscription_name='Sandwich-Allowed Subscription',
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state':
            'open',
        'labels':
            self._issue_tracker.issue.get('labels') +
            ['Chromeperf-Auto-BisectOptOut']
    })
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
    )
    self.assertIn('Chromeperf-Auto-BisectOptOut',
                  self._issue_tracker.issue.get('labels'))

    update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        )
    res = w._TryVerifyRegression(update)
    self.assertFalse(res)
    self.assertIsNone(self._pinpoint.new_job_request)

  def testBisect_GroupTriaged_WithSummary(self):
    anomalies = [
        self._AddAnomaly(
            test='master/bot1/test_suite/measurement/test_case1',
            median_before_anomaly=0.2,
        ),
        self._AddAnomaly(
            test='master/bot1/test_suite/measurement/test_case2',
            median_before_anomaly=0.1,
            is_summary=True,
        ),
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    tags = json.loads(self._pinpoint.new_job_request['tags'])
    self.assertEqual(six.ensure_str(anomalies[0].urlsafe()), tags['alert'])

    # Tags must be a dict of key/value string pairs.
    for k, v in tags.items():
      self.assertIsInstance(k, six.string_types)
      self.assertIsInstance(v, six.string_types)

    self.assertEqual(['123456'], group.get().bisection_ids)
    self.assertEqual(['Chromeperf-Auto-Bisected'],
                     self._issue_tracker.add_comment_kwargs['labels'])

  def testBisect_GroupTriaged_WithSignalQuality(self):
    anomalies = [
        self._AddAnomaly(
            test='master/bot/test_suite/measurement/test_case1',
            median_before_anomaly=0.2,
        ),
        self._AddAnomaly(
            test='master/bot/test_suite/measurement/test_case2',
            median_before_anomaly=0.1,
        ),
    ]
    self._AddSignalQualityScore(anomalies[0], 0.9)
    self._AddSignalQualityScore(anomalies[1], 0.8)
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    tags = json.loads(self._pinpoint.new_job_request['tags'])
    self.assertEqual(six.ensure_str(anomalies[0].urlsafe()), tags['alert'])


  def testBisect_GroupTriaged_WithDefaultSignalQuality(self):
    anomalies = [
        self._AddAnomaly(
            test='master/bot/test_suite/measurement/test_case1',
            median_before_anomaly=0.1,
        ),
        self._AddAnomaly(
            test='master/bot/test_suite/measurement/test_case2',
            median_before_anomaly=0.2,
        ),
        self._AddAnomaly(
            test='master/bot/test_suite/measurement/test_case3',
            median_before_anomaly=0.3,
        ),
    ]
    self._AddSignalQualityScore(anomalies[0], 0.3)
    self._AddSignalQualityScore(anomalies[1], 0.2)
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    tags = json.loads(self._pinpoint.new_job_request['tags'])
    self.assertEqual(six.ensure_str(anomalies[2].urlsafe()), tags['alert'])

    # Tags must be a dict of key/value string pairs.
    for k, v in tags.items():
      self.assertIsInstance(k, six.string_types)
      self.assertIsInstance(v, six.string_types)

    self.assertEqual(['123456'], group.get().bisection_ids)
    self.assertEqual(['Chromeperf-Auto-Bisected'],
                     self._issue_tracker.add_comment_kwargs['labels'])

  def testBisect_GroupTriaged_MultiSubscriptions(self):
    anomalies = [
        self._AddAnomaly(median_before_anomaly=0.2),
        self._AddAnomaly(median_before_anomaly=0.1),
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff'),
            subscription.Subscription(
                name='sheriff_not_bind',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertIsNone(self._pinpoint.new_job_request)

  def testBisect_GroupBisected(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.bisected,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertIsNone(self._pinpoint.new_job_request)

  def testBisect_GroupTriaged_NoRecovered(self):
    anomalies = [
        self._AddAnomaly(
            median_before_anomaly=0.1, median_after_anomaly=1.0,
            recovered=True),
        self._AddAnomaly(median_before_anomaly=0.2, median_after_anomaly=1.0),
    ]
    group = self._AddAlertGroup(
        anomalies[1],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
        anomalies=anomalies,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertIsNotNone(self._pinpoint.new_job_request)
    self.assertEqual(group.get().status, alert_group.AlertGroup.Status.bisected)

    # Check that we bisected the anomaly that is not recovered.
    recovered_anomaly = anomalies[0].get()
    bisected_anomaly = anomalies[1].get()
    self.assertNotEqual(recovered_anomaly.pinpoint_bisects, ['123456'])
    self.assertEqual(bisected_anomaly.pinpoint_bisects, ['123456'])

  def testBisect_GroupTriaged_NoIgnored(self):
    anomalies = [
        # This anomaly is manually ignored.
        self._AddAnomaly(
            median_before_anomaly=0.1, median_after_anomaly=1.0, bug_id=-2),
        self._AddAnomaly(
            median_before_anomaly=0.2,
            median_after_anomaly=1.0,
            start_revision=20),
    ]
    group = self._AddAlertGroup(
        anomalies[1],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
        anomalies=anomalies,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertIsNotNone(self._pinpoint.new_job_request)
    self.assertEqual(self._pinpoint.new_job_request['bug_id'], 12345)
    self.assertEqual(group.get().status, alert_group.AlertGroup.Status.bisected)

    # Check that we bisected the anomaly that is not ignored.
    ignored_anomaly = anomalies[0].get()
    bisected_anomaly = anomalies[1].get()
    self.assertNotEqual(ignored_anomaly.pinpoint_bisects, ['123456'])
    self.assertEqual(bisected_anomaly.pinpoint_bisects, ['123456'])

  def testBisect_GroupTriaged_AlertWithBug(self):
    anomalies = [
        self._AddAnomaly(median_before_anomaly=0.2),
        self._AddAnomaly(
            median_before_anomaly=0.1,
            bug_id=12340,
            project_id='v8',
        ),
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertEqual(self._issue_tracker.bug_id,
                     self._pinpoint.new_job_request['bug_id'])
    self.assertEqual('chromium', self._pinpoint.new_job_request['project'])
    self.assertEqual(['123456'], group.get().bisection_ids)

  def testBisect_GroupTriaged_MultiBot(self):
    anomalies = [
        self._AddAnomaly(
            test='master/bot1/test_suite/measurement/test_case1',
            median_before_anomaly=0.3,
        ),
        self._AddAnomaly(
            test='master/bot1/test_suite/measurement/test_case2',
            median_before_anomaly=0.2,
        ),
        self._AddAnomaly(
            test='master/bot2/test_suite/measurement/test_case2',
            median_before_anomaly=0.1,
        ),
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertEqual(
        six.ensure_str(anomalies[1].urlsafe()),
        json.loads(self._pinpoint.new_job_request['tags'])['alert'])
    self.assertEqual(['123456'], group.get().bisection_ids)

  def testBisect_GroupTriaged_MultiBot_PartInf(self):
    anomalies = [
        self._AddAnomaly(
            test='master/bot1/test_suite/measurement/test_case1',
            median_before_anomaly=0.0,
        ),
        self._AddAnomaly(
            test='master/bot1/test_suite/measurement/test_case2',
            median_before_anomaly=0.2,
        ),
        self._AddAnomaly(
            test='master/bot2/test_suite/measurement/test_case2',
            median_before_anomaly=0.1,
        ),
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertEqual(
        six.ensure_str(anomalies[1].urlsafe()),
        json.loads(self._pinpoint.new_job_request['tags'])['alert'])
    self.assertEqual(['123456'], group.get().bisection_ids)

  def testBisect_GroupTriaged_MultiBot_AllInf(self):
    anomalies = [
        self._AddAnomaly(
            test='master/bot1/test_suite/measurement/test_case1',
            median_before_anomaly=0.0,
            median_after_anomaly=1.0,
        ),
        self._AddAnomaly(
            test='master/bot1/test_suite/measurement/test_case2',
            median_before_anomaly=0.0,
            median_after_anomaly=2.0,
        ),
        self._AddAnomaly(
            test='master/bot2/test_suite/measurement/test_case2',
            median_before_anomaly=0.1,
        ),
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertEqual(
        six.ensure_str(anomalies[1].urlsafe()),
        json.loads(self._pinpoint.new_job_request['tags'])['alert'])
    self.assertEqual(['123456'], group.get().bisection_ids)

  def testBisect_GroupTriaged_AlertBisected(self):
    anomalies = [
        self._AddAnomaly(
            test='master/bot1/test_suite/measurement/test_case1',
            pinpoint_bisects=['abcdefg'],
            median_before_anomaly=0.2,
        ),
        self._AddAnomaly(
            test='master/bot1/test_suite/measurement/test_case2',
            pinpoint_bisects=['abcdef'],
            median_before_anomaly=0.1,
        ),
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
        bisection_ids=['abcdef'],
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertEqual(
        six.ensure_str(anomalies[0].urlsafe()),
        json.loads(self._pinpoint.new_job_request['tags'])['alert'])
    self.assertEqual(['abcdef', '123456'], group.get().bisection_ids)

  def testBisect_GroupTriaged_CrrevFailed(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._crrev.SetFailure()
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertEqual(alert_group.AlertGroup.Status.bisected, group.get().status)
    self.assertEqual([], group.get().bisection_ids)
    self.assertEqual(['Chromeperf-Auto-NeedsAttention'],
                     self._issue_tracker.add_comment_kwargs['labels'])

  def testBisect_GroupTriaged_PinpointFailed(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state': 'open',
    })
    self._pinpoint.SetFailure()
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
    )
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertEqual(alert_group.AlertGroup.Status.bisected, group.get().status)
    self.assertEqual([], group.get().bisection_ids)
    self.assertEqual(['Chromeperf-Auto-NeedsAttention'],
                     self._issue_tracker.add_comment_kwargs['labels'])

  def testBisect_SingleCL(self):
    anomalies = [
        self._AddAnomaly(
            # Current implementation requires that a revision string is between
            # 5 and 7 digits long.
            start_revision=11111,
            end_revision=11111,
            test='ChromiumPerf/some-bot/some-benchmark/some-metric/some-story')
    ]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged)
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ]
    }
    # Here we are simulating that a gitiles service will respond to a specific
    # repository URL (the format is not important) and can map a commit (40
    # hexadecimal characters) to some commit information.
    self._gitiles._repo_commit_list.update({
        'git://chromium': {
            'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa': {
                'author': {
                    'email': 'author@chromium.org',
                },
                'message': 'This is some commit.\n\nWith some details.',
            }
        }
    })

    # We are also seeding some repository information to let us set which
    # repository URL is being used to look up data from a gitiles service.
    namespaced_stored_object.Set('repositories', {
        'chromium': {
            'repository_url': 'git://chromium'
        },
    })

    # Current implementation requires that a git hash is 40 characters of
    # hexadecimal digits.
    self._crrev.SetSuccess('aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa')
    self._SandwichVerify(group, anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
        cloud_workflows=self._cloud_workflows,
        gitiles=self._gitiles)
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue))
    self.assertEqual(alert_group.AlertGroup.Status.bisected, group.get().status)
    self.assertEqual([], group.get().bisection_ids)
    self.assertEqual(['Chromeperf-Auto-Assigned'],
                     self._issue_tracker.add_comment_kwargs['labels'])
    self.assertIn(('Assigning to author@chromium.org because this is the '
                   'only CL in range:'),
                  self._issue_tracker.add_comment_kwargs['comment'])

  def testBisect_ExplicitOptOut(self):
    anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        anomalies[0],
        issue=self._issue_tracker.issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._issue_tracker.issue.update({
        'state':
            'open',
        'labels':
            self._issue_tracker.issue.get('labels') +
            ['Chromeperf-Auto-BisectOptOut']
    })
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                auto_triage_enable=True,
                auto_bisect_enable=True)
        ],
    }
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        pinpoint=self._pinpoint,
        crrev=self._crrev,
    )
    self.assertIn('Chromeperf-Auto-BisectOptOut',
                  self._issue_tracker.issue.get('labels'))
    self._UpdateTwice(
        workflow=w,
        update=alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
            now=datetime.datetime.utcnow(),
            anomalies=ndb.get_multi(anomalies),
            issue=self._issue_tracker.issue,
        ))
    self.assertIsNone(self._pinpoint.new_job_request)

  def testAutoMerge_SucessfulMerge(self):
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff',
                bug_components=['Foo>Bar'],
                auto_triage_enable=True,
                auto_merge_enable=True)
        ],
    }
    self._issue_tracker._bug_id_counter = 42
    duplicate_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug(status='Duplicate',
                                   state='closed')['issue_id'])
    canonical_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug()['issue_id'])

    grouped_anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    all_anomalies = grouped_anomalies + [self._AddAnomaly()]
    group = self._AddAlertGroup(
        grouped_anomalies[0],
        issue=duplicate_issue,
        anomalies=grouped_anomalies,
        status=alert_group.AlertGroup.Status.triaged,
    )
    canonical_group = self._AddAlertGroup(
        grouped_anomalies[0],
        issue=canonical_issue,
        status=alert_group.AlertGroup.Status.triaged,
    )

    self._SandwichVerify(group, grouped_anomalies)
    self._SandwichVerify(canonical_group, grouped_anomalies)

    # if both issues are now in 'sandwiched' state, we'd expect the
    # next Workflow update to call _TryVerifyRegression on them.
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
        cloud_workflows=self._cloud_workflows,
    )
    u = alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
        now=datetime.datetime.utcnow(),
        anomalies=ndb.get_multi(all_anomalies),
        issue=duplicate_issue,
        canonical_group=canonical_group.get(),
    )

    w.Process(update=u)

    # First two are NewBug calls in the test itself.
    self.assertEqual(len(self._issue_tracker.calls), 4)

    self.assertEqual(self._issue_tracker.calls[2]['method'], 'AddBugComment')
    self.assertEqual(len(self._issue_tracker.calls[2]['args']), 2)
    self.assertEqual(self._issue_tracker.calls[2]['args'][0], 42)
    self.assertEqual(self._issue_tracker.calls[2]['args'][1], 'chromium')
    self.assertIn(
        '(%s) was automatically merged into %s' %
        (group.string_id(), canonical_group.string_id()),
        self._issue_tracker.calls[2]['kwargs']['comment'])

    self.assertEqual(self._issue_tracker.calls[2]['kwargs'], {
        'comment': mock.ANY,
        'send_email': False
    })

    self.assertEqual(
        self._issue_tracker.calls[3], {
            'method': 'AddBugComment',
            'args': (42, 'chromium'),
            'kwargs': {
                'title':
                    '[%s]: [%d] regressions in %s' %
                    ('sheriff', 3, 'test_suite'),
                'labels': [
                    'Chromeperf-Auto-Triaged',
                    'Pri-2',
                    'Restrict-View-Google',
                    'Type-Bug-Regression',
                ],
                'cc': [],
                'comment':
                    None,
                'components': ['Foo>Bar'],
                'send_email':
                    False
            },
        })

    self.assertTrue(all(a.get().bug_id == 43 for a in all_anomalies))
    self.assertEqual(group.get().canonical_group, canonical_group)
    self.assertEqual(group.get().status, alert_group.AlertGroup.Status.closed)

  def testAutoMerge_AutoMergeNotOptIn(self):
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }

    self._issue_tracker._bug_id_counter = 42
    duplicate_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug(status='Duplicate',
                                   state='closed')['issue_id'])
    canonical_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug()['issue_id'])

    grouped_anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    all_anomalies = grouped_anomalies + [self._AddAnomaly()]
    group = self._AddAlertGroup(
        grouped_anomalies[0],
        issue=duplicate_issue,
        anomalies=grouped_anomalies,
        status=alert_group.AlertGroup.Status.triaged,
    )
    canonical_group = self._AddAlertGroup(
        grouped_anomalies[0],
        issue=canonical_issue,
        status=alert_group.AlertGroup.Status.triaged,
    )

    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
    )
    u = alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
        now=datetime.datetime.utcnow(),
        anomalies=ndb.get_multi(all_anomalies),
        issue=duplicate_issue,
        canonical_group=canonical_group.get(),
    )

    self._UpdateTwice(workflow=w, update=u)

    # First two are NewBug calls in the test itself.
    self.assertEqual(len(self._issue_tracker.calls), 3)
    self.assertEqual(self._issue_tracker.calls[2]['method'], 'AddBugComment')
    self.assertEqual(len(self._issue_tracker.calls[2]['args']), 2)
    self.assertEqual(self._issue_tracker.calls[2]['args'][0], 42)
    self.assertEqual(self._issue_tracker.calls[2]['args'][1], 'chromium')
    self.assertNotIn('was automatically merged into',
                     self._issue_tracker.calls[2]['kwargs']['comment'])
    self.assertIn('Alert group updated:',
                  self._issue_tracker.calls[2]['kwargs']['comment'])
    self.assertEqual(
        self._issue_tracker.calls[2]['kwargs'], {
            'title':
                '[%s]: [%d] regressions in %s' % ('sheriff', 3, 'test_suite'),
            'labels': [
                'Chromeperf-Auto-Triaged',
                'Pri-2',
                'Restrict-View-Google',
                'Type-Bug-Regression',
            ],
            'cc': [],
            'components': ['Foo>Bar'],
            'comment':
                mock.ANY,
            'send_email':
                False
        })

    self.assertTrue(all(a.get().bug_id == 42 for a in all_anomalies))
    self.assertIsNone(group.get().canonical_group)
    self.assertEqual(group.get().status, alert_group.AlertGroup.Status.closed)

  def testAutoMerge_NoCanonicalIssue(self):
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff', auto_triage_enable=True, auto_merge_enable=True)
        ],
    }

    self._issue_tracker._bug_id_counter = 42
    issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug()['issue_id'])

    grouped_anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    all_anomalies = grouped_anomalies + [self._AddAnomaly()]
    group = self._AddAlertGroup(
        grouped_anomalies[0],
        issue=issue,
        anomalies=grouped_anomalies,
        status=alert_group.AlertGroup.Status.triaged,
    )

    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
    )
    u = alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
        now=datetime.datetime.utcnow(),
        anomalies=ndb.get_multi(all_anomalies),
        issue=issue,
    )

    self._UpdateTwice(workflow=w, update=u)

    # First one is NewBug call in the test itself.
    self.assertEqual(len(self._issue_tracker.calls), 2)

    self.assertEqual(self._issue_tracker.calls[1]['method'], 'AddBugComment')
    self.assertEqual(len(self._issue_tracker.calls[1]['args']), 2)
    self.assertEqual(self._issue_tracker.calls[1]['args'][0], 42)
    self.assertEqual(self._issue_tracker.calls[1]['args'][1], 'chromium')
    self.assertNotIn('was automatically merged into',
                     self._issue_tracker.calls[1]['kwargs']['comment'])
    self.assertIn('Alert group updated:',
                  self._issue_tracker.calls[1]['kwargs']['comment'])
    self.assertEqual(
        self._issue_tracker.calls[1]['kwargs'], {
            'title':
                '[%s]: [%d] regressions in %s' % ('sheriff', 3, 'test_suite'),
            'labels': [
                'Chromeperf-Auto-Triaged',
                'Pri-2',
                'Restrict-View-Google',
                'Type-Bug-Regression',
            ],
            'cc': [],
            'components': ['Foo>Bar'],
            'comment':
                mock.ANY,
            'send_email':
                False
        })

    self.assertTrue(all(a.get().bug_id == 42 for a in all_anomalies))
    self.assertIsNone(group.get().canonical_group)
    self.assertEqual(group.get().status, alert_group.AlertGroup.Status.triaged)

  def testAutoMerge_SandwichSubsMerged(self):
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='blocked-sheriff',
                auto_triage_enable=True,
                auto_merge_enable=True),
        ],
    }

    self._issue_tracker._bug_id_counter = 42
    duplicate_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug(status='Duplicate',
                                   state='closed')['issue_id'])
    canonical_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug()['issue_id'])

    grouped_anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    all_anomalies = grouped_anomalies + [self._AddAnomaly()]
    group = self._AddAlertGroup(
        grouped_anomalies[0],
        subscription_name='blocked-sheriff',
        issue=duplicate_issue,
        anomalies=grouped_anomalies,
        status=alert_group.AlertGroup.Status.triaged,
    )
    canonical_group = self._AddAlertGroup(
        grouped_anomalies[0],
        issue=canonical_issue,
        status=alert_group.AlertGroup.Status.triaged,
    )

    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
    )
    u = alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
        now=datetime.datetime.utcnow(),
        anomalies=ndb.get_multi(all_anomalies),
        issue=duplicate_issue,
        canonical_group=canonical_group.get(),
    )

    w.Process(update=u)

    # First two are NewBug calls in the test itself.
    self.assertEqual(len(self._issue_tracker.calls), 4)

    # Create the canonical issue
    self.assertEqual(self._issue_tracker.calls[2]['method'], 'AddBugComment')
    self.assertEqual(len(self._issue_tracker.calls[2]['args']), 2)
    self.assertEqual(self._issue_tracker.calls[2]['args'][0], 42)
    self.assertEqual(self._issue_tracker.calls[2]['args'][1], 'chromium')

    self.assertEqual(
        self._issue_tracker.calls[3], {
            'method': 'AddBugComment',
            'args': (42, 'chromium'),
            'kwargs': {
                'title': '[blocked-sheriff]: [3] regressions in test_suite',
                'labels': [
                    'Chromeperf-Auto-Triaged',
                    'Pri-2',
                    'Restrict-View-Google',
                    'Type-Bug-Regression',
                ],
                'cc': [],
                'components': ['Foo>Bar'],
                'comment': mock.ANY,
                'send_email': False
            },
        })

    # Verify that it did set the 'duplicate' sandwiched alert group's canonical group.
    self.assertIsNotNone(group.get().canonical_group)
    self.assertEqual(group.get().status, alert_group.AlertGroup.Status.closed)

  def testAutoMerge_SucessfulMerge_AutoMergeForOneAnomaly(self):
    self._sheriff_config.patterns = {
        '*auto_merge*': [
            subscription.Subscription(
                name='sheriff', auto_triage_enable=True, auto_merge_enable=True)
        ],
        '*regular*': [
            subscription.Subscription(name='sheriff', auto_triage_enable=True)
        ],
    }

    self._issue_tracker._bug_id_counter = 42
    duplicate_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug(status='Duplicate',
                                   state='closed')['issue_id'])
    canonical_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug()['issue_id'])

    grouped_anomalies = [
        self._AddAnomaly(test='master/bot/regular_suite/measurement'),
        self._AddAnomaly(test='master/bot/auto_merge_suite/measurement')
    ]
    all_anomalies = grouped_anomalies + [
        self._AddAnomaly(test='master/bot/regular_suite/measurement'),
    ]
    group = self._AddAlertGroup(
        grouped_anomalies[0],
        issue=duplicate_issue,
        anomalies=grouped_anomalies,
        status=alert_group.AlertGroup.Status.triaged,
    )
    canonical_group = self._AddAlertGroup(
        grouped_anomalies[0],
        issue=canonical_issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    self._SandwichVerify(group, grouped_anomalies)
    self._SandwichVerify(canonical_group, grouped_anomalies)
    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
        cloud_workflows=self._cloud_workflows,
    )
    u = alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
        now=datetime.datetime.utcnow(),
        anomalies=ndb.get_multi(all_anomalies),
        issue=duplicate_issue,
        canonical_group=canonical_group.get(),
    )

    w.Process(update=u)

    # First two are NewBug calls in the test itself.
    self.assertEqual(len(self._issue_tracker.calls), 4)

    self.assertEqual(self._issue_tracker.calls[2]['method'], 'AddBugComment')
    self.assertEqual(len(self._issue_tracker.calls[2]['args']), 2)
    self.assertEqual(self._issue_tracker.calls[2]['args'][0], 42)
    self.assertEqual(self._issue_tracker.calls[2]['args'][1], 'chromium')
    self.assertIn(
        '(%s) was automatically merged into %s' %
        (group.string_id(), canonical_group.string_id()),
        self._issue_tracker.calls[2]['kwargs']['comment'])
    self.assertEqual(self._issue_tracker.calls[2]['kwargs'], {
        'comment': mock.ANY,
        'send_email': False
    })

    self.assertEqual(
        self._issue_tracker.calls[3], {
            'method': 'AddBugComment',
            'args': (42, 'chromium'),
            'kwargs': {
                'title':
                    '[%s]: [%d] regressions in %s' %
                    ('sheriff', 3, 'regular_suite'),
                'labels': [
                    'Chromeperf-Auto-Triaged',
                    'Pri-2',
                    'Restrict-View-Google',
                    'Type-Bug-Regression',
                ],
                'cc': [],
                'components': ['Foo>Bar'],
                'comment':
                    None,
                'send_email':
                    False
            },
        })

    self.assertEqual(all_anomalies[0].get().bug_id, 42)
    self.assertEqual(all_anomalies[1].get().bug_id, 43)
    self.assertEqual(all_anomalies[2].get().bug_id, 42)

    self.assertEqual(group.get().canonical_group, canonical_group)
    self.assertEqual(group.get().status, alert_group.AlertGroup.Status.closed)

  def testAutoMerge_SucessfulMerge_NoNewAnomalies(self):
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff', auto_triage_enable=True, auto_merge_enable=True)
        ],
    }

    self._issue_tracker._bug_id_counter = 42
    duplicate_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug(status='Duplicate',
                                   state='closed')['issue_id'])
    canonical_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug()['issue_id'])

    grouped_anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    group = self._AddAlertGroup(
        grouped_anomalies[0],
        issue=duplicate_issue,
        anomalies=grouped_anomalies,
        status=alert_group.AlertGroup.Status.triaged,
    )
    canonical_group = self._AddAlertGroup(
        grouped_anomalies[0],
        issue=canonical_issue,
        status=alert_group.AlertGroup.Status.triaged,
    )

    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
    )
    u = alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
        now=datetime.datetime.utcnow(),
        anomalies=ndb.get_multi(grouped_anomalies),
        issue=duplicate_issue,
        canonical_group=canonical_group.get(),
    )

    w.Process(update=u)

    # First two are NewBug calls in the test itself.
    self.assertEqual(len(self._issue_tracker.calls), 3)

    self.assertEqual(self._issue_tracker.calls[2]['method'], 'AddBugComment')
    self.assertEqual(len(self._issue_tracker.calls[2]['args']), 2)
    self.assertEqual(self._issue_tracker.calls[2]['args'][0], 42)
    self.assertEqual(self._issue_tracker.calls[2]['args'][1], 'chromium')
    self.assertIn(
        '(%s) was automatically merged into %s' %
        (group.string_id(), canonical_group.string_id()),
        self._issue_tracker.calls[2]['kwargs']['comment'])
    self.assertEqual(self._issue_tracker.calls[2]['kwargs'], {
        'comment': mock.ANY,
        'send_email': False
    })

    self.assertTrue(all(a.get().bug_id == 43 for a in grouped_anomalies))
    self.assertEqual(group.get().canonical_group, canonical_group)
    self.assertEqual(group.get().status, alert_group.AlertGroup.Status.closed)

  def testAutoMerge_SeparatingGroups(self):
    self._sheriff_config.patterns = {
        '*': [
            subscription.Subscription(
                name='sheriff', auto_triage_enable=True, auto_merge_enable=True)
        ],
    }

    self._issue_tracker._bug_id_counter = 42
    duplicate_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug()['issue_id'])

    grouped_anomalies = [self._AddAnomaly(), self._AddAnomaly()]
    all_anomalies = grouped_anomalies + [self._AddAnomaly()]
    canonical_group = self._AddAlertGroup(
        grouped_anomalies[0],
        status=alert_group.AlertGroup.Status.triaged,
    )
    group = self._AddAlertGroup(
        grouped_anomalies[0],
        issue=duplicate_issue,
        anomalies=grouped_anomalies,
        status=alert_group.AlertGroup.Status.closed,
        canonical_group=canonical_group)

    w = alert_group_workflow.AlertGroupWorkflow(
        group.get(),
        sheriff_config=self._sheriff_config,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
    )
    u = alert_group_workflow.AlertGroupWorkflow.GroupUpdate(
        now=datetime.datetime.utcnow(),
        anomalies=ndb.get_multi(all_anomalies),
        issue=duplicate_issue,
        canonical_group=None,
    )

    w.Process(update=u)

    # First one is NewBug calls in the test itself.
    self.assertEqual(len(self._issue_tracker.calls), 2)

    self.assertEqual(self._issue_tracker.calls[1]['method'], 'AddBugComment')
    self.assertEqual(len(self._issue_tracker.calls[1]['args']), 2)
    self.assertEqual(self._issue_tracker.calls[1]['args'][0], 42)
    self.assertIn('Alert group updated:',
                  self._issue_tracker.calls[1]['kwargs']['comment'])

    self.assertIsNone(group.get().canonical_group)
    self.assertEqual(group.get().status, alert_group.AlertGroup.Status.triaged)

  def testPrepareGroupUpdate_DuplicateGroupFound(self):
    base_anomaly = self._AddAnomaly()

    self._issue_tracker._bug_id_counter = 42
    canonical_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug()['issue_id'])
    canonical_group = self._AddAlertGroup(
        base_anomaly,
        issue=canonical_issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    canonical_anomalies = [
        self._AddAnomaly(groups=[canonical_group]),
        self._AddAnomaly(groups=[canonical_group])
    ]

    duplicate_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug(status='Duplicate',
                                   state='closed')['issue_id'])
    duplicate_group = self._AddAlertGroup(
        base_anomaly,
        issue=duplicate_issue,
        status=alert_group.AlertGroup.Status.triaged,
        canonical_group=canonical_group,
    )
    duplicate_anomalies = [
        self._AddAnomaly(groups=[duplicate_group]),
        self._AddAnomaly(groups=[duplicate_group])
    ]

    w = alert_group_workflow.AlertGroupWorkflow(
        canonical_group.get(),
        sheriff_config=self._sheriff_config,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
    )

    update = w._PrepareGroupUpdate()

    self.assertEqual(
        update.anomalies,
        [a.get() for a in canonical_anomalies + duplicate_anomalies])
    self.assertIsNotNone(update.issue)
    self.assertIsNone(update.canonical_group)

  def testPrepareGroupUpdate_CanonicalGroupFound(self):
    base_anomaly = self._AddAnomaly()

    self._issue_tracker._bug_id_counter = 42

    canonical_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug()['issue_id'])
    canonical_group = self._AddAlertGroup(
        base_anomaly,
        issue=canonical_issue,
        status=alert_group.AlertGroup.Status.triaged,
    )

    duplicate_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug(
            status='Duplicate',
            state='closed',
            mergedInto={'issueId': canonical_issue['id']})['issue_id'])
    duplicate_group = self._AddAlertGroup(
        base_anomaly,
        issue=duplicate_issue,
        status=alert_group.AlertGroup.Status.triaged,
    )
    anomalies = [
        self._AddAnomaly(groups=[duplicate_group]),
        self._AddAnomaly(groups=[duplicate_group])
    ]

    w = alert_group_workflow.AlertGroupWorkflow(
        duplicate_group.get(),
        sheriff_config=self._sheriff_config,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
    )
    update = w._PrepareGroupUpdate()

    self.assertEqual(update.anomalies, [a.get() for a in anomalies])
    self.assertIsNotNone(update.issue)
    self.assertEqual(update.canonical_group, canonical_group.get())

  def testPrepareGroupUpdate_CanonicalGroupLoop(self):
    base_anomaly = self._AddAnomaly()

    self._issue_tracker._bug_id_counter = 42
    duplicate_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug(status='Duplicate',
                                   state='closed')['issue_id'])
    duplicate_group = self._AddAlertGroup(
        base_anomaly,
        issue=duplicate_issue,
        status=alert_group.AlertGroup.Status.triaged,
    )

    looped_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug()['issue_id'])
    looped_group = self._AddAlertGroup(
        base_anomaly,
        issue=looped_issue,
        status=alert_group.AlertGroup.Status.triaged,
        canonical_group=duplicate_group,
    )

    canonical_issue = self._issue_tracker.GetIssue(
        self._issue_tracker.NewBug()['issue_id'])
    self._AddAlertGroup(
        base_anomaly,
        issue=canonical_issue,
        status=alert_group.AlertGroup.Status.triaged,
        canonical_group=looped_group,
    )

    self._issue_tracker.issue_comments.update({
        ('chromium', duplicate_issue['id']): [{
            'id': 2,
            'updates': {
                'status': 'Duplicate',
                # According to Monorail API documentation, mergedInto
                # has string type.
                'mergedInto': str(canonical_issue['id'])
            },
        }]
    })

    w = alert_group_workflow.AlertGroupWorkflow(
        duplicate_group.get(),
        sheriff_config=self._sheriff_config,
        config=alert_group_workflow.AlertGroupWorkflow.Config(
            active_window=datetime.timedelta(days=7),
            triage_delay=datetime.timedelta(hours=0),
        ),
    )

    update = w._PrepareGroupUpdate()

    self.assertIsNone(update.canonical_group)
