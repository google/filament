# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from unittest import mock

from dashboard.common import testing_common
from dashboard.api import api_auth
from dashboard.pinpoint import test
from dashboard.pinpoint.handlers import cancel
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import scheduler


@mock.patch('dashboard.common.utils.ServiceAccountHttp', mock.MagicMock())
@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
@mock.patch('dashboard.common.cloud_metric.PublishPinpointJobStatusMetric',
            mock.MagicMock())
@mock.patch('dashboard.common.cloud_metric.PublishPinpointJobRunTimeMetric',
            mock.MagicMock())
@mock.patch('dashboard.common.cloud_metric._PublishTSCloudMetric',
            mock.MagicMock())
class CancelJobTest(test.TestCase):

  def setUp(self):
    super().setUp()
    self.SetCurrentUserOAuth(testing_common.INTERNAL_USER)
    self.SetCurrentClientIdOAuth(api_auth.OAUTH_CLIENT_ID_ALLOWLIST[0])

    self.add_bug_comment = mock.MagicMock()
    self.get_issue = mock.MagicMock()

    perf_issue_patcher = mock.patch(
        'dashboard.services.perf_issue_service_client.GetIssue', self.get_issue)
    perf_issue_patcher.start()
    self.addCleanup(perf_issue_patcher.stop)

    perf_comment_post_patcher = mock.patch(
        'dashboard.services.perf_issue_service_client.PostIssueComment',
        self.add_bug_comment)
    perf_comment_post_patcher.start()
    self.addCleanup(perf_comment_post_patcher.stop)

  @mock.patch.object(cancel.utils, 'GetEmail',
                     mock.MagicMock(return_value='lovely.user@example.com'))
  @mock.patch.object(cancel.utils, 'IsAdministrator',
                     mock.MagicMock(return_value=False))
  @mock.patch.object(cancel.utils, 'IsTryjobUser',
                     mock.MagicMock(return_value=True))
  @mock.patch.object(cancel.utils, 'IsAllowedToDelegate',
                     mock.MagicMock(return_value=False))
  def testCancelKnownJobByOwner(self):
    job = job_module.Job.New((), (), user='lovely.user@example.com', bug_id=123)
    scheduler.Schedule(job)
    self.Post(
        '/api/job/cancel', {
            'job_id': job.job_id,
            'reason': 'testing!'
        },
        status=200)
    job = job_module.JobFromId(job.job_id)
    self.assertTrue(job.cancelled)
    self.assertIn('lovely.user@example.com: testing!', job.cancel_reason)
    self.ExecuteDeferredTasks('default')
    self.assertTrue(self.add_bug_comment.called)

  @mock.patch.object(cancel.utils, 'GetEmail',
                     mock.MagicMock(return_value='an.administrator@example.com')
                    )
  @mock.patch.object(cancel.utils, 'IsAdministrator',
                     mock.MagicMock(return_value=True))
  @mock.patch.object(cancel.utils, 'IsTryjobUser',
                     mock.MagicMock(return_value=True))
  @mock.patch.object(cancel.utils, 'IsAllowedToDelegate',
                     mock.MagicMock(return_value=False))
  def testCancelKnownJobByAdmin(self):
    job = job_module.Job.New((), (), user='lovely.user@example.com', bug_id=123)
    scheduler.Schedule(job)
    self.Post(
        '/api/job/cancel', {
            'job_id': job.job_id,
            'reason': 'testing!'
        },
        status=200)
    job = job_module.JobFromId(job.job_id)
    self.assertTrue(job.cancelled)
    self.assertIn('an.administrator@example.com: testing!', job.cancel_reason)
    self.ExecuteDeferredTasks('default')
    self.assertTrue(self.add_bug_comment.called)

  @mock.patch.object(cancel.utils, 'GetEmail',
                     mock.MagicMock(return_value='lovely.user@example.com'))
  @mock.patch.object(cancel.utils, 'IsAdministrator',
                     mock.MagicMock(return_value=False))
  @mock.patch.object(cancel.utils, 'IsTryjobUser',
                     mock.MagicMock(return_value=True))
  @mock.patch.object(cancel.utils, 'IsAllowedToDelegate',
                     mock.MagicMock(return_value=False))
  def testCancelUnknownJob(self):
    job = job_module.Job.New((), (), user='lovely.user@example.com')
    scheduler.Schedule(job)
    self.addCleanup(scheduler.Cancel, job)
    self.Post(
        '/api/job/cancel', {
            'job_id': job.job_id + '1',
            'reason': 'testing!'
        },
        status=404)
    job = job_module.JobFromId(job.job_id + '1')
    self.assertIsNone(job)

  @mock.patch.object(cancel.utils, 'GetEmail',
                     mock.MagicMock(return_value='lovely.user@example.com'))
  @mock.patch.object(cancel.utils, 'IsAdministrator',
                     mock.MagicMock(return_value=False))
  @mock.patch.object(cancel.utils, 'IsTryjobUser',
                     mock.MagicMock(return_value=True))
  def testCancelCancelledJob(self):
    job = job_module.Job.New((), (), user='lovely.user@example.com')
    scheduler.Schedule(job)
    self.Post(
        '/api/job/cancel', {
            'job_id': job.job_id,
            'reason': 'testing!'
        },
        status=200)
    job = job_module.JobFromId(job.job_id)
    self.assertTrue(job.cancelled)
    self.assertIn('lovely.user@example.com: testing!', job.cancel_reason)
    self.Post(
        '/api/job/cancel', {
            'job_id': job.job_id,
            'reason': 'cancelling again!'
        },
        status=400)
    job = job_module.JobFromId(job.job_id)
    self.assertTrue(job.cancelled)
    self.assertIn('lovely.user@example.com: testing!', job.cancel_reason)

  @mock.patch.object(cancel.utils, 'GetEmail',
                     mock.MagicMock(return_value='another.user@example.com'))
  @mock.patch.object(cancel.utils, 'IsAdministrator',
                     mock.MagicMock(return_value=False))
  @mock.patch.object(cancel.utils, 'IsTryjobUser',
                     mock.MagicMock(return_value=True))
  @mock.patch.object(cancel.utils, 'IsAllowedToDelegate',
                     mock.MagicMock(return_value=False))
  def testCancelForbiddenUser(self):
    job = job_module.Job.New((), (), user='lovely.user@example.com')
    scheduler.Schedule(job)
    self.addCleanup(scheduler.Cancel, job)
    self.Post(
        '/api/job/cancel', {
            'job_id': job.job_id,
            'reason': 'testing!'
        },
        status=403)

  @mock.patch.object(cancel.utils, 'GetEmail',
                     mock.MagicMock(return_value='another.user@example.com'))
  @mock.patch.object(cancel.utils, 'IsAdministrator',
                     mock.MagicMock(return_value=False))
  @mock.patch.object(cancel.utils, 'IsTryjobUser',
                     mock.MagicMock(return_value=True))
  @mock.patch.object(cancel.utils, 'IsAllowedToDelegate',
                     mock.MagicMock(return_value=True))
  def testCancelDelegationSupported(self):
    job = job_module.Job.New((), (), user='lovely.user@example.com')
    scheduler.Schedule(job)
    self.addCleanup(scheduler.Cancel, job)
    self.Post(
        '/api/job/cancel', {
            'user': job.user,
            'job_id': job.job_id,
            'reason': 'testing!'
        },
        status=200)

  @mock.patch.object(
      cancel.utils, 'GetEmail',
      mock.MagicMock(return_value='some-service-account@example.com'))
  @mock.patch.object(cancel.utils, 'IsAdministrator',
                     mock.MagicMock(return_value=False))
  @mock.patch.object(cancel.utils, 'IsTryjobUser',
                     mock.MagicMock(return_value=True))
  @mock.patch.object(cancel.utils, 'IsAllowedToDelegate',
                     mock.MagicMock(return_value=False))
  def testCancelDelegationRejected(self):
    job = job_module.Job.New((), (), user='lovely.user@example.com')
    scheduler.Schedule(job)
    self.addCleanup(scheduler.Cancel, job)
    self.Post(
        '/api/job/cancel', {
            'user': job.user,
            'job_id': job.job_id,
            'reason': 'testing!'
        },
        status=403)

  @mock.patch.object(
      cancel.utils, 'GetEmail',
      mock.MagicMock(return_value='some-service-account@example.com'))
  @mock.patch.object(cancel.utils, 'IsAdministrator',
                     mock.MagicMock(return_value=True))
  @mock.patch.object(cancel.utils, 'IsTryjobUser',
                     mock.MagicMock(return_value=True))
  @mock.patch.object(cancel.utils, 'IsAllowedToDelegate',
                     mock.MagicMock(return_value=True))
  def testCancelDelegationForAnAdminWorks(self):
    job = job_module.Job.New((), (), user='lovely.user@example.com')
    scheduler.Schedule(job)
    self.addCleanup(scheduler.Cancel, job)
    self.Post(
        '/api/job/cancel', {
            'user': 'admin@example.com',
            'job_id': job.job_id,
            'reason': 'testing!'
        },
        status=200)

  @mock.patch.object(cancel.utils, 'GetEmail',
                     mock.MagicMock(return_value='lovely.user@example.com'))
  @mock.patch.object(cancel.utils, 'IsAdministrator',
                     mock.MagicMock(return_value=False))
  @mock.patch.object(cancel.utils, 'IsTryjobUser',
                     mock.MagicMock(return_value=True))
  def testCancelAlreadyRunningJob(self):
    job = job_module.Job.New((), (),
                             arguments={'configuration': 'mock'},
                             user='lovely.user@example.com')
    scheduler.Schedule(job)
    _, status = scheduler.PickJobs(job.configuration)[0]
    self.assertEqual(status, 'Queued')
    job.task = '123'
    job.started = True
    job.put()
    self.assertTrue(job.running)
    self.addCleanup(scheduler.Cancel, job)
    self.Post(
        '/api/job/cancel', {
            'job_id': job.job_id,
            'reason': 'testing!'
        },
        status=200)
