# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Tests the culprit verification results update Handler."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from unittest import mock
import json

from dashboard.pinpoint import test
from dashboard.pinpoint.models import sandwich_workflow_group
from dashboard.services import workflow_service
from google.appengine.ext import ndb


class FakeResponse():

  def __init__(self, state, result):
    self.state = state
    self.result = result


@mock.patch('dashboard.services.perf_issue_service_client.PostIssueComment',
            mock.MagicMock())
class CulpritTest(test.TestCase):

  @mock.patch('dashboard.services.workflow_service.GetExecution',
              mock.MagicMock(
                  return_value={
                      'state': workflow_service.EXECUTION_STATE_SUCCEEDED,
                      'result': json.dumps({'decision': False})
                  }))
  def testCulpritVerificationAllExecutionCompletedZeroVerified(self):
    sandwich_workflow_group.SandwichWorkflowGroup(
        key=ndb.Key('SandwichWorkflowGroup', 'group1'),
        metric='test_metric',
        url='test_url',
        active=True,
        cloud_workflows_keys=[1]).put()
    sandwich_workflow_group.CloudWorkflow(
        key=ndb.Key('CloudWorkflow', 1), execution_name='execution-id-0').put()
    response = self.testapp.get('/cron/update-culprit-verification-results')
    self.assertEqual(response.status_code, 200)
    updated_cloud_workflow = ndb.Key('CloudWorkflow', 1).get()
    self.assertEqual(updated_cloud_workflow.execution_status, 'SUCCEEDED')
    updated_sandwich_workflow_group = ndb.Key('SandwichWorkflowGroup',
                                              'group1').get()
    self.assertEqual(updated_sandwich_workflow_group.active, False)

  @mock.patch(
      'dashboard.services.workflow_service.GetExecution',
      mock.MagicMock(
          return_value={'state': workflow_service.EXECUTION_STATE_FAILED}))
  @mock.patch('google.appengine.ext.deferred.defer', mock.MagicMock())
  @mock.patch(
      'dashboard.pinpoint.models.job_bug_update.DifferencesFoundBugUpdateBuilder.AddDifference'
  )
  def testCulpritVerificationAllExecutionCompletedNonZeroVerified(
      self, mock_bug_update_builder):
    sandwich_workflow_group.SandwichWorkflowGroup(
        key=ndb.Key('SandwichWorkflowGroup', 'group1'),
        metric='test_metric',
        url='test_url',
        active=True,
        cloud_workflows_keys=[1]).put()
    sandwich_workflow_group.CloudWorkflow(
        key=ndb.Key('CloudWorkflow', 1),
        kind='patch',
        commit_dict={
            'server': 'change_b.patch.server',
            'change': 'change_b.patch.change',
            'revision': 'change_b.patch.revision',
        },
        values_a=[1],
        values_b=[2],
        execution_name='test_execution_name').put()
    response = self.testapp.get('/cron/update-culprit-verification-results')
    self.assertEqual(response.status_code, 200)
    updated_cloud_workflow = ndb.Key('CloudWorkflow', 1).get()
    self.assertEqual(updated_cloud_workflow.execution_status, 'FAILED')
    updated_sandwich_workflow_group = ndb.Key('SandwichWorkflowGroup',
                                              'group1').get()
    self.assertEqual(updated_sandwich_workflow_group.active, False)
    mock_bug_update_builder.assert_called_with(
        None, [1.0], [2.0], 'patch', {
            'server': 'change_b.patch.server',
            'change': 'change_b.patch.change',
            'revision': 'change_b.patch.revision'
        })

  @mock.patch(
      'dashboard.pinpoint.models.sandwich_workflow_group.SandwichWorkflowGroup.GetAll',
      mock.MagicMock(return_value=[]))
  def testCulpritVerificationNoExecutionExisted(self):
    response = self.testapp.get('/cron/update-culprit-verification-results')
    self.assertEqual(response.status_code, 200)

  @mock.patch('dashboard.services.workflow_service.GetExecution',
              mock.MagicMock(
                  return_value={
                      'state': workflow_service.EXECUTION_STATE_SUCCEEDED,
                      'result': json.dumps({'decision': False})
                  }))
  def testCulpritVerificationMultipleSandwichWorkflowGroups(self):
    sandwich_workflow_group.SandwichWorkflowGroup(
        key=ndb.Key('SandwichWorkflowGroup', 'group1'),
        metric='test_metric1',
        url='test_url1',
        active=True,
        cloud_workflows_keys=[1]).put()
    sandwich_workflow_group.SandwichWorkflowGroup(
        key=ndb.Key('SandwichWorkflowGroup', 'group2'),
        metric='test_metric2',
        url='test_url2',
        active=True,
        cloud_workflows_keys=[2]).put()
    sandwich_workflow_group.CloudWorkflow(
        key=ndb.Key('CloudWorkflow', 1), execution_name='execution-id-0').put()
    sandwich_workflow_group.CloudWorkflow(
        key=ndb.Key('CloudWorkflow', 2), execution_name='execution-id-1').put()
    response = self.testapp.get('/cron/update-culprit-verification-results')
    self.assertEqual(response.status_code, 200)
    updated_cloud_workflow_1 = ndb.Key('CloudWorkflow', 1).get()
    self.assertEqual(updated_cloud_workflow_1.execution_status, 'SUCCEEDED')
    updated_cloud_workflow_2 = ndb.Key('CloudWorkflow', 2).get()
    self.assertEqual(updated_cloud_workflow_2.execution_status, 'SUCCEEDED')
    updated_sandwich_workflow_group_1 = ndb.Key('SandwichWorkflowGroup',
                                                'group1').get()
    self.assertEqual(updated_sandwich_workflow_group_1.active, False)
    updated_sandwich_workflow_group_2 = ndb.Key('SandwichWorkflowGroup',
                                                'group2').get()
    self.assertEqual(updated_sandwich_workflow_group_2.active, False)

  @mock.patch('dashboard.services.workflow_service.GetExecution',
              mock.MagicMock(
                  return_value={
                      'state': workflow_service.EXECUTION_STATE_ACTIVE,
                      'result': json.dumps({})
                  }))
  def testCulpritVerificationExecutiveActive(self):
    sandwich_workflow_group.SandwichWorkflowGroup(
        key=ndb.Key('SandwichWorkflowGroup', 'group1'),
        metric='test_metric',
        url='test_url',
        active=True,
        cloud_workflows_keys=[1]).put()
    sandwich_workflow_group.CloudWorkflow(
        key=ndb.Key('CloudWorkflow', 1), execution_name='execution-id-0').put()
    response = self.testapp.get('/cron/update-culprit-verification-results')
    self.assertEqual(response.status_code, 200)
    updated_sandwich_workflow_group = ndb.Key('SandwichWorkflowGroup',
                                              'group1').get()
    self.assertEqual(updated_sandwich_workflow_group.active, True)
