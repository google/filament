# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
import json
from unittest import mock

from dashboard.common import bot_configurations, namespaced_stored_object
from dashboard.pinpoint import test
from dashboard.pinpoint.handlers import job as job_handler
from dashboard.pinpoint.handlers import new as new_module
from dashboard.models import anomaly
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import results2 as results2_module

TEST_ARGS = {
    'comparisonMode': 'performance',
    'target': 'performance_test_suite',
    'startGitHash': 'd9ac8dd553c566b8fe107dd8c8b2275c2c9c27f1',
    'endGitHash': '81a6a08061d9a2da7413021bce961d125dc40ca2',
    'initialAttemptCount': 20,
    'configuration': 'win-10_laptop_low_end-perf',
    'benchmark': 'blink_perf.owp_storage',
    'story': 'blob-perf-shm.html',
    'chart': 'blob-perf-shm',
    'comparisonMagnitude': 21.9925,
    'project': 'chromium'
}

TEST_CHANGE = {
    'commits': [{
        'gitHash': 'd9ac8dd553c566b8fe107dd8c8b2275c2c9c27f1',
        'repository': 'https://chromium.googlesource.com/chromium/src.git',
        'url': ('https://chromium.googlesource.com/chromium/src.git/+/'
                'd9ac8dd553c566b8fe107dd8c8b2275c2c9c27f1'),
        'author': ('chromium-internal-autoroll@skia-corp.google.com.iam.'
                   'gserviceaccount.com'),
        'created': {
            'seconds': 1715203672,
            'nanos': 523378460
        },
        'subject':
            ('Roll clank/internal/apps from aa5f21626312 to cf4e4bc14dfa '
             '(1 revision)'),
        'message':
            ('https://chrome-internal.googlesource.com/clank/internal/apps.'
             'git/+log/aa5f21626312..cf4e4bc14dfa\n\nIf this roll has '
             'caused a breakage, revert this CL and stop the roller\nusing '
             'the controls here:\nhttps://skia-autoroll.corp.goog/r/clank-'
             'apps-chromium-autoroll\nPlease CC chrome-brapp-engprod@google'
             '.com,haileywang@google.com on the revert to ensure that a '
             'human\nis aware of the problem.\n\nTo report a problem with '
             'the AutoRoller itself, please file a bug:\nhttps://issues.'
             'skia.org/issues/new?component=1389291&template=1850622\n\n'
             'Documentation for the AutoRoller is here:\nhttps://skia.'
             'googlesource.com/buildbot/+doc/main/autoroll/README.md\n\n'
             'Bug: None\nTbr: haileywang@google.com\nNo-Try: true\nChange-'
             'Id: Ia639005699512a1af35c52e898ac2d9b37d9f941\nReviewed-on: '
             'https://chromium-review.googlesource.com/c/chromium/src/+/'
             '5448433\nCommit-Queue: chromium-internal-autoroll <chromium-'
             'internal-autoroll@skia-corp.google.com.iam.gserviceaccount.'
             'com>\nBot-Commit: chromium-internal-autoroll <chromium-'
             'internal-autoroll@skia-corp.google.com.iam.gserviceaccount.'
             'com>\nCr-Commit-Position: refs/heads/main@{#1286049}\n'),
        'commitBranch': 'refs/heads/main',
        'commitPosition': 1286049,
        'reviewUrl':
            ('https://chromium-review.googlesource.com/c/chromium/src/+/'
             '5448433'),
        'changeId': 'Ia639005699512a1af35c52e898ac2d9b37d9f941'
    }]
}

TEST_ATTEMPT = {
    'executions': [{
        'completed':
            True,
        'exception':
            None,
        'details': [{
            'key': 'builder',
            'value': ''
        }, {
            'key':
                'isolate',
            'value':
                '',
            'url': ('https://cas-viewer.appspot.com/projects/chrome-swarming/'
                    'instances/default_instance/blobs/72b3779783071eb0ac3d730b'
                    '5b020bdcc7667cee0d036964c386a5721ea33fb0/183/tree')
        }]
    }, {
        'completed':
            True,
        'exception':
            None,
        'details': [{
            'key': 'bot',
            'value': 'win-80-e504',
            'url': 'https://chrome-swarming.appspot.com/bot?id=win-80-e504'
        }, {
            'key':
                'task',
            'value':
                '69341cde448cdc10',
            'url':
                'https://chrome-swarming.appspot.com/task?id=69341cde448cdc10'
        }, {
            'key':
                'isolate',
            'value': ('72b3779783071eb0ac3d730b5b020bdcc7667cee0d036964c386'
                      'a5721ea33fb0/183'),
            'url': ('https://cas-viewer.appspot.com/projects/chrome-swarming/'
                    'instances/default_instance/blobs/72b3779783071eb0ac3d730b'
                    '5b020bdcc7667cee0d036964c386a5721ea33fb0/183/tree')
        }]
    }, {
        'completed': True,
        'exception': None,
        'details': []
    }],
    'resultValues': [
        42, 40.200000000186265, 45.60000000009313, 57, 41, 38.60000000009313
    ]
}

TEST_JOB = {
    "jobId": "a692835d-c71e-4700-abd5-65a0d2c9a5f9",
    "configuration": "win-10_laptop_low_end-perf",
    "improvementDirection": 1,
    "arguments": TEST_ARGS,
    "project": "chromium",
    "comparisonMode": "performance",
    "name": "[Test Skia] Performance bisect test change",
    "user": "chromeperf@appspot.gserviceaccount.com",
    "created": {
        "seconds": 1715203672,
        "nanos": 523378460
    },
    "updated": {
        "seconds": 1715203672
    },
    "differenceCount": 2,
    "bots": [
        "win-78-e504",
        "win-80-e504",
        "win-96-e504",
    ],
    "metric": "blob-perf-shm",
    "quests": ["Build", "Test", "Get values"],
    "state": [{
        "change": TEST_CHANGE,
        "attempts": [TEST_ATTEMPT],
    }]
}


@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
@mock.patch('dashboard.common.cloud_metric._PublishTSCloudMetric',
            mock.MagicMock())
class JobTest(test.TestCase):

  @mock.patch.object(results2_module, 'GetCachedResults2', return_value="")
  def testGet_NotStarted(self, _):
    job = job_module.Job.New((), ())
    data = json.loads(self.testapp.get('/api/job/' + job.job_id).body)
    self.assertEqual(job.created.isoformat(), data['created'])

  @mock.patch.object(results2_module, 'GetCachedResults2', return_value="")
  def testGet_Started(self, _):
    job = job_module.Job.New((), ())
    job.started = True
    job.started_time = datetime.datetime.utcnow()
    job.put()

    data = json.loads(self.testapp.get('/api/job/' + job.job_id).body)
    self.assertEqual(job.started_time.isoformat(), data['started_time'])


class MarshalObjectsTest(test.TestCase):

  def testParseRepoName_GitEnding_Chromium(self):
    self.assertEqual(
        job_handler.ParseRepoName(
            'https://chromium.googlesource.com/chromium/src.git'),
        'chromium',
    )

  def testParseRepoName_NoGitEnding_V8(self):
    self.assertEqual(
        job_handler.ParseRepoName('https://chromium.googlesource.com/v8/v8'),
        'v8',
    )

  def testMarshalArguments_SkiaResponse_AllStringValues(self):
    actual_args = job_handler.MarshalArguments(TEST_ARGS)
    # These are fields that are in camel case that should be replaced by
    # underscore delimited
    self.assertTrue('comparisonMagnitude' not in actual_args)
    self.assertEqual(
        str(TEST_ARGS['comparisonMagnitude']),
        actual_args['comparison_magnitude'])

    self.assertTrue('startGitHash' not in actual_args)
    self.assertEqual(TEST_ARGS['startGitHash'], actual_args['start_git_hash'])

    self.assertTrue('initialAttemptCount' not in actual_args)
    self.assertEqual(
        str(TEST_ARGS['initialAttemptCount']),
        actual_args['initial_attempt_count'])

  def testMarshalChange_ChromiumCommit_CommitObject(self):
    actual_change = job_handler.MarshalToChange(TEST_CHANGE)
    self.assertEqual(len(actual_change.commits), 1)
    actual_commit = actual_change.commits[0]

    self.assertEqual(actual_commit.repository, 'chromium')
    self.assertEqual(actual_commit.git_hash,
                     'd9ac8dd553c566b8fe107dd8c8b2275c2c9c27f1')

  def testMarshalAttempt_HappyPath_AttemptObject(self):
    namespaced_stored_object.Set(
        bot_configurations.BOT_CONFIGURATIONS_KEY, {
            'win-10_laptop_low_end-perf': {
                'builder': 'foo',
                'target': 'bar',
                'bucket': 'try',
                'swarming_server': 'https://chrome-swarming.appspot.com',
                'dimensions': [{
                    'key': 'pool',
                    'value': 'chrome.tests.pinpoint'
                }],
                'browser': 'somebrowser'
            },
        })

    test_args = job_handler.MarshalArguments(TEST_ARGS)
    new_args = new_module._ArgumentsWithConfiguration(test_args)
    quests = new_module._GenerateQuests(new_args)

    change_data = job_handler.MarshalToChange(TEST_CHANGE)

    actual_attempt = job_handler.MarshalToAttempt(quests, change_data,
                                                  TEST_ATTEMPT)

    self.assertEqual(tuple(quests), actual_attempt.quests)
    self.assertEqual(len(actual_attempt.executions), 3)

    run_test_exec = TEST_ATTEMPT['executions'][1]
    actual_run_test_exec = actual_attempt.executions[1]
    self.assertEqual(run_test_exec['details'][0]['value'],
                     actual_run_test_exec.bot_id)
    self.assertEqual(run_test_exec['details'][1]['value'],
                     actual_run_test_exec._task_id)
    actual_cas_ref = actual_run_test_exec._result_arguments['cas_root_ref']
    self.assertEqual(('72b3779783071eb0ac3d730b5b020bdcc7667cee0d036964c386a57'
                      '21ea33fb0'), actual_cas_ref['digest']['hash'])
    self.assertEqual('183', actual_cas_ref['digest']['sizeBytes'])
    self.assertEqual('projects/chrome-swarming/instances/default_instance',
                     actual_cas_ref['casInstance'])

  def testMarshalToJob_HappyPath_JobObject(self):
    namespaced_stored_object.Set(
        bot_configurations.BOT_CONFIGURATIONS_KEY, {
            'win-10_laptop_low_end-perf': {
                'builder': 'foo',
                'target': 'bar',
                'bucket': 'try',
                'swarming_server': 'https://chrome-swarming.appspot.com',
                'dimensions': [{
                    'key': 'pool',
                    'value': 'chrome.tests.pinpoint'
                }],
                'browser': 'somebrowser'
            },
        })

    # This includes using the provided arguments to create a job state object,
    # which in turn creates attempts and changes.
    _, actual_job = job_handler.MarshalToJob(TEST_JOB)
    self.assertEqual(2, actual_job.difference_count)
    self.assertEqual('blink_perf.owp_storage',
                     actual_job.benchmark_arguments.benchmark)
    self.assertEqual(3, len(actual_job.bots))

    # Job state
    actual_state = actual_job.state
    self.assertEqual(3, len(actual_state._quests))
    self.assertEqual(20, actual_state.attempt_count)
    self.assertEqual(anomaly.DOWN, actual_state._improvement_direction)
