# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
from unittest import mock

from dashboard.api import api_auth
from dashboard.common import datastore_hooks
from dashboard.common import namespaced_stored_object
from dashboard.common import stored_object
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.services import perf_issue_service_client
from dashboard.pinpoint import test
from dashboard.pinpoint.handlers import new
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import quest as quest_module
from dashboard.pinpoint.models.change import change_test

_JOB_URL_HOST = 'https://localhost:80'

# All arguments must have string values.
_BASE_REQUEST = {
    'target': 'telemetry_perf_tests',
    'configuration': 'chromium-rel-mac11-pro',
    'benchmark': 'speedometer',
    'bug_id': '12345',
    'base_git_hash': '3',
    'start_git_hash': '1',
    'end_git_hash': '3',
    'story': 'speedometer',
    'comparison_mode': 'performance',
    'initial_attempt_count': '10'
}

# TODO: Make this agnostic to the parameters the Quests take.
_CONFIGURATION_ARGUMENTS = {
    'browser': 'release',
    'builder': 'Mac Builder',
    'bucket': 'luci.bucket',
    'dimensions': '[{"key": "pool", "value": "cool pool"}]',
    'repository': 'chromium',
    'swarming_server': 'https://chromium-swarm.appspot.com',
}


class _NewTest(test.TestCase):

  def setUp(self):
    super().setUp()

    self.SetCurrentUserOAuth(testing_common.INTERNAL_USER)
    self.SetCurrentClientIdOAuth(api_auth.OAUTH_CLIENT_ID_ALLOWLIST[0])

    key = namespaced_stored_object.NamespaceKey('bot_configurations',
                                                datastore_hooks.INTERNAL)
    config_with_args = _CONFIGURATION_ARGUMENTS.copy()
    config_with_args.update({'extra_test_args': '--experimental-flag'})
    stored_object.Set(
        key, {
            'chromium-rel-mac11-pro': _CONFIGURATION_ARGUMENTS,
            'test-config-with-args': config_with_args,
        })


@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
class NewAuthTest(_NewTest):

  @mock.patch.object(api_auth, 'Authorize',
                     mock.MagicMock(side_effect=api_auth.NotLoggedInError()))
  def testPost_NotLoggedIn(self):
    self.SetCurrentUserOAuth(None)

    response = self.Post('/api/new', _BASE_REQUEST, status=401)
    result = json.loads(response.body)
    self.assertEqual(result, {'error': 'User not authenticated'})

  @mock.patch.object(api_auth, 'Authorize',
                     mock.MagicMock(side_effect=api_auth.OAuthError()))
  def testFailsOauth(self):
    self.SetCurrentUserOAuth(testing_common.EXTERNAL_USER)

    response = self.Post('/api/new', _BASE_REQUEST, status=403)
    result = json.loads(response.body)
    self.assertEqual(result, {'error': 'User authentication error'})


@mock.patch('dashboard.services.perf_issue_service_client.PostIssueComment',
            mock.MagicMock())
@mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
@mock.patch.object(api_auth, 'Authorize', mock.MagicMock())
@mock.patch.object(utils, 'IsTryjobUser', mock.MagicMock())
@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
@mock.patch('dashboard.common.cloud_metric.PublishPinpointJobStatusMetric',
            mock.MagicMock())
@mock.patch('dashboard.common.cloud_metric._PublishTSCloudMetric',
            mock.MagicMock())
class NewTest(_NewTest):

  def testPost(self):
    response = self.Post('/api/new', _BASE_REQUEST, status=200)
    result = json.loads(response.body)
    self.assertIn('jobId', result)
    self.assertEqual(result['jobUrl'],
                     _JOB_URL_HOST + '/job/%s' % result['jobId'])

  def testNoConfiguration(self):
    request = dict(_BASE_REQUEST)
    request.update(_CONFIGURATION_ARGUMENTS)
    del request['configuration']
    response = self.Post('/api/new', request, status=200)
    result = json.loads(response.body)
    self.assertIn('jobId', result)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertIsNotNone(job.batch_id)
    self.assertEqual(result['jobUrl'],
                     _JOB_URL_HOST + '/job/%s' % result['jobId'])

  def testBadConfiguration(self):
    request = dict(_BASE_REQUEST)
    request.update(_CONFIGURATION_ARGUMENTS)
    request['configuration'] = 'lalala'
    response = self.Post('/api/new', request, status=400)
    result = json.loads(response.body)
    self.assertIn('error', result)

  def testBrowserOverride(self):
    request = dict(_BASE_REQUEST)
    request['browser'] = 'debug'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertIsInstance(job.state._quests[1], quest_module.RunTelemetryTest)
    self.assertIn('debug', job.state._quests[1]._extra_args)
    self.assertNotIn('release', job.state._quests[1]._extra_args)

  def testComparisonModeFunctional(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'functional'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.comparison_mode, 'functional')

  def testComparisonModePerformance(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'performance'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.comparison_mode, 'performance')

  def testComparisonModePerformance_ApplyPatch(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'performance'
    request['patch'] = 'https://lalala/c/foo/bar/+/123'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.comparison_mode, 'performance')
    self.assertEqual(
        job.state._changes[0].id_string,
        'chromium@1 + %s' % ('https://lalala/repo~branch~id/abc123',))
    self.assertEqual(
        job.state._changes[1].id_string,
        'chromium@3 + %s' % ('https://lalala/repo~branch~id/abc123',))

  def testComparisonModeTry(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'try'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.comparison_mode, 'try')
    self.assertEqual(job.state._changes[0].id_string, 'chromium@3')
    self.assertEqual(job.state._changes[1].id_string, 'chromium@3')

  def testDifferentAttemptCount(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'try'
    request['initial_attempt_count'] = '2'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.state._initial_attempt_count, 2)

  def testComparisonModeTry_ApplyPatch(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'try'
    request['patch'] = 'https://lalala/c/foo/bar/+/123'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.comparison_mode, 'try')
    self.assertEqual(job.state._changes[0].id_string, 'chromium@3')
    self.assertEqual(
        job.state._changes[1].id_string,
        'chromium@3 + %s' % ('https://lalala/repo~branch~id/abc123',))

  def testComparisonModeTry_InvalidPatch(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'try'
    request['patch'] = 'https://lalala/c/123/4'
    response = self.Post('/api/new', request, status=400)
    self.assertIn('error', json.loads(response.body))

  def testComparisonModeTry_ApplyBaseAndExperimentPatchLegacy(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'try'
    request['base_patch'] = 'https://lalala/c/foo/bar/+/122'
    request['patch'] = 'https://lalala/c/foo/bar/+/123'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.comparison_mode, 'try')
    self.assertEqual(
        job.state._changes[1].id_string,
        'chromium@3 + %s' % ('https://lalala/repo~branch~id/abc123',))
    self.assertEqual(
        job.state._changes[0].id_string,
        'chromium@3 + %s' % ('https://lalala/repo~branch~id/abc123',))

  def testComparisonModeTry_ApplyBaseAndExperimentPatch(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'try'
    request['base_patch'] = 'https://lalala/c/foo/bar/+/122'
    request['experiment_patch'] = 'https://lalala/c/foo/bar/+/123'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.comparison_mode, 'try')
    self.assertEqual(
        job.state._changes[1].id_string,
        'chromium@3 + %s' % ('https://lalala/repo~branch~id/abc123',))
    self.assertEqual(
        job.state._changes[0].id_string,
        'chromium@3 + %s' % ('https://lalala/repo~branch~id/abc123',))

  def testComparisonModeTry_BaseAndExpFlags(self):
    request = dict(_BASE_REQUEST)
    del request['end_git_hash']
    del request['start_git_hash']
    request['comparison_mode'] = 'try'
    base_args = [
        '--extra-browser-args',
        'something',
    ]
    exp_args = [
        '--extra-browser-args',
        'something-else',
    ]
    request['base_extra_args'] = json.dumps(base_args)
    request['experiment_extra_args'] = json.dumps(exp_args)
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.comparison_mode, 'try')
    self.assertEqual(
        str(job.state._changes[0]),
        'base: chromium@3 (%s) (Variant: 0)' % (', '.join(base_args)),
    )
    self.assertEqual(
        str(job.state._changes[1]),
        'exp: chromium@3 (%s) (Variant: 1)' % (', '.join(exp_args)),
    )

  def testComparisonModeTry_BaseNoPatchAndExperimentCommitPatch(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'try'
    request['end_git_hash'] = '60061ec0de'
    request['experiment_patch'] = 'https://lalala/c/foo/bar/+/123'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.comparison_mode, 'try')
    self.assertEqual(
        job.state._changes[1].id_string,
        'chromium@60061ec0de + %s' % ('https://lalala/repo~branch~id/abc123',))
    self.assertEqual(job.state._changes[0].id_string, 'chromium@3')

  def testComparisonModeTry_MissingRequiredArgs(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'try'
    del request['story']
    response = self.Post('/api/new', request, status=400)
    self.assertIn('error', json.loads(response.body))

  def testComparisonModeTry_MissingBaseGitHash(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'try'
    del request['base_git_hash']
    response = self.Post('/api/new', request, status=400)
    self.assertIn('error', json.loads(response.body))

  def testComparisonModeTry_SupportDebugTrace(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'try'
    request['end_git_hash'] = 'f00d'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.comparison_mode, 'try')
    self.assertEqual(job.state._changes[0].id_string, 'chromium@3')
    self.assertEqual(job.state._changes[1].id_string, 'chromium@f00d')

  def testComparisonModeTry_SupportDebugTraceWithPatch(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'try'
    request['end_git_hash'] = 'f00d'
    request['patch'] = 'https://lalala/c/foo/bar/+/123'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.comparison_mode, 'try')
    self.assertEqual(
        job.state._changes[0].id_string,
        'chromium@3 + %s' % ('https://lalala/repo~branch~id/abc123',))
    self.assertEqual(
        job.state._changes[1].id_string,
        'chromium@f00d + %s' % ('https://lalala/repo~branch~id/abc123',))

  def testComparisonModeUnknown(self):
    request = dict(_BASE_REQUEST)
    request['comparison_mode'] = 'invalid comparison mode'
    response = self.Post('/api/new', request, status=400)
    self.assertIn('error', json.loads(response.body))

  def testComparisonMagnitude(self):
    request = dict(_BASE_REQUEST)
    request['comparison_magnitude'] = '123.456'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.state._comparison_magnitude, 123.456)

  def testQuests(self):
    request = dict(_BASE_REQUEST)
    request['quests'] = ['FindIsolate', 'RunTelemetryTest']
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(len(job.state._quests), 2)
    self.assertIsInstance(job.state._quests[0], quest_module.FindIsolate)
    self.assertIsInstance(job.state._quests[1], quest_module.RunTelemetryTest)

  def testQuestsString(self):
    request = dict(_BASE_REQUEST)
    request['quests'] = 'FindIsolate,RunTelemetryTest'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(len(job.state._quests), 2)
    self.assertIsInstance(job.state._quests[0], quest_module.FindIsolate)
    self.assertIsInstance(job.state._quests[1], quest_module.RunTelemetryTest)

  def testUnknownQuest(self):
    request = dict(_BASE_REQUEST)
    request['quests'] = 'FindIsolate,UnknownQuest'
    response = self.Post('/api/new', request, status=400)
    self.assertIn('error', json.loads(response.body))

  def testWithChanges(self):
    request = dict(_BASE_REQUEST)
    del request['start_git_hash']
    del request['end_git_hash']
    request['changes'] = json.dumps([{
        'commits': [{
            'repository': 'chromium',
            'git_hash': '1'
        }]
    }, {
        'commits': [{
            'repository': 'chromium',
            'git_hash': '3'
        }]
    }])

    response = self.Post('/api/new', request, status=200)
    result = json.loads(response.body)
    self.assertIn('jobId', result)
    self.assertEqual(result['jobUrl'],
                     _JOB_URL_HOST + '/job/%s' % result['jobId'])

  def testWithPatch(self):
    request = dict(_BASE_REQUEST)
    request['patch'] = 'https://lalala/c/foo/bar/+/123'

    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual('https://lalala', job.gerrit_server)
    self.assertEqual('repo~branch~id', job.gerrit_change_id)

  def testMissingTarget(self):
    request = dict(_BASE_REQUEST)
    del request['target']
    response = self.Post('/api/new', request, status=200)
    self.assertNotIn('error', json.loads(response.body))

  def testEmptyTarget(self):
    request = dict(_BASE_REQUEST)
    request['target'] = ''
    response = self.Post('/api/new', request, status=200)
    self.assertNotIn('error', json.loads(response.body))

  def testFallbackTarget(self):
    request = dict(_BASE_REQUEST)
    request['target'] = 'performance_test_suite_android_chrome'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    quests = job.state._quests
    # Make sure we only have one quest with a fallback target and its fallback
    # target is performance_test_suite.
    self.assertEqual(
        ['performance_test_suite'],
        [q._fallback_target for q in quests if hasattr(q, '_fallback_target')])

  def testInvalidTestConfig(self):
    request = dict(_BASE_REQUEST)
    del request['configuration']
    response = self.Post('/api/new', request, status=400)
    self.assertIn('error', json.loads(response.body))

  def testInvalidBug(self):
    request = dict(_BASE_REQUEST)
    request['bug_id'] = 'not_an_int'
    response = self.Post('/api/new', request, status=400)
    self.assertEqual({'error': new._ERROR_BUG_ID}, json.loads(response.body))

  def testEmptyBug(self):
    request = dict(_BASE_REQUEST)
    request['bug_id'] = ''
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertIsNone(job.bug_id)

  def testPin(self):
    request = dict(_BASE_REQUEST)
    request['pin'] = 'https://codereview.com/c/foo/bar/+/123'

    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.state._pin, change_test.Change(patch=True))

  def testValidTags(self):
    request = dict(_BASE_REQUEST)
    request['tags'] = json.dumps({'key': 'value'})
    response = self.Post('/api/new', request, status=200)
    result = json.loads(response.body)
    self.assertIn('jobId', result)

  def testInvalidTags(self):
    request = dict(_BASE_REQUEST)
    request['tags'] = json.dumps(['abc'])
    response = self.Post('/api/new', request, status=400)
    self.assertIn('error', json.loads(response.body))

  def testInvalidTagType(self):
    request = dict(_BASE_REQUEST)
    request['tags'] = json.dumps({'abc': 123})
    response = self.Post('/api/new', request, status=400)
    self.assertIn('error', json.loads(response.body))

  def testInvalidPriority(self):
    request = dict(_BASE_REQUEST)
    request['priority'] = 'unsupported'
    response = self.Post('/api/new', request, status=400)
    self.assertIn('error', json.loads(response.body))

  def testPriorityIsAString(self):
    request = dict(_BASE_REQUEST)
    request['priority'] = '10'
    response = self.Post('/api/new', request, status=200)
    self.assertNotIn('error', json.loads(response.body))

  def testUserFromParams(self):
    request = dict(_BASE_REQUEST)
    request['user'] = 'foo@example.org'
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.user, 'foo@example.org')

  def testUserFromAuth(self):
    response = self.Post('/api/new', _BASE_REQUEST, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.user, testing_common.INTERNAL_USER.email())

  def testNewHasBenchmarkArgs(self):
    request = dict(_BASE_REQUEST)
    request.update({
        'chart': 'some_chart',
        'story': 'some_story',
        'story_tags': 'some_tag,some_other_tag'
    })
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertIsNotNone(job.benchmark_arguments)
    self.assertEqual('speedometer', job.benchmark_arguments.benchmark)
    self.assertEqual('some_story', job.benchmark_arguments.story)
    self.assertEqual('some_tag,some_other_tag',
                     job.benchmark_arguments.story_tags)
    self.assertEqual('some_chart', job.benchmark_arguments.chart)
    self.assertEqual(None, job.benchmark_arguments.statistic)

  def testNewHasBatchId(self):
    request = dict(_BASE_REQUEST)
    request.update({
        'chart': 'some_chart',
        'story': 'some_story',
        'story_tags': 'some_tag,some_other_tag',
        'batch_id': 'some-identifier',
    })
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(job.batch_id, 'some-identifier')

  def testNewPostsCreationMessage(self):
    post_issue = perf_issue_service_client.PostIssueComment
    post_issue.return_value = None

    request = dict(_BASE_REQUEST)
    request.update({
        'chart': 'some_chart',
        'story': 'some_story',
        'story_tags': 'some_tag,some_other_tag'
    })
    response = self.Post('/api/new', request, status=200)

    self.assertIsNotNone(
        job_module.JobFromId(json.loads(response.body)['jobId']))
    self.ExecuteDeferredTasks('default')

    post_issue.assert_called_once_with(
        12345, 'chromium', comment=mock.ANY, send_email=True)
    message = post_issue.call_args.kwargs['comment']
    self.assertIn('Pinpoint job created and queued.', message)

  def testExtraArgsSupported(self):
    request = dict(_BASE_REQUEST)
    request.update({
        'extra_test_args': '["--provided-args"]',
        'configuration': 'test-config-with-args',
    })
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])

    # Validate that the arguments are only the input arguments.
    self.assertEqual(
        job.arguments.get('extra_test_args'), json.dumps(['--provided-args']))

    # And that the RunTest instance has the extra arguments.
    for quest in job.state._quests:
      if isinstance(quest,
                    (quest_module.RunGTest, quest_module.RunTelemetryTest)):
        self.assertIn('--experimental-flag', quest._extra_args)

  def testNewUsingExecutionEngine(self):
    request = dict(_BASE_REQUEST)
    request.update({
        'chart': 'some_chart',
        'story': 'some_story',
        'story_tags': 'some_tag,some_other_tag',
        'experimental_execution_engine': 'on',
        'target': 'performance_test_suite',
        'comparison_mode': 'performance',
    })
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertIsNotNone(job.benchmark_arguments)
    self.assertEqual('speedometer', job.benchmark_arguments.benchmark)
    self.assertEqual('some_story', job.benchmark_arguments.story)
    self.assertEqual('some_tag,some_other_tag',
                     job.benchmark_arguments.story_tags)
    self.assertEqual('some_chart', job.benchmark_arguments.chart)
    self.assertEqual(None, job.benchmark_arguments.statistic)
    self.assertTrue(job.use_execution_engine)

  def testVrQuest(self):
    request = dict(_BASE_REQUEST)
    request['target'] = 'vr_perf_tests'
    configuration = dict(_CONFIGURATION_ARGUMENTS)
    configuration['browser'] = 'android-chromium-bundle'
    request.update(configuration)
    del request['configuration']
    response = self.Post('/api/new', request, status=200)
    job = job_module.JobFromId(json.loads(response.body)['jobId'])
    self.assertEqual(len(job.state._quests), 3)
    self.assertIsInstance(job.state._quests[0], quest_module.FindIsolate)
    self.assertIsInstance(job.state._quests[1], quest_module.RunVrTelemetryTest)
    self.assertIsInstance(job.state._quests[2], quest_module.ReadValue)
