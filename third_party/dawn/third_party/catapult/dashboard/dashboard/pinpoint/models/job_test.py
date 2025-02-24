# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
from flask import Flask
from unittest import mock
import sys

from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import reserved_infos

from dashboard import sheriff_config_client
from dashboard.common import testing_common
from dashboard.common import layered_cache
from dashboard.common import utils
from dashboard.models import histogram
from dashboard.models import anomaly
from dashboard.models import graph_data
from dashboard.models import subscription
from dashboard.pinpoint.models import change
from dashboard.pinpoint.models import errors
from dashboard.pinpoint.models import job
from dashboard.pinpoint.models import scheduler
from dashboard.pinpoint import test
from six.moves import zip # pylint: disable=redefined-builtin

# This is a very long file.
# pylint: disable=too-many-lines

_CHROMIUM_URL = 'https://chromium.googlesource.com/chromium/src'
_COMMENT_STARTED = (u"""\U0001f4cd Pinpoint job started.
https://testbed.example.com/job/1""")

_COMMENT_COMPLETED_NO_COMPARISON = (
    u"""<b>\U0001f4cd Job complete. See results below.</b>
https://testbed.example.com/job/1""")

_COMMENT_COMPLETED_NO_DIFFERENCES = (
    u"""<b>\U0001f4cd Couldn't reproduce a difference.</b>
https://testbed.example.com/job/1""")

_COMMENT_COMPLETED_NO_DIFFERENCES_DUE_TO_FAILURE = (
    u"""<b>\U0001f63f Job finished with errors.</b>
https://testbed.example.com/job/1

One or both of the initial changes failed to produce any results.
Perhaps the job is misconfigured or the tests are broken? See the job
page for details.""")

_COMMENT_FAILED = (u"""\U0001f63f Pinpoint job stopped with an error.
https://testbed.example.com/job/1

Error string""")

_COMMENT_CODE_REVIEW = (u"""\U0001f4cd Job complete.

See results at: https://testbed.example.com/job/1""")


def FakeCommitAsDict(commit_self):
  """Fake for Commit.AsDict.

  Returns a canned commit dict based on the Commit's git_hash, which must start
  with the prefix "git_hash_".

  Use like::

    @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict',
                autospec=True)
    def testFoo(self, commit_as_dict):
      commit_as_dict.side_effect = FakeCommitAsDict
      ...
  """
  git_hash = commit_self.git_hash
  n = git_hash[len('git_hash_'):]
  return {
      'repository': 'chromium',
      'git_hash': git_hash,
      'url': 'https://example.com/repository/+/' + git_hash,
      'author': 'author%s@chromium.org' % (n,),
      'subject': 'Subject.',
      'message': 'Subject.\n\nCommit message.',
      }


@mock.patch.object(job.results2, 'GetCachedResults2',
                   mock.MagicMock(return_value='http://foo'))
@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
class JobTest(test.TestCase):

  @mock.patch.object(
      job.timing_record, 'GetSimilarHistoricalTimings',
      mock.MagicMock(
          return_value=job.timing_record.EstimateResult(
              job.timing_record.Timings(
                  datetime.timedelta(seconds=10), datetime.timedelta(
                      seconds=5), datetime.timedelta(
                          seconds=100)), ['try', 'linux'])))
  @mock.patch.object(job.scheduler, 'QueueStats',
                     mock.MagicMock(return_value=[]))
  def testAsDictOptions_Estimate(self):
    j = job.Job.New((), (), bug_id=123456)

    d = j.AsDict([job.OPTION_ESTIMATE])
    self.assertTrue('estimate' in d)
    self.assertEqual(d['estimate']['timings'][0], 10)
    self.assertEqual(d['estimate']['timings'][1], 5)
    self.assertEqual(d['estimate']['timings'][2], 100)
    self.assertEqual(d['estimate']['tags'], ['try', 'linux'])

  def testAsDictOptions_Inputs(self):
    j = job.Job.New((), (), bug_id=123456)
    d = j.AsDict([job.OPTION_INPUTS])
    self.assertEqual(d['state'], [])

  @mock.patch.object(job.timing_record, 'GetSimilarHistoricalTimings',
                     mock.MagicMock(return_value=None))
  @mock.patch.object(job.scheduler, 'QueueStats',
                     mock.MagicMock(return_value=[]))
  def testAsDictOptions_EstimateFails(self):
    j = job.Job.New((), (), bug_id=123456)

    d = j.AsDict([job.OPTION_ESTIMATE])
    self.assertFalse('estimate' in d)

  def testImprovementDirectionToStr(self):
    j = job.Job.New((), (), bug_id=123456)
    self.assertEqual(j._ImprovementDirectionToStr(anomaly.UP), 'UP')
    self.assertEqual(j._ImprovementDirectionToStr(anomaly.DOWN), 'DOWN')
    self.assertEqual(j._ImprovementDirectionToStr(anomaly.UNKNOWN), 'UNKNOWN')

  def testGetGitHash(self):
    j = job.Job.New((), (), bug_id=123456)
    c = change.Change((change.Commit('chromium', 'test_git_hash'),))
    self.assertEqual(j._GetGitHash(c), 'test_git_hash')

  def testCreateWorkflowExecutionRequest(self):
    j = job.Job.New((), (),
                    arguments={
                        'configuration': 'bot1',
                        'benchmark': 'webrtc_perf_tests'
                    },
                    bug_id=123456)
    c1 = change.Change((change.Commit('chromium', 'test_git_hash1'),))
    c2 = change.Change((change.Commit('chromium', 'test_git_hash2'),))
    improvement_dir = 'UP'
    request = j._CreateWorkflowExecutionRequest(c1, c2, improvement_dir)
    self.assertEqual(request['start_git_hash'], 'test_git_hash1')
    self.assertEqual(request['end_git_hash'], 'test_git_hash2')
    self.assertEqual(request['target'], 'webrtc_perf_tests')
    self.assertEqual(request['improvement_dir'], improvement_dir)

  @mock.patch('dashboard.services.perf_issue_service_client.GetIssue')
  def testCanSandwich(self, get_issue):

    def _GetIssue(bug_id, project_name='chromium'):
      if bug_id == 123456:
        return {
            'projectId':
                project_name,
            'id':
                123456,
            'title':
                '[Sandwich Verification Test Speedometer2]: 2 regressions in speedometer2',
        }
      if bug_id == 123457:
        return {
            'projectId':
                project_name,
            'id':
                123457,
            'title':
                '[[b/12345 fix a bug] Sandwich Verification Test Speedometer2]: 2 regressions',
        }
      if bug_id == 123458:
        return {
            'projectId':
                project_name,
            'id':
                123458,
            'title':
                '[b/12345 fix a bug] [Sandwich Verification Test Speedometer2]: 2 regressions',
        }
      if bug_id == 123459:
        return {
            'projectId':
                project_name,
            'id':
                123459,
            'title':
                '[Sandwich Verification Test Speedometer2][b/12345 fix a bug]: 2 regressions',
        }
      if bug_id == 123460:
        return {
            'projectId':
                project_name,
            'id':
                123460,
            'title':
                '[Sandwich Verification Test Speedometer2 [b/12345 fix a bug]]: 2 regressions',
        }

      return {'projectId': project_name, 'id': bug_id, 'title': 'fake_title'}

    get_issue.side_effect = _GetIssue

    j1 = job.Job.New((), (),
                     arguments={
                         'configuration': 'bot1',
                         'benchmark': 'webrtc_perf_tests'
                     },
                     bug_id=12345)
    j2 = job.Job.New((), (),
                     arguments={
                         'configuration': 'linux-perf',
                         'benchmark': 'speedometer2'
                     },
                     bug_id=123456,
                     user='chromeperf@appspot.gserviceaccount.com')
    j3 = job.Job.New((), (),
                     arguments={
                         'configuration': 'linux-perf',
                         'benchmark': 'speedometer2'
                     },
                     bug_id=123457,
                     user='chromeperf@appspot.gserviceaccount.com')
    j4 = job.Job.New((), (),
                     arguments={
                         'configuration': 'linux-perf',
                         'benchmark': 'speedometer2'
                     },
                     bug_id=123458,
                     user='chromeperf@appspot.gserviceaccount.com')
    j5 = job.Job.New((), (),
                     arguments={
                         'configuration': 'linux-perf',
                         'benchmark': 'speedometer2'
                     },
                     bug_id=123459,
                     user='chromeperf@appspot.gserviceaccount.com')
    j6 = job.Job.New((), (),
                     arguments={
                         'configuration': 'linux-perf',
                         'benchmark': 'speedometer2'
                     },
                     bug_id=123460,
                     user='chromeperf@appspot.gserviceaccount.com')
    j7 = job.Job.New((), (),
                     arguments={
                         'configuration': 'linux-perf',
                         'benchmark': 'speedometer2'
                     },
                     bug_id=123456)
    self.assertFalse(j1._CanSandwich())
    self.assertTrue(j2._CanSandwich())
    self.assertTrue(j3._CanSandwich())
    self.assertTrue(j4._CanSandwich())
    self.assertTrue(j5._CanSandwich())
    self.assertTrue(j6._CanSandwich())
    self.assertFalse(j7._CanSandwich())

  @mock.patch('dashboard.services.workflow_service.CreateExecution',
              mock.MagicMock(return_value='test-workflow-execution-name'))
  def testStartSandwichAndUpdateWorkflowGroup(self):
    j = job.Job.New((), (),
                    arguments={
                        'configuration': 'bot1',
                        'benchmark': 'webrtc_perf_tests'
                    },
                    bug_id=123456)
    c0 = change.Change((change.Commit('chromium', 'git_hash_0'),))
    c1 = change.Change((change.Commit('chromium', 'git_hash_1'),))
    change_map = {c0: [0], c1: [10]}
    differences = [(c0, c1)]
    got_regression_cnt, got_wf_executions = j._StartSandwichAndUpdateWorkflowGroup(
        anomaly.DOWN, differences, change_map)
    self.assertEqual(got_regression_cnt, 1)
    self.assertEqual(got_wf_executions, ['test-workflow-execution-name'])

  @mock.patch('dashboard.services.workflow_service.CreateExecution',
              mock.MagicMock(return_value='test-workflow-execution-name'))
  def testStartSandwichAndUpdateWorkflowGroup_multipleCulprits(self):
    j = job.Job.New((), (),
                    arguments={
                        'configuration': 'bot1',
                        'benchmark': 'webrtc_perf_tests'
                    },
                    bug_id=123456)
    c0 = change.Change((change.Commit('chromium', 'git_hash_0'),))
    c1 = change.Change((change.Commit('chromium', 'git_hash_1'),))
    c2 = change.Change((change.Commit('chromium', 'git_hash_2'),))
    c3 = change.Change((change.Commit('chromium', 'git_hash_3'),))
    change_map = {c0: [0], c1: [10], c2: [11], c3: [20]}
    differences = [(c0, c1), (c2, c3)]
    got_regression_cnt, got_wf_executions = j._StartSandwichAndUpdateWorkflowGroup(
        anomaly.DOWN, differences, change_map)
    self.assertEqual(got_regression_cnt, 2)
    self.assertEqual(got_wf_executions, ['test-workflow-execution-name',
      'test-workflow-execution-name'])

@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=[]))
class JobTestNoBots(test.TestCase):

  def testNoBots(self):
    with self.assertRaises(errors.SwarmingNoBots):
      job.Job.New((), (), bug_id=123456)


@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=['a', 'b', 'c', 'd', 'e']))
class JobTestOddBots(test.TestCase):

  def testOddBots(self):
    j = job.Job.New((), (), bug_id=123456)
    self.assertEqual(len(j.bots), 4)


@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
@mock.patch('dashboard.common.cloud_metric._PublishTSCloudMetric',
            mock.MagicMock())
class RetryTest(test.TestCase):

  def testStarted_RecoverableError_BacksOff(self):
    j = job.Job.New((), (), comparison_mode='performance')
    j.Start()
    scheduler.Schedule(j)
    j.state.Explore = mock.MagicMock(side_effect=errors.RecoverableError(None))
    j._Schedule = mock.MagicMock()
    j.put = mock.MagicMock()
    j.Fail = mock.MagicMock()
    j.Run()
    j.Run()
    j.Run()
    self.assertEqual(j._Schedule.call_args_list[0],
                     mock.call(countdown=job._TASK_INTERVAL * 2))
    self.assertEqual(j._Schedule.call_args_list[1],
                     mock.call(countdown=job._TASK_INTERVAL * 4))
    self.assertEqual(j._Schedule.call_args_list[2],
                     mock.call(countdown=job._TASK_INTERVAL * 8))
    self.assertFalse(j.Fail.called)
    j.Run()
    self.assertTrue(j.Fail.called)

  def testStarted_RecoverableError_Resets(self):
    j = job.Job.New((), (), comparison_mode='performance')
    j.Start()
    scheduler.Schedule(j)
    j.state.Explore = mock.MagicMock(side_effect=errors.RecoverableError(None))
    j._Schedule = mock.MagicMock()
    j.put = mock.MagicMock()
    j.Fail = mock.MagicMock()
    j.Run()
    j.Run()
    j.Run()
    self.assertEqual(j._Schedule.call_args_list[0],
                     mock.call(countdown=job._TASK_INTERVAL * 2))
    self.assertEqual(j._Schedule.call_args_list[1],
                     mock.call(countdown=job._TASK_INTERVAL * 4))
    self.assertEqual(j._Schedule.call_args_list[2],
                     mock.call(countdown=job._TASK_INTERVAL * 8))
    self.assertFalse(j.Fail.called)
    j.state.Explore = mock.MagicMock()
    j.Run()
    self.assertEqual(0, j.retry_count)


@mock.patch('dashboard.pinpoint.models.job_state.JobState.ChangesExamined',
            lambda _: 10)
@mock.patch('dashboard.common.utils.ServiceAccountHttp', mock.MagicMock())
@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
@mock.patch('dashboard.common.cloud_metric._PublishTSCloudMetric',
            mock.MagicMock())
@mock.patch('dashboard.services.perf_issue_service_client.GetAlertGroupQuality',
            mock.MagicMock(return_value={'result': 'Good'}))
@mock.patch.object(sheriff_config_client.SheriffConfigClient, '__init__',
                     mock.MagicMock(return_value=None))
class BugCommentTest(test.TestCase):

  def setUp(self):
    super().setUp()
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

    self.PatchDatastoreHooksRequest()

  def testNoBug(self):
    j = job.Job.New((), ())
    j.Start()
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(self.add_bug_comment.called)

  def testStarted(self):
    j = job.Job.New((), (), bug_id=123456)
    j.Start()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        123456,
        'chromium',
        comment=_COMMENT_STARTED,
        labels=mock.ANY,
        send_email=True)
    labels = self.add_bug_comment.call_args[1]['labels']
    self.assertIn('Pinpoint-Job-Started', labels)
    self.assertNotIn('-Pinpoint-Job-Started', labels)

  def testCompletedNoComparison(self):
    j = job.Job.New((), (), bug_id=123456)
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        123456,
        'chromium',
        comment=_COMMENT_COMPLETED_NO_COMPARISON,
        labels=['Pinpoint-Tryjob-Completed'],
    )

  def testCompletedNoDifference(self):
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        123456,
        'chromium',
        comment=_COMMENT_COMPLETED_NO_DIFFERENCES,
        labels=mock.ANY,
        status='WontFix',
    )
    labels = self.add_bug_comment.call_args[1]['labels']
    self.assertIn('Pinpoint-Job-Completed', labels)
    self.assertNotIn('-Pinpoint-Job-Completed', labels)
    self.assertIn('Pinpoint-No-Repro', labels)
    self.assertNotIn('-Pinpoint-No-Repro', labels)

  @mock.patch.object(job.job_state.JobState, 'FirstOrLastChangeFailed')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedNoDifferenceDueToFailureAtOneChange(
      self, differences, first_or_last_change_failed):
    differences.return_value = []
    first_or_last_change_failed.return_value = True
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        123456,
        'chromium',
        comment=_COMMENT_COMPLETED_NO_DIFFERENCES_DUE_TO_FAILURE,
        labels=mock.ANY)
    labels = self.add_bug_comment.call_args[1]['labels']
    self.assertIn('Pinpoint-Job-Failed', labels)
    self.assertNotIn('-Pinpoint-Job-Failed', labels)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedWithCommit(self, differences, result_values, commit_as_dict):
    c = change.Change((change.Commit('chromium', 'git_hash'),))
    differences.return_value = [(None, c)]
    result_values.side_effect = [0], [1.23456]
    commit_as_dict.return_value = {
        'repository': 'chromium',
        'git_hash': 'git_hash',
        'url': 'https://example.com/repository/+/git_hash',
        'author': 'author@chromium.org',
        'subject': 'Subject.',
        'message': 'Subject.\n\nCommit message.',
    }
    self.get_issue.return_value = {'status': 'Untriaged'}
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        status='Assigned',
        owner='author@chromium.org',
        labels=mock.ANY,
        cc=['author@chromium.org'],
        components=[],
        merge_issue=None)
    message = self.add_bug_comment.call_args.kwargs['comment']
    self.assertIn('Found a significant difference at 1 commit.', message)
    self.assertIn('<b>Subject.</b>', message)
    self.assertIn('https://example.com/repository/+/git_hash', message)
    labels = self.add_bug_comment.call_args.kwargs['labels']
    self.assertIn('Pinpoint-Culprit-Found', labels)
    self.assertNotIn('-Pinpoint-Culprit-Found', labels)
    self.assertIn('Pinpoint-Job-Completed', labels)
    self.assertNotIn('-Pinpoint-Job-Completed', labels)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  @mock.patch.object(sheriff_config_client.SheriffConfigClient, 'Match')
  @mock.patch.object(histogram.SparseDiagnostic, 'GetMostRecentDataByNamesSync')
  def testCompletedWithCommitAndReport(self, recent_data, match, differences,
                                       result_values, commit_as_dict):
    c = change.Change((change.Commit('chromium', 'git_hash'),))
    differences.return_value = [(None, c)]
    result_values.side_effect = [0], [1.23456]
    commit_as_dict.return_value = {
        'repository': 'chromium',
        'git_hash': 'git_hash',
        'url': 'https://example.com/repository/+/git_hash',
        'author': 'author@chromium.org',
        'subject': 'Subject.',
        'message': 'Subject.\n\nCommit message.',
    }
    match.return_value = ([
        subscription.Subscription(
            name='sheriff subscription',
            bug_components=['test-component'],
            bug_cc_emails=['test@cc.email'],
            bug_labels=['test-label'])
    ], None)
    recent_data.return_value = ''
    self.get_issue.return_value = {
        'status': 'Untriaged',
        'labels': ['Chromeperf-Delay-Reporting']
    }
    j = job.Job.New((), (),
                    bug_id=123456,
                    comparison_mode='performance',
                    tags={
                        'auto_bisection': 'true',
                        'test_path': 'dummy/path'
                    })
    scheduler.Schedule(j)
    app = Flask(__name__)
    with app.test_request_context('dummy/path', 'GET'):
      j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        status='Assigned',
        owner='author@chromium.org',
        labels=mock.ANY,
        cc=['author@chromium.org', 'test@cc.email'],
        components=['test-component', '-Speed>Regressions'],
        merge_issue=None)
    message = self.add_bug_comment.call_args.kwargs['comment']
    self.assertIn('Found a significant difference at 1 commit.', message)
    labels = self.add_bug_comment.call_args.kwargs['labels']
    self.assertIn('test-label', labels)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedWithCommitDoNotNotify(self, differences, result_values,
                                         commit_as_dict):
    c = change.Change((change.Commit('chromium', 'git_hash'),))
    differences.return_value = [(None, c)]
    result_values.side_effect = [0], [1.23456]
    commit_as_dict.return_value = {
        'repository': 'chromium',
        'git_hash': 'git_hash',
        'url': 'https://example.com/repository/+/git_hash',
        'author': 'author@chromium.org',
        'subject': 'Subject.',
        'message': 'Subject.\n\nCommit message.',
    }
    self.get_issue.return_value = {
        'status': 'Untriaged',
        'labels': ['DoNotNotify']
    }
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        status='Available',  # Status should be available since no owner
        owner='',  # No owner expected
        labels=mock.ANY,
        cc=[],  # No cc list expected
        components=[],
        merge_issue=None)
    message = self.add_bug_comment.call_args.kwargs['comment']
    self.assertIn('Found a significant difference at 1 commit.', message)
    self.assertIn('<b>Subject.</b>', message)
    self.assertIn('https://example.com/repository/+/git_hash', message)
    labels = self.add_bug_comment.call_args.kwargs['labels']
    self.assertIn('Pinpoint-Culprit-Found', labels)
    self.assertNotIn('-Pinpoint-Culprit-Found', labels)
    self.assertIn('Pinpoint-Job-Completed', labels)
    self.assertNotIn('-Pinpoint-Job-Completed', labels)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedMergeIntoExisting(self, differences, result_values,
                                     commit_as_dict):
    c = change.Change((change.Commit('chromium', 'git_hash'),))
    differences.return_value = [(None, c)]
    result_values.side_effect = [0], [1.23456]
    commit_as_dict.return_value = {
        'repository': 'chromium',
        'git_hash': 'git_hash',
        'author': 'author@chromium.org',
        'subject': 'Subject.',
        'url': 'https://example.com/repository/+/git_hash',
        'message': 'Subject.\n\nCommit message.',
    }
    self.get_issue.return_value = {
        'status': 'Untriaged',
        'id': '111222',
        'projectId': 'chromium'
    }
    layered_cache.SetExternal('commit_hash_git_hash', 'chromium:111222')
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        status='Assigned',
        owner='author@chromium.org',
        cc=[],
        components=[],
        labels=mock.ANY,
        merge_issue='111222')
    message = self.add_bug_comment.call_args.kwargs['comment']
    self.assertIn('Found a significant difference at 1 commit.', message)
    self.assertIn('https://example.com/repository/+/git_hash', message)
    labels = self.add_bug_comment.call_args.kwargs['labels']
    self.assertIn('Pinpoint-Job-Completed', labels)
    self.assertNotIn('-Pinpoint-Job-Completed', labels)
    self.assertIn('Pinpoint-Culprit-Found', labels)
    self.assertNotIn('-Pinpoint-Culprit-Found', labels)

  @mock.patch('dashboard.services.perf_issue_service_client.GetIssue')
  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedSkipsMergeWhenDuplicate(self, differences, result_values,
                                           commit_as_dict, get_issue):
    c = change.Change((change.Commit('chromium', 'git_hash'),))
    differences.return_value = [(None, c)]
    result_values.side_effect = [0], [1.23456]
    commit_as_dict.return_value = {
        'repository': 'chromium',
        'git_hash': 'git_hash',
        'author': 'author@chromium.org',
        'subject': 'Subject.',
        'url': 'https://example.com/repository/+/git_hash',
        'message': 'Subject.\n\nCommit message.',
    }

    def _GetIssue(bug_id, project_name='chromium'):
      if bug_id == '111222':
        return {
            'status': 'Duplicate',
            'projectId': project_name,
            'id': '111222'
        }
      return {
          'status': 'Untriaged',
          'projectId': project_name,
          'id': str(bug_id)
      }

    get_issue.side_effect = _GetIssue
    layered_cache.SetExternal('commit_hash_git_hash', 'chromium:111222')
    j = job.Job.New((), (),
                    bug_id=123456,
                    comparison_mode='performance',
                    project='chromium')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        status='Assigned',
        owner='author@chromium.org',
        labels=mock.ANY,
        cc=['author@chromium.org'],
        components=[],
        merge_issue=None)
    message = self.add_bug_comment.call_args.kwargs['comment']
    self.assertIn('Found a significant difference at 1 commit.', message)
    self.assertIn('https://example.com/repository/+/git_hash', message)
    labels = self.add_bug_comment.call_args.kwargs['labels']
    self.assertIn('Pinpoint-Job-Completed', labels)
    self.assertNotIn('-Pinpoint-Job-Completed', labels)
    self.assertIn('Pinpoint-Culprit-Found', labels)
    self.assertNotIn('-Pinpoint-Culprit-Found', labels)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedWithInvalidIssue(self, differences, result_values,
                                    commit_as_dict):
    c = change.Change((change.Commit('chromium', 'git_hash'),))
    differences.return_value = [(None, c)]
    result_values.side_effect = [0], [1.23456]
    commit_as_dict.return_value = {
        'repository': 'chromium',
        'git_hash': 'git_hash',
        'url': 'https://example.com/repository/+/git_hash',
        'author': 'author@chromium.org',
        'subject': 'Subject.',
        'message': 'Subject.\n\nCommit message.',
    }
    self.get_issue.return_value = None
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.assertFalse(self.add_bug_comment.called)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedWithCommitAndDocs(self, differences, result_values,
                                     commit_as_dict):
    c = change.Change((change.Commit('chromium', 'git_hash'),))
    differences.return_value = [(None, c)]
    result_values.side_effect = [1.23456], [0]
    commit_as_dict.return_value = {
        'repository': 'chromium',
        'git_hash': 'git_hash',
        'url': 'https://example.com/repository/+/git_hash',
        'author': 'author@chromium.org',
        'subject': 'Subject.',
        'message': 'Subject.\n\nCommit message.',
    }
    self.get_issue.return_value = {'status': 'Untriaged'}
    j = job.Job.New((), (),
                    bug_id=123456,
                    comparison_mode='performance',
                    tags={'test_path': 'master/bot/benchmark'})
    diag_dict = generic_set.GenericSet([[u'Benchmark doc link',
                                         u'http://docs']])
    diag = histogram.SparseDiagnostic(
        data=diag_dict.AsDict(),
        start_revision=1,
        end_revision=sys.maxsize,
        name=reserved_infos.DOCUMENTATION_URLS.name,
        test=utils.TestKey('master/bot/benchmark'))
    diag.put()
    scheduler.Schedule(j)
    app = Flask(__name__)
    with app.test_request_context('dummy/path', 'GET'):
      j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        status='Assigned',
        owner='author@chromium.org',
        labels=mock.ANY,
        cc=['author@chromium.org'],
        components=[],
        merge_issue=None)
    message = self.add_bug_comment.call_args.kwargs['comment']
    self.assertIn('Found a significant difference at 1 commit.', message)
    self.assertIn('http://docs', message)
    self.assertIn('Benchmark doc link', message)
    labels = self.add_bug_comment.call_args.kwargs['labels']
    self.assertIn('Pinpoint-Job-Completed', labels)
    self.assertNotIn('-Pinpoint-Job-Completed', labels)
    self.assertIn('Pinpoint-Culprit-Found', labels)
    self.assertNotIn('-Pinpoint-Culprit-Found', labels)

  @mock.patch('dashboard.pinpoint.models.change.patch.GerritPatch.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedWithPatch(self, differences, result_values, patch_as_dict):
    commits = (change.Commit('chromium', 'git_hash'),)
    patch = change.GerritPatch('https://codereview.com', 672011, '2f0d5c7')
    c = change.Change(commits, patch)
    differences.return_value = [(None, c)]
    result_values.side_effect = [40], [20]
    patch_as_dict.return_value = {
        'url': 'https://codereview.com/c/672011/2f0d5c7',
        'author': 'author@chromium.org',
        'subject': 'Subject.',
        'message': 'Subject.\n\nCommit message.',
    }
    self.get_issue.return_value = {'status': 'Untriaged'}
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        status='Assigned',
        owner='author@chromium.org',
        labels=mock.ANY,
        cc=['author@chromium.org'],
        components=[],
        merge_issue=None)
    message = self.add_bug_comment.call_args.kwargs['comment']
    self.assertIn('Found a significant difference at 1 commit.', message)
    self.assertIn('https://codereview.com/c/672011/2f0d5c7', message)
    labels = self.add_bug_comment.call_args.kwargs['labels']
    self.assertIn('Pinpoint-Culprit-Found', labels)
    self.assertNotIn('-Pinpoint-Culprit-Found', labels)

  @mock.patch('dashboard.pinpoint.models.change.patch.GerritPatch.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedDoesReassign(self, differences, result_values,
                                patch_as_dict):
    commits = (change.Commit('chromium', 'git_hash'),)
    patch = change.GerritPatch('https://codereview.com', 672011, '2f0d5c7')
    c = change.Change(commits, patch)
    c = change.Change(commits, patch)
    differences.return_value = [(None, c)]
    result_values.side_effect = [40], [20]
    patch_as_dict.return_value = {
        'url': 'https://codereview.com/c/672011/2f0d5c7',
        'author': 'author@chromium.org',
        'subject': 'Subject.',
        'message': 'Subject.\n\nCommit message.',
    }
    self.get_issue.return_value = {
        'status': 'Assigned',
        'owner': {
            'email': 'some-author@somewhere.org'
        }
    }
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        owner='author@chromium.org',
        status=None,
        cc=['author@chromium.org', 'some-author@somewhere.org'],
        components=[],
        labels=mock.ANY,
        merge_issue=None)
    message = self.add_bug_comment.call_args.kwargs['comment']
    self.assertIn('Found a significant difference at 1 commit.', message)
    self.assertIn('https://codereview.com/c/672011/2f0d5c7', message)
    labels = self.add_bug_comment.call_args.kwargs['labels']
    self.assertIn('Pinpoint-Job-Completed', labels)
    self.assertNotIn('-Pinpoint-Job-Completed', labels)
    self.assertIn('Pinpoint-Culprit-Found', labels)
    self.assertNotIn('-Pinpoint-Culprit-Found', labels)

  @mock.patch('dashboard.pinpoint.models.change.patch.GerritPatch.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedDoesNotReopen(self, differences, result_values,
                                 patch_as_dict):
    commits = (change.Commit('chromium', 'git_hash'),)
    patch = change.GerritPatch('https://codereview.com', 672011, '2f0d5c7')
    c = change.Change(commits, patch)
    differences.return_value = [(None, c)]
    result_values.side_effect = [40], [20]
    patch_as_dict.return_value = {
        'url': 'https://codereview.com/c/672011/2f0d5c7',
        'author': 'author@chromium.org',
        'subject': 'Subject.',
        'message': 'Subject.\n\nCommit message.',
    }
    self.get_issue.return_value = {'status': 'Fixed'}
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        owner=None,
        status=None,
        cc=['author@chromium.org'],
        components=[],
        labels=mock.ANY,
        merge_issue=None)
    message = self.add_bug_comment.call_args.kwargs['comment']
    self.assertIn('10 revisions compared', message)
    labels = self.add_bug_comment.call_args.kwargs['labels']
    self.assertIn('Pinpoint-Job-Completed', labels)
    self.assertNotIn('-Pinpoint-Job-Completed', labels)
    self.assertIn('Pinpoint-Culprit-Found', labels)
    self.assertNotIn('-Pinpoint-Culprit-Found', labels)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedMultipleDifferences(self, differences, result_values,
                                       commit_as_dict):
    c0 = change.Change((change.Commit('chromium', 'git_hash_0'),))
    c1 = change.Change((change.Commit('chromium', 'git_hash_1'),))
    c2 = change.Change((change.Commit('chromium', 'git_hash_2'),))
    c2_5 = change.Change((change.Commit('chromium', 'git_hash_2_5')))
    c3 = change.Change((change.Commit('chromium', 'git_hash_3'),))
    change_map = {c0: [50], c1: [0], c2: [40], c2_5: [0], c3: []}
    differences.return_value = [(c0, c1), (c1, c2), (c2_5, c3)]
    result_values.side_effect = lambda c: change_map.get(c, [])
    commit_as_dict.side_effect = (
        {
            'repository': 'chromium',
            'git_hash': 'git_hash_1',
            'url': 'https://example.com/repository/+/git_hash_1',
            'author': 'author1@chromium.org',
            'subject': 'Subject.',
            'message': 'Subject.\n\nCommit message.',
        },
        {
            'repository': 'chromium',
            'git_hash': 'git_hash_2',
            'url': 'https://example.com/repository/+/git_hash_2',
            'author': 'author2@chromium.org',
            'subject': 'Subject.',
            'message': 'Subject.\n\nCommit message.',
        },
        {
            'repository': 'chromium',
            'git_hash': 'git_hash_3',
            'url': 'https://example.com/repository/+/git_hash_3',
            'author': 'author3@chromium.org',
            'subject': 'Subject.',
            'message': 'Subject.\n\nCommit message.',
        },
    )
    self.get_issue.return_value = {'status': 'Untriaged'}
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)

    # We now only CC folks from the top commit.
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        status='Assigned',
        owner='author1@chromium.org',
        cc=['author1@chromium.org'],
        components=[],
        labels=mock.ANY,
        merge_issue=None,
    )
    message = self.add_bug_comment.call_args.kwargs['comment']
    self.assertIn('Found significant differences at 2 commits.', message)
    self.assertIn('1. Subject.', message)
    self.assertIn('transitions from "no values" to "some values"', message)
    labels = self.add_bug_comment.call_args[1]['labels']
    self.assertIn('Pinpoint-Job-Completed', labels)
    self.assertNotIn('-Pinpoint-Job-Completed', labels)
    self.assertIn('Pinpoint-Multiple-Culprits', labels)
    self.assertNotIn('-Pinpoint-Multiple-Culprits', labels)
    self.assertIn('Pinpoint-Multiple-MissingValues', labels)
    self.assertNotIn('-Pinpoint-Multiple-MissingValues', labels)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict',
              autospec=True)
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedMultipleDifferences_BlameAbsoluteLargest(
      self, differences, result_values, commit_as_dict):
    c1 = change.Change((change.Commit('chromium', 'git_hash_1'),))
    c2 = change.Change((change.Commit('chromium', 'git_hash_2'),))
    c3 = change.Change((change.Commit('chromium', 'git_hash_3'),))
    change_map = {c1: [10], c2: [0], c3: [-100]}
    differences.return_value = [(None, c1), (c1, c2), (c2, c3)]
    result_values.side_effect = lambda c: change_map.get(c, [])
    commit_as_dict.side_effect = FakeCommitAsDict
    self.get_issue.return_value = {'status': 'Untriaged'}
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)

    # We now only CC folks from the top commit.
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        status='Assigned',
        owner='author3@chromium.org',
        cc=['author3@chromium.org'],
        components=[],
        labels=mock.ANY,
        merge_issue=None)
    message = self.add_bug_comment.call_args.kwargs['comment']
    self.assertIn('Found significant differences at 2 commits.', message)
    self.assertIn('https://example.com/repository/+/git_hash_3', message)
    self.assertIn('https://example.com/repository/+/git_hash_2', message)
    self.assertIn('https://example.com/repository/+/git_hash_1', message)
    labels = self.add_bug_comment.call_args.kwargs['labels']
    self.assertIn('Pinpoint-Job-Completed', labels)
    self.assertNotIn('-Pinpoint-Job-Completed', labels)
    self.assertIn('Pinpoint-Multiple-Culprits', labels)
    self.assertNotIn('-Pinpoint-Multiple-Culprits', labels)
    self.assertIn('Pinpoint-Multiple-MissingValues', labels)
    self.assertNotIn('-Pinpoint-Multiple-MissingValues', labels)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict',
              autospec=True)
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedMultipleDifferences_TenCulpritsCcTopTwo(
      self, differences, result_values, commit_as_dict):
    self.Parameterized_TestCompletedMultipleDifferences(10, 2, differences,
                                                        result_values,
                                                        commit_as_dict)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict',
              autospec=True)
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedMultipleDifferences_HundredCulpritsCcTopThree(
      self, differences, result_values, commit_as_dict):
    self.Parameterized_TestCompletedMultipleDifferences(100, 3, differences,
                                                        result_values,
                                                        commit_as_dict)

  def Parameterized_TestCompletedMultipleDifferences(self, number_culprits,
                                                     expected_num_ccs,
                                                     differences, result_values,
                                                     commit_as_dict):
    changes = [
        change.Change((change.Commit('chromium', 'git_hash_%d' % (i,)),))
        for i in range(1, number_culprits + 1)
    ]
    # Return [(None,c1), (c1,c2), (c2,c3), ...]
    differences.return_value = list(zip([None] + changes, changes))

    # Ensure culprits are ordered by deriving change results values from commit
    # names.  E.g.:
    #   Change(git_hash_1) -> result_value=[1],
    #   Change(git_hash_2) -> result_value=[4],
    # etc.
    def ResultValuesFromFakeGitHash(change_obj):
      if change_obj is None:
        return [0]
      v = int(change_obj.commits[0].git_hash[len('git_hash_'):])
      return [v * v]  # Square the value to ensure increasing deltas.

    result_values.side_effect = ResultValuesFromFakeGitHash
    commit_as_dict.side_effect = FakeCommitAsDict

    self.get_issue.return_value = {'status': 'Untriaged'}
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    expected_ccs = [
        'author%d@chromium.org' % (i,)
        for i in range(number_culprits, number_culprits - expected_num_ccs, -1)
    ]

    # We only CC folks from the top commits.
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        status='Assigned',
        owner=expected_ccs[0],
        cc=sorted(expected_ccs),
        components=[],
        labels=mock.ANY,
        merge_issue=None)
    labels = self.add_bug_comment.call_args[1]['labels']
    self.assertIn('Pinpoint-Job-Completed', labels)
    self.assertNotIn('-Pinpoint-Job-Completed', labels)
    self.assertIn('Pinpoint-Multiple-Culprits', labels)
    self.assertNotIn('-Pinpoint-Multiple-Culprits', labels)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedMultipleDifferences_NoDeltas(self, differences,
                                                result_values, commit_as_dict):
    """Regression test for http://crbug.com/1078680.

    Picks people to notify even when none of the differences have deltas (they
    are all transitions to/from "No values").
    """
    # Two differences, neither has deltas (50 -> No Values, No Values -> 50).
    c0 = change.Change((change.Commit('chromium', 'git_hash_0'),))
    c1 = change.Change((change.Commit('chromium', 'git_hash_1'),))
    c2 = change.Change((change.Commit('chromium', 'git_hash_2'),))
    change_map = {c0: [50], c1: [], c2: [50]}
    differences.return_value = [(c0, c1), (c1, c2)]
    result_values.side_effect = lambda c: change_map.get(c, [])
    commit_as_dict.side_effect = (
        {
            'repository': 'chromium',
            'git_hash': 'git_hash_1',
            'url': 'https://example.com/repository/+/git_hash_1',
            'author': 'author1@chromium.org',
            'subject': 'Subject.',
            'message': 'Subject.\n\nCommit message.',
        },
        {
            'repository': 'chromium',
            'git_hash': 'git_hash_2',
            'url': 'https://example.com/repository/+/git_hash_2',
            'author': 'author2@chromium.org',
            'subject': 'Subject.',
            'message': 'Subject.\n\nCommit message.',
        },
    )

    self.get_issue.return_value = {'status': 'Untriaged'}
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)

    # Notifies the owner of the first change in the list of differences, seeing
    # as they are all equally small.
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        status='Assigned',
        owner='author1@chromium.org',
        cc=['author1@chromium.org'],
        components=[],
        labels=mock.ANY,
        merge_issue=None)
    message = self.add_bug_comment.call_args.kwargs['comment']
    self.assertIn('Missing Values', message)
    self.assertIn('author1@chromium.org', message)
    self.assertIn('author2@chromium.org', message)
    labels = self.add_bug_comment.call_args.kwargs['labels']
    self.assertIn('Pinpoint-Job-Completed', labels)
    self.assertNotIn('-Pinpoint-Job-Completed', labels)
    self.assertIn('Pinpoint-Multiple-MissingValues', labels)
    self.assertNotIn('-Pinpoint-Multiple-MissingValues', labels)
    self.assertNotIn('Pinpoint-Multiple-Culprits', labels)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedWithAutoroll(self, differences, result_values,
                                commit_as_dict):
    c = change.Change((change.Commit('chromium', 'git_hash'),))
    differences.return_value = [(None, c)]
    result_values.side_effect = [20], [30]
    commit_as_dict.return_value = {
        'repository': 'chromium',
        'git_hash': 'git_hash',
        'url': 'https://example.com/repository/+/git_hash',
        'author': 'chromium-autoroll@skia-public.iam.gserviceaccount.com',
        'subject': 'Subject.',
        'message': 'Subject.\n\nCommit message.\n\nTBR=sheriff@bar.com',
    }

    self.get_issue.return_value = {'status': 'Untriaged'}
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    j.put()
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        issue_id=123456,
        project_name='chromium',
        comment=mock.ANY,
        status='Assigned',
        owner='sheriff@bar.com',
        cc=['chromium-autoroll@skia-public.iam.gserviceaccount.com'],
        components=[],
        labels=mock.ANY,
        merge_issue=None)
    message = self.add_bug_comment.call_args.kwargs['comment']
    self.assertIn('Found a significant difference at 1 commit.', message)
    self.assertIn('chromium-autoroll@skia-public.iam.gserviceaccount.com',
                  message)
    labels = self.add_bug_comment.call_args.kwargs['labels']
    self.assertIn('Pinpoint-Job-Completed', labels)
    self.assertNotIn('-Pinpoint-Job-Completed', labels)
    self.assertIn('Pinpoint-Culprit-Found', labels)
    self.assertNotIn('-Pinpoint-Culprit-Found', labels)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict')
  @mock.patch.object(job.job_state.JobState, 'ResultValues')
  @mock.patch.object(job.job_state.JobState, 'Differences')
  def testCompletedWithAutorollCulpritButNotMostRecent(self, differences,
                                                       result_values,
                                                       commit_as_dict):
    """Regression test for http://crbug.com/1076756.

    When an autoroll has the biggest delta, assigns to its sheriff even when it
    is not the latest change.
    """
    c0 = change.Change((change.Commit('chromium', 'git_hash_0'),))
    c1 = change.Change((change.Commit('chromium', 'git_hash_1'),))
    c2 = change.Change((change.Commit('chromium', 'git_hash_2'),))
    change_map = {c0: [0], c1: [10], c2: [10]}
    differences.return_value = [(c0, c1), (c1, c2)]
    result_values.side_effect = lambda c: change_map.get(c, [])
    commit_as_dict.side_effect = (
        {
            'repository': 'chromium',
            'git_hash': 'git_hash_1',
            'url': 'https://example.com/repository/+/git_hash_1',
            'author': 'chromium-autoroll@skia-public.iam.gserviceaccount.com',
            'subject': 'Subject.',
            'message': 'Subject.\n\nCommit message.\n\nTBR=sheriff@bar.com',
        },
        {
            'repository': 'chromium',
            'git_hash': 'git_hash_2',
            'url': 'https://example.com/repository/+/git_hash_2',
            'author': 'author2@chromium.org',
            'subject': 'Subject.',
            'message': 'Subject.\n\nCommit message.',
        },
    )

    self.get_issue.return_value = {'status': 'Untriaged'}
    j = job.Job.New((), (), bug_id=123456, comparison_mode='performance')
    j.put()
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    self.assertFalse(j.failed)
    self.add_bug_comment.assert_called_once_with(
        issue_id=mock.ANY,
        project_name='chromium',
        comment=mock.ANY,
        status='Assigned',
        owner='sheriff@bar.com',
        cc=['chromium-autoroll@skia-public.iam.gserviceaccount.com'],
        components=[],
        labels=mock.ANY,
        merge_issue=None)

  @mock.patch.object(job.job_state.JobState, 'ScheduleWork',
                     mock.MagicMock(side_effect=AssertionError('Error string')))
  def testFailed(self):
    j = job.Job.New((), (), bug_id=123456)
    scheduler.Schedule(j)
    with self.assertRaises(AssertionError):
      j.Run()

    self.ExecuteDeferredTasks('default')
    self.assertTrue(j.failed)
    self.add_bug_comment.assert_called_once_with(
        123456,
        'chromium',
        comment=_COMMENT_FAILED,
        send_email=True,
        labels=mock.ANY)
    labels = self.add_bug_comment.call_args[1]['labels']
    self.assertIn('Pinpoint-Job-Failed', labels)

  @mock.patch.object(job.job_state.JobState, 'ScheduleWork',
                     mock.MagicMock(side_effect=AssertionError('Error string')))
  def testFailed_ExceptionDetailsFieldAdded(self):
    j = job.Job.New((), (), bug_id=123456)
    scheduler.Schedule(j)
    with self.assertRaises(AssertionError):
      j.Run()

    j.exception = j.exception_details['traceback']
    exception_details = job.Job.exception_details
    delattr(job.Job, 'exception_details')
    j.put()
    self.assertTrue(j.failed)
    self.assertFalse(hasattr(j, 'exception_details'))
    job.Job.exception_details = exception_details
    j = j.key.get(use_cache=False)
    self.assertTrue(j.failed)
    self.assertTrue(hasattr(j, 'exception_details'))
    self.assertEqual(j.exception, j.exception_details['traceback'])
    self.assertTrue(
        j.exception_details['message'] in j.exception.splitlines()[-1])

  @mock.patch('dashboard.services.gerrit_service.PostChangeComment')
  def testCompletedUpdatesGerrit(self, post_change_comment):
    j = job.Job.New((), (),
                    gerrit_server='https://review.com',
                    gerrit_change_id='123456')
    scheduler.Schedule(j)
    j.Run()
    self.ExecuteDeferredTasks('default')
    post_change_comment.assert_called_once_with('https://review.com', '123456',
                                                _COMMENT_CODE_REVIEW)

@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
class GetImprovementDirectionTest(testing_common.TestCase):

  def testGetImprovementDirection(self):
    # create metric and improvement directions
    t = graph_data.TestMetadata(id='ChromiumPerf/win7/dromaeo/down',)
    t.improvement_direction = anomaly.DOWN
    t.put()
    t = graph_data.TestMetadata(id='ChromiumPerf/win7/dromaeo/up',)
    t.improvement_direction = anomaly.UP
    t.put()
    t = graph_data.TestMetadata(id='ChromiumPerf/win7/dromaeo/unknown',)
    t.put()

    # test _getImprovementDirection
    self.PatchDatastoreHooksRequest()
    app = Flask(__name__)
    with app.test_request_context('dummy/path', 'GET'):
      j = job.Job.New((), (),
                      tags={'test_path': "ChromiumPerf/win7/dromaeo/down"})
      self.assertEqual(j._GetImprovementDirection(), anomaly.DOWN)
      j = job.Job.New((), (),
                      tags={'test_path': "ChromiumPerf/win7/dromaeo/up"})
      self.assertEqual(j._GetImprovementDirection(), anomaly.UP)
      j = job.Job.New((), (),
                      tags={'test_path': "ChromiumPerf/win7/dromaeo/unknown"})
      self.assertEqual(j._GetImprovementDirection(), anomaly.UNKNOWN)


class GetIterationCountTest(test.TestCase):

  def testEvenlyDivisibleBots(self):
    self.assertEqual(
        job.GetIterationCount(initial_attempt_count=6), 6)
    self.assertEqual(
        job.GetIterationCount(initial_attempt_count=12), 12)

  def testOddAttemptCount(self):
    self.assertEqual(
        job.GetIterationCount(initial_attempt_count=5), 6)
