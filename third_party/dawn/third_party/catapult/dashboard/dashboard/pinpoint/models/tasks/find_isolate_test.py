# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from unittest import mock

from dashboard.pinpoint import test
from dashboard.pinpoint.models import change as change_module
from dashboard.pinpoint.models import event as event_module
from dashboard.pinpoint.models import isolate
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import task as task_module
from dashboard.pinpoint.models.tasks import find_isolate


# The find_isolate Evaluator is special because it's meant to handle a "leaf"
# task in a graph, so we can test the evaluator on its own without setting up
# dependencies.
class FindIsolateEvaluatorBase(test.TestCase):

  def setUp(self):
    super().setUp()
    self.maxDiff = None
    with mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
                    mock.MagicMock(return_value=["a"])):
      self.job = job_module.Job.New((), ())
    task_module.PopulateTaskGraph(
        self.job,
        find_isolate.CreateGraph(
            find_isolate.TaskOptions(
                builder='Mac Builder',
                target='telemetry_perf_tests',
                bucket='luci.bucket',
                change=change_module.Change.FromDict({
                    'commits': [{
                        'repository': 'chromium',
                        'git_hash': '7c7e90be',
                    }],
                }))))


@mock.patch('dashboard.services.buildbucket_service.GetJobStatus')
@mock.patch('dashboard.services.buildbucket_service.Put')
class FindIsolateEvaluatorTest(FindIsolateEvaluatorBase):

  def testInitiate_FoundIsolate(self, *_):
    # Seed the isolate for this change.
    change = change_module.Change(
        commits=[change_module.Commit('chromium', '7c7e90be')])
    isolate.Put((('Mac Builder', change, 'telemetry_perf_tests',
                  'https://isolate.server', '7c7e90be'),))

    # Then ensure that we can find the seeded isolate for the specified
    # revision.
    self.assertDictEqual(
        {
            'find_isolate_chromium@7c7e90be': {
                'bucket': 'luci.bucket',
                'builder': 'Mac Builder',
                'change': mock.ANY,
                'isolate_hash': '7c7e90be',
                'isolate_server': 'https://isolate.server',
                'status': 'completed',
                'target': 'telemetry_perf_tests',
            },
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(
                type='initiate',
                target_task='find_isolate_chromium@7c7e90be',
                payload={}), find_isolate.Evaluator(self.job)))

  def testInitiate_ScheduleBuild(self, put, _):
    # We then need to make sure that the buildbucket put was called.
    put.return_value = {'build': {'id': '345982437987234'}}

    # This time we don't seed the isolate for the change to force the build.
    self.assertDictEqual(
        {
            'find_isolate_chromium@7c7e90be': {
                'bucket': 'luci.bucket',
                'buildbucket_result': {
                    'build': {
                        'id': '345982437987234'
                    },
                },
                'builder': 'Mac Builder',
                'change': mock.ANY,
                'status': 'ongoing',
                'target': 'telemetry_perf_tests',
                'tries': 1,
            },
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(
                type='initiate',
                target_task='find_isolate_chromium@7c7e90be',
                payload={}), find_isolate.Evaluator(self.job)))
    self.assertEqual(1, put.call_count)

  def testUpdate_BuildSuccessful(self, put, get_build_status):
    # First we're going to initiate so we have a build scheduled.
    put.return_value = {
        'build': {
            'id': '345982437987234',
            'url': 'https://some.buildbucket/url'
        }
    }
    self.assertDictEqual(
        {
            'find_isolate_chromium@7c7e90be': {
                'bucket': 'luci.bucket',
                'buildbucket_result': {
                    'build': {
                        'id': '345982437987234',
                        'url': 'https://some.buildbucket/url'
                    },
                },
                'builder': 'Mac Builder',
                'change': mock.ANY,
                'status': 'ongoing',
                'target': 'telemetry_perf_tests',
                'tries': 1,
            },
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(
                type='initiate',
                target_task='find_isolate_chromium@7c7e90be',
                payload={}), find_isolate.Evaluator(self.job)))
    self.assertEqual(1, put.call_count)

    # Now we send an update event which should cause us to poll the status of
    # the build on demand.
    json = """
    {
      "properties": {
          "got_revision_cp": "refs/heads/master@7c7e90be",
          "isolate_server": "https://isolate.server",
          "swarm_hashes_refs/heads/master(at)7c7e90be_without_patch":
              {"telemetry_perf_tests": "192923affe212adf"}
      }
    }"""
    get_build_status.return_value = {
        'build': {
            'status': 'COMPLETED',
            'result': 'SUCCESS',
            'result_details_json': json,
        }
    }
    self.assertDictEqual(
        {
            'find_isolate_chromium@7c7e90be': {
                'bucket': 'luci.bucket',
                'buildbucket_job_status': mock.ANY,
                'buildbucket_result': {
                    'build': {
                        'id': '345982437987234',
                        'url': 'https://some.buildbucket/url'
                    }
                },
                'builder': 'Mac Builder',
                'build_url': mock.ANY,
                'change': mock.ANY,
                'isolate_hash': '192923affe212adf',
                'isolate_server': 'https://isolate.server',
                'status': 'completed',
                'target': 'telemetry_perf_tests',
                'tries': 1,
            },
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(
                type='update',
                target_task='find_isolate_chromium@7c7e90be',
                payload={'status': 'build_completed'}),
            find_isolate.Evaluator(self.job)))
    self.assertEqual(1, get_build_status.call_count)
    self.assertEqual(
        {
            'find_isolate_chromium@7c7e90be': {
                'completed':
                    True,
                'exception':
                    None,
                'details': [{
                    'key': 'builder',
                    'value': 'Mac Builder',
                    'url': None,
                }, {
                    'key': 'build',
                    'value': '345982437987234',
                    'url': mock.ANY,
                }, {
                    'key':
                        'isolate',
                    'value':
                        '192923affe212adf',
                    'url':
                        'https://isolate.server/browse?digest=192923affe212adf',
                }]
            }
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(
                type='unimportant', target_task=None, payload={}),
            find_isolate.Serializer()))


@mock.patch('dashboard.services.buildbucket_service.GetJobStatus')
class FindIsolateEvaluatorUpdateTests(FindIsolateEvaluatorBase):

  def setUp(self):
    super().setUp()

    # Here we set up the pre-requisite for polling, where we've already had a
    # successful build scheduled.
    with mock.patch('dashboard.services.buildbucket_service.Put') as put:
      put.return_value = {'build': {'id': '345982437987234'}}
      self.assertDictEqual(
          {
              'find_isolate_chromium@7c7e90be': {
                  'buildbucket_result': {
                      'build': {
                          'id': '345982437987234'
                      }
                  },
                  'status': 'ongoing',
                  'builder': 'Mac Builder',
                  'bucket': 'luci.bucket',
                  'change': mock.ANY,
                  'target': 'telemetry_perf_tests',
                  'tries': 1,
              },
          },
          task_module.Evaluate(
              self.job,
              event_module.Event(
                  type='initiate',
                  target_task='find_isolate_chromium@7c7e90be',
                  payload={}), find_isolate.Evaluator(self.job)))
      self.assertEqual(1, put.call_count)

  def testUpdate_BuildFailed_HardFailure(self, get_build_status):
    get_build_status.return_value = {
        'build': {
            'status': 'COMPLETED',
            'result': 'FAILURE',
            'result_details_json': '{}',
        }
    }
    self.assertDictEqual(
        {
            'find_isolate_chromium@7c7e90be': {
                'bucket': 'luci.bucket',
                'buildbucket_result': {
                    'build': {
                        'id': '345982437987234'
                    },
                },
                'buildbucket_job_status': mock.ANY,
                'builder': 'Mac Builder',
                'build_url': mock.ANY,
                'change': mock.ANY,
                'status': 'failed',
                'target': 'telemetry_perf_tests',
                'errors': [mock.ANY],
                'tries': 1,
            },
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(
                type='update',
                target_task='find_isolate_chromium@7c7e90be',
                payload={'status': 'build_completed'}),
            find_isolate.Evaluator(self.job)))
    self.assertEqual(1, get_build_status.call_count)
    self.assertEqual(
        {
            'find_isolate_chromium@7c7e90be': {
                'completed':
                    True,
                'exception':
                    mock.ANY,
                'details': [{
                    'key': 'builder',
                    'value': 'Mac Builder',
                    'url': None,
                }, {
                    'key': 'build',
                    'value': '345982437987234',
                    'url': mock.ANY,
                }]
            }
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(
                type='unimportant', target_task=None, payload={}),
            find_isolate.Serializer()))

  def testUpdate_BuildFailed_Cancelled(self, get_build_status):
    get_build_status.return_value = {
        'build': {
            'status': 'COMPLETED',
            'result': 'CANCELLED',
            'result_details_json': '{}',
        }
    }
    self.assertDictEqual(
        {
            'find_isolate_chromium@7c7e90be': {
                'bucket': 'luci.bucket',
                'builder': 'Mac Builder',
                'buildbucket_result': {
                    'build': {
                        'id': '345982437987234'
                    }
                },
                'buildbucket_job_status': {
                    'status': 'COMPLETED',
                    'result': 'CANCELLED',
                    'result_details_json': '{}',
                },
                'build_url': mock.ANY,
                'change': mock.ANY,
                'errors': [mock.ANY],
                'status': 'cancelled',
                'target': 'telemetry_perf_tests',
                'tries': 1,
            },
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(
                type='update',
                target_task='find_isolate_chromium@7c7e90be',
                payload={'status': 'build_completed'}),
            find_isolate.Evaluator(self.job)))
    self.assertEqual(1, get_build_status.call_count)

  def testUpdate_MissingIsolates_Server(self, get_build_status):
    json = """
    {
      "properties": {
          "got_revision_cp": "refs/heads/master@7c7e90be",
          "swarm_hashes_refs/heads/master(at)7c7e90be_without_patch":
              {"telemetry_perf_tests": "192923affe212adf"}
      }
    }"""
    get_build_status.return_value = {
        'build': {
            'status': 'COMPLETED',
            'result': 'SUCCESS',
            'result_details_json': json,
        }
    }
    self.assertDictEqual(
        {
            'find_isolate_chromium@7c7e90be': {
                'bucket': 'luci.bucket',
                'buildbucket_result': {
                    'build': {
                        'id': '345982437987234'
                    }
                },
                'buildbucket_job_status': mock.ANY,
                'change': mock.ANY,
                'builder': 'Mac Builder',
                'build_url': mock.ANY,
                'status': 'failed',
                'errors': mock.ANY,
                'tries': 1,
                'target': 'telemetry_perf_tests',
            },
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(
                type='update',
                target_task='find_isolate_chromium@7c7e90be',
                payload={'status': 'build_completed'}),
            find_isolate.Evaluator(self.job)))
    self.assertEqual(1, get_build_status.call_count)

  def testUpdate_MissingIsolates_Revision(self, get_build_status):
    json = """
    {
      "properties": {
          "isolate_server": "https://isolate.server",
          "swarm_hashes_refs/heads/master(at)7c7e90be_without_patch":
              {"telemetry_perf_tests": "192923affe212adf"}
      }
    }"""
    get_build_status.return_value = {
        'build': {
            'status': 'COMPLETED',
            'result': 'SUCCESS',
            'result_details_json': json,
        }
    }
    self.assertDictEqual(
        {
            'find_isolate_chromium@7c7e90be': {
                'bucket': 'luci.bucket',
                'builder': 'Mac Builder',
                'build_url': mock.ANY,
                'buildbucket_result': {
                    'build': {
                        'id': '345982437987234'
                    }
                },
                'buildbucket_job_status': mock.ANY,
                'change': mock.ANY,
                'status': 'failed',
                'target': 'telemetry_perf_tests',
                'tries': 1,
                'errors': mock.ANY,
            },
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(
                type='update',
                target_task='find_isolate_chromium@7c7e90be',
                payload={'status': 'build_completed'}),
            find_isolate.Evaluator(self.job)))
    self.assertEqual(1, get_build_status.call_count)

  def testUpdate_MissingIsolates_Hashes(self, get_build_status):
    json = """
    {
      "properties": {
          "got_revision_cp": "refs/heads/master@7c7e90be",
          "isolate_server": "https://isolate.server"
      }
    }"""
    get_build_status.return_value = {
        'build': {
            'status': 'COMPLETED',
            'result': 'SUCCESS',
            'result_details_json': json,
        }
    }
    self.assertDictEqual(
        {
            'find_isolate_chromium@7c7e90be': {
                'bucket': 'luci.bucket',
                'builder': 'Mac Builder',
                'build_url': mock.ANY,
                'buildbucket_result': {
                    'build': {
                        'id': '345982437987234'
                    }
                },
                'buildbucket_job_status': mock.ANY,
                'change': mock.ANY,
                'status': 'failed',
                'errors': mock.ANY,
                'target': 'telemetry_perf_tests',
                'tries': 1,
            },
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(
                type='update',
                target_task='find_isolate_chromium@7c7e90be',
                payload={'status': 'build_completed'}),
            find_isolate.Evaluator(self.job)))
    self.assertEqual(1, get_build_status.call_count)

  def testUpdate_MissingIsolates_InvalidJson(self, get_build_status):
    json = '{ invalid }'
    get_build_status.return_value = {
        'build': {
            'status': 'COMPLETED',
            'result': 'SUCCESS',
            'result_details_json': json,
        }
    }
    self.assertDictEqual(
        {
            'find_isolate_chromium@7c7e90be': {
                'bucket': 'luci.bucket',
                'build_url': mock.ANY,
                'buildbucket_result': {
                    'build': {
                        'id': '345982437987234'
                    }
                },
                'buildbucket_job_status': mock.ANY,
                'builder': 'Mac Builder',
                'change': mock.ANY,
                'status': 'failed',
                'errors': mock.ANY,
                'target': 'telemetry_perf_tests',
                'tries': 1,
            },
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(
                type='update',
                target_task='find_isolate_chromium@7c7e90be',
                payload={'status': 'build_completed'}),
            find_isolate.Evaluator(self.job)))
    self.assertEqual(1, get_build_status.call_count)

  def testUpdate_BuildFailed_ScheduleRetry(self, *_):
    self.skipTest('Not implemented yet.')
