# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import unittest

from unittest import mock

from dashboard.pinpoint import test
from dashboard.pinpoint.models import errors
from dashboard.pinpoint.models import isolate
from dashboard.pinpoint.models.change import change_test
from dashboard.pinpoint.models.quest import find_isolate
import six

FakeJob = collections.namedtuple('Job',
                                 ['job_id', 'url', 'comparison_mode', 'user'])


class FindIsolateQuestTest(unittest.TestCase):

  def testMissingBuilder(self):
    arguments = {
        'builder': 'Mac Builder',
        'target': 'telemetry_perf_tests',
        'bucket': 'luci.bucket',
        'comparison_mode': 'try'
    }
    del arguments['builder']
    with self.assertRaises(TypeError):
      find_isolate.FindIsolate.FromDict(arguments)

  def testMissingTarget(self):
    arguments = {
        'builder': 'Mac Builder',
        'target': 'telemetry_perf_tests',
        'bucket': 'luci.bucket',
        'comparison_mode': 'try'
    }
    del arguments['target']
    with self.assertRaises(TypeError):
      find_isolate.FindIsolate.FromDict(arguments)

  def testMissingBucket(self):
    arguments = {
        'builder': 'Mac Builder',
        'target': 'telemetry_perf_tests',
        'bucket': 'luci.bucket',
        'comparison_mode': 'try'
    }
    del arguments['bucket']
    with self.assertRaises(TypeError):
      find_isolate.FindIsolate.FromDict(arguments)

  def testAllArguments(self):
    arguments = {
        'builder': 'Mac Builder',
        'target': 'telemetry_perf_tests',
        'bucket': 'luci.bucket',
        'comparison_mode': 'try'
    }
    expected = find_isolate.FindIsolate(
        'Mac Builder',
        'telemetry_perf_tests',
        'luci.bucket',
        comparison_mode='try')
    self.assertEqual(find_isolate.FindIsolate.FromDict(arguments), expected)


class _FindIsolateExecutionTest(test.TestCase):

  def setUp(self):
    super().setUp()

    change = change_test.Change(123)
    isolate.Put((('Mac Builder', change, 'telemetry_perf_tests',
                  'isolate.server', '7c7e90be'),))

    change2 = change_test.Change(999)
    isolate.Put((('linux-builder-perf', change2, 'telemetry_perf_tests',
                  'isolate.server', '1212abcd'),))

  def assertExecutionFailure(self, execution, exception_class):
    self.assertTrue(execution.completed)
    self.assertTrue(execution.failed)
    self.assertIsInstance(execution.exception['traceback'], six.string_types)
    last_exception_line = execution.exception['traceback'].splitlines()[-1]
    self.assertTrue(exception_class.__name__ in last_exception_line)
    self.assertEqual(execution.result_arguments, {})

  def assertExecutionSuccess(self, execution):
    self.assertTrue(execution.completed)
    self.assertFalse(execution.failed)
    self.assertIsNone(execution.exception)


class IsolateLookupTest(_FindIsolateExecutionTest):

  def testIsolateLookupSuccess(self):
    quest = find_isolate.FindIsolate('Mac Builder', 'telemetry_perf_tests',
                                     'luci.bucket')

    # Propagate a thing that looks like a job.
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'performance',
                'user@example.com'))

    execution = quest.Start(change_test.Change(123))
    execution.Poll()

    expected_result_arguments = {
        'isolate_server': 'isolate.server',
        'isolate_hash': '7c7e90be',
    }
    expected_url = 'https://cas-viewer.appspot.com/{}/blobs/{}/tree'.format(
        'isolate.server', '7c7e90be')
    expected_as_dict = {
        'completed':
            True,
        'exception':
            None,
        'details': [
            {
                'key': 'builder',
                'value': 'Mac Builder',
            },
            {
                'key': 'isolate',
                'value': '7c7e90be',
                'url': expected_url,
            },
        ],
    }
    self.assertExecutionSuccess(execution)
    self.assertEqual(execution.result_values, ())
    self.assertEqual(execution.result_arguments, expected_result_arguments)
    self.assertEqual(execution.AsDict(), expected_as_dict)

  def testIsolateLookupWaterfallAlias(self):
    quest = find_isolate.FindIsolate('Linux Builder Perf',
                                     'telemetry_perf_tests', 'luci.bucket')

    # Propagate a thing that looks like a job.
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'performance',
                'user@example.com'))

    execution = quest.Start(change_test.Change(999))
    execution.Poll()

    expected_result_arguments = {
        'isolate_server': 'isolate.server',
        'isolate_hash': '1212abcd',
    }
    expected_url = 'https://cas-viewer.appspot.com/{}/blobs/{}/tree'.format(
        'isolate.server', '1212abcd')
    expected_as_dict = {
        'completed':
            True,
        'exception':
            None,
        'details': [
            {
                'key': 'builder',
                'value': 'Linux Builder Perf',
            },
            {
                'key': 'isolate',
                'value': '1212abcd',
                'url': expected_url,
            },
        ],
    }
    self.assertExecutionSuccess(execution)
    self.assertEqual(execution.result_values, ())
    self.assertEqual(execution.result_arguments, expected_result_arguments)
    self.assertEqual(execution.AsDict(), expected_as_dict)


  def testIsolateLookupFallbackSuccess(self):
    quest = find_isolate.FindIsolate(
        'Mac Builder',
        'not_real_target',
        'luci.bucket',
        fallback_target='telemetry_perf_tests')

    # Propagate a thing that looks like a job.
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'performance',
                'user@example.com'))

    execution = quest.Start(change_test.Change(123))
    execution.Poll()

    expected_result_arguments = {
        'isolate_server': 'isolate.server',
        'isolate_hash': '7c7e90be',
    }
    expected_url = 'https://cas-viewer.appspot.com/{}/blobs/{}/tree'.format(
        'isolate.server', '7c7e90be')
    expected_as_dict = {
        'completed':
            True,
        'exception':
            None,
        'details': [
            {
                'key': 'builder',
                'value': 'Mac Builder',
            },
            {
                'key': 'isolate',
                'value': '7c7e90be',
                'url': expected_url,
            },
        ],
    }
    self.assertExecutionSuccess(execution)
    self.assertEqual(execution.result_values, ())
    self.assertEqual(execution.result_arguments, expected_result_arguments)
    self.assertEqual(execution.AsDict(), expected_as_dict)

  def testIsolateLookupFailure(self):
    quest = find_isolate.FindIsolate(
        'Mac Builder',
        'not_real_target',
        'luci.bucket',
        fallback_target='also_not_real_target')

    # Propagate a thing that looks like a job.
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'performance',
                'user@example.com'))

    execution = quest.Start(change_test.Change(123))
    execution._build = True
    with mock.patch(
        'dashboard.services.buildbucket_service.GetJobStatus',
        return_value={
            'build': {
                'url':
                    'foo',
                'status':
                    'COMPLETED',
                'result':
                    '',
                'result_details_json':
                    """{
                         "properties": {
                           "got_revision_cp": "",
                           "swarm_hashes__without_patch": {}
                         }
                       }""",
            }
        }):
      with self.assertRaises(errors.BuildIsolateNotFound):
        execution.Poll()


@mock.patch('dashboard.services.buildbucket_service.GetJobStatus')
@mock.patch('dashboard.services.buildbucket_service.Put')
class BuildTest(_FindIsolateExecutionTest):

  def FakePutReturn(self):
    return {'id': 'build_id_2'}

  def testBuildNoReviewUrl(self, put, _):
    change = change_test.Change(123, 456, patch=True)
    results = change.base_commit.AsDict()
    del results['review_url']
    change.base_commit.AsDict = mock.MagicMock(return_value=results)

    quest = find_isolate.FindIsolate('Mac Builder', 'telemetry_perf_tests',
                                     'luci.bucket')
    execution = quest.Start(change)
    del execution._bucket

    # Request a build.
    put.return_value = self.FakePutReturn()
    execution.Poll()

    self.assertExecutionFailure(execution, errors.BuildGerritUrlNotFound)

  def testBuildNoBucket(self, put, _):
    change = change_test.Change(123, 456, patch=True)
    quest = find_isolate.FindIsolate('Mac Builder', 'telemetry_perf_tests',
                                     'luci.bucket')
    execution = quest.Start(change)
    del execution._bucket

    # Request a build.
    put.return_value = self.FakePutReturn()
    execution.Poll()

    self.assertFalse(execution.completed)
    put.assert_called_once_with(
        find_isolate.BUCKET, [
            'buildset:patch/gerrit/codereview.com/567890/5',
            'buildset:commit/gitiles/chromium.googlesource.com/'
            'project/name/+/commit_123'
        ], {
            'builder_name': 'Mac Builder',
            'properties': {
                'clobber': False,
                'revision': 'commit_123',
                'deps_revision_overrides': {
                    test.CATAPULT_URL: 'commit_456'
                },
                'git_repo': test.CHROMIUM_URL,
                'staging': False,
                'patch_gerrit_url': 'https://codereview.com',
                'patch_issue': 567890,
                'patch_project': 'project/name',
                'patch_ref': 'refs/changes/90/567890/5',
                'patch_repository_url': test.CHROMIUM_URL,
                'patch_set': 5,
                'patch_storage': 'gerrit',
            }
        })


  def testBuildLifecycle(self, put, get_job_status):
    change = change_test.Change(123, 456, patch=True)
    quest = find_isolate.FindIsolate('Mac Builder', 'telemetry_perf_tests',
                                     'luci.bucket')
    # Propagate a thing that looks like a job.
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'performance',
                'user@example.com'))
    execution = quest.Start(change)

    # Request a build.
    put.return_value = self.FakePutReturn()
    execution.Poll()

    self.assertFalse(execution.completed)
    put.assert_called_once_with(
        'luci.bucket', [
            'buildset:patch/gerrit/codereview.com/567890/5',
            'buildset:commit/gitiles/chromium.googlesource.com/'
            'project/name/+/commit_123',
            'pinpoint_job_id:cafef00d',
            'pinpoint_user:user@example.com',
            'pinpoint_url:https://pinpoint/cafef00d',
        ], {
            'builder_name': 'Mac Builder',
            'properties': {
                'clobber': False,
                'revision': 'commit_123',
                'deps_revision_overrides': {
                    test.CATAPULT_URL: 'commit_456'
                },
                'git_repo': test.CHROMIUM_URL,
                'staging': False,
                'patch_gerrit_url': 'https://codereview.com',
                'patch_issue': 567890,
                'patch_project': 'project/name',
                'patch_ref': 'refs/changes/90/567890/5',
                'patch_repository_url': test.CHROMIUM_URL,
                'patch_set': 5,
                'patch_storage': 'gerrit',
            },
        })

    # Check build status.
    get_job_status.return_value = {
        'id': 'build_id_2',
        'status': 'STARTED',
    }
    execution.Poll()

    self.assertFalse(execution.completed)
    get_job_status.assert_called_once_with('build_id_2')

    # Look up isolate hash.
    get_job_status.return_value = {
        'id': 'build_id_2',
        'status': 'SUCCESS',
    }
    isolate.Put((('Mac Builder', change, 'telemetry_perf_tests',
                  'isolate.server', 'isolate git hash'),))
    execution.Poll()

    expected_result_arguments = {
        'isolate_server': 'isolate.server',
        'isolate_hash': 'isolate git hash',
    }
    expected_url = 'https://cas-viewer.appspot.com/{}/blobs/{}/tree'.format(
        'isolate.server', 'isolate git hash')
    expected_as_dict = {
        'completed':
            True,
        'exception':
            None,
        'details': [
            {
                'key': 'builder',
                'value': 'Mac Builder',
            },
            {
                'key': 'build',
                'value': 'build_id_2',
                'url': 'https://ci.chromium.org/b/build_id_2',
            },
            {
                'key': 'isolate',
                'value': 'isolate git hash',
                'url': expected_url,
            },
        ],
    }
    self.assertExecutionSuccess(execution)
    self.assertEqual(execution.result_values, ())
    self.assertEqual(execution.result_arguments, expected_result_arguments)
    self.assertEqual(execution.AsDict(), expected_as_dict)


  def testSimultaneousBuilds(self, put, get_job_status):
    # Two builds started at the same time on the same Change should reuse the
    # same build request.
    change = change_test.Change(0)
    quest = find_isolate.FindIsolate('Mac Builder', 'telemetry_perf_tests',
                                     'luci.bucket')
    execution_1 = quest.Start(change)
    execution_2 = quest.Start(change)

    # Request a build.
    put.return_value = self.FakePutReturn()
    execution_1.Poll()
    execution_2.Poll()

    self.assertFalse(execution_1.completed)
    self.assertFalse(execution_2.completed)
    self.assertEqual(put.call_count, 1)

    # Check build status.
    get_job_status.return_value = {'status': 'STARTED'}
    execution_1.Poll()
    execution_2.Poll()

    self.assertFalse(execution_1.completed)
    self.assertFalse(execution_2.completed)
    self.assertEqual(get_job_status.call_count, 2)

    # Look up isolate hash.
    get_job_status.return_value = {'status': 'SUCCESS'}
    isolate.Put((('Mac Builder', change, 'telemetry_perf_tests',
                  'isolate.server', 'isolate git hash'),))
    execution_1.Poll()
    execution_2.Poll()

    self.assertExecutionSuccess(execution_1)
    self.assertExecutionSuccess(execution_2)


  def testBuildFailure(self, put, get_job_status):
    quest = find_isolate.FindIsolate('Mac Builder', 'telemetry_perf_tests',
                                     'luci.bucket')
    execution = quest.Start(change_test.Change(0))

    # Request a build.
    put.return_value = self.FakePutReturn()
    execution.Poll()

    # Check build status.
    get_job_status.return_value = {
        'status': 'FAILURE',
    }
    execution.Poll()

    self.assertExecutionFailure(execution, errors.BuildFailed)

  def testBuildFailureFatal(self, put, get_job_status):
    quest = find_isolate.FindIsolate(
        'Mac Builder',
        'telemetry_perf_tests',
        'luci.bucket',
        comparison_mode='try')
    execution = quest.Start(change_test.Change(0))

    # Request a build.
    put.return_value = self.FakePutReturn()
    execution.Poll()

    # Check build status.
    get_job_status.return_value = {
        'status': 'FAILURE',
    }

    with self.assertRaises(errors.BuildFailedFatal):
      execution.Poll()

  def testBuildCanceled(self, put, get_job_status):
    quest = find_isolate.FindIsolate('Mac Builder', 'telemetry_perf_tests',
                                     'luci.bucket')
    execution = quest.Start(change_test.Change(0))

    # Request a build.
    put.return_value = self.FakePutReturn()
    execution.Poll()

    # Check build status.
    get_job_status.return_value = {
        'status': 'CANCELED',
        "statusDetails": {
            "timeout": {}
        },
    }

    execution.Poll()

    self.assertExecutionFailure(execution, errors.BuildCancelled)

  def testBuildCanceledFatal(self, put, get_job_status):
    quest = find_isolate.FindIsolate(
        'Mac Builder',
        'telemetry_perf_tests',
        'luci.bucket',
        comparison_mode='try')
    execution = quest.Start(change_test.Change(0))

    # Request a build.
    put.return_value = self.FakePutReturn()
    execution.Poll()

    # Check build status.
    get_job_status.return_value = {
        'status': 'CANCELED',
        "statusDetails": {
            "timeout": {}
        },
    }

    with self.assertRaises(errors.BuildCancelledFatal):
      execution.Poll()

  def testBuildSucceededButIsolateIsMissing(self, put, get_job_status):
    quest = find_isolate.FindIsolate('Mac Builder', 'telemetry_perf_tests',
                                     'luci.bucket')
    execution = quest.Start(change_test.Change(0))

    # Request a build.
    put.return_value = self.FakePutReturn()
    execution.Poll()

    # Check build status.
    get_job_status.return_value = {
        'build': {
            'status':
                'COMPLETED',
            'result':
                'SUCCESS',
            'result_details_json':
                """{
                "properties": {
                    "got_revision_cp": "refs/heads/master@{#123}",
                    "isolate_server": "isolate.server",
                    "swarm_hashes_refs/heads/master(at){#123}_without_patch": {}
                }
            }""",
        }
    }
    with self.assertRaises(errors.BuildIsolateNotFound):
      execution.Poll()

  def testBuildSucceedsAndIsolateIsCached(self, *_):
    self.testBuildLifecycle()  # pylint: disable=no-value-for-parameter
    cached_isolate = isolate.Get('Mac Builder',
                                 change_test.Change(123, 456, patch=True),
                                 'telemetry_perf_tests')
    self.assertIsNotNone(cached_isolate)
