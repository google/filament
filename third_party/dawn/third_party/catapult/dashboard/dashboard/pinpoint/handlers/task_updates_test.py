# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import base64
import itertools
import json
from unittest import mock

from dashboard.pinpoint.handlers import task_updates
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import results2 as results2_module
from dashboard.pinpoint.models.tasks import bisection_test_util
from tracing.value import histogram as histogram_module
from tracing.value import histogram_set
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import reserved_infos


def FailsWithKeyError(*_):
  raise KeyError('Mock failure')


def CreateBuildUpdate(job, commit_id):
  return json.dumps({
      'message': {
          'attributes': {
              'nothing': 'important',
          },
          'data':
              base64.urlsafe_b64encode(
                  json.dumps({
                      'task_id':
                          'some_task_id',
                      'user_data':
                          json.dumps({
                              'job_id': job.job_id,
                              'task': {
                                  'type':
                                      'build',
                                  'id':
                                      'find_isolate_chromium@commit_%s' %
                                      (commit_id,)
                              }
                          })
                  }).encode('utf-8')).decode('utf-8')
      }
  })


def CreateTestUpdate(job, commit_id, attempt):
  return json.dumps({
      'message': {
          'attributes': {
              'nothing': 'important',
          },
          'data':
              base64.urlsafe_b64encode(
                  json.dumps({
                      'task_id':
                          'some_task_id',
                      'userdata':
                          json.dumps({
                              'job_id': job.job_id,
                              'task': {
                                  'type':
                                      'test',
                                  'id':
                                      'run_test_chromium@commit_%s_%s' %
                                      (commit_id, attempt)
                              }
                          })
                  }).encode('utf-8')).decode('utf-8')
      }
  })


@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
@mock.patch('dashboard.common.utils.ServiceAccountHttp', mock.MagicMock())
@mock.patch('dashboard.services.buildbucket_service.Put')
@mock.patch('dashboard.services.buildbucket_service.GetJobStatus')
@mock.patch('dashboard.common.cloud_metric.PublishPinpointJobStatusMetric',
            mock.MagicMock())
@mock.patch('dashboard.common.cloud_metric.PublishPinpointJobRunTimeMetric',
            mock.MagicMock())
class ExecutionEngineTaskUpdatesTest(bisection_test_util.BisectionTestBase):

  def setUp(self):
    self.maxDiff = None
    super().setUp()

  def testHandlerGoodCase(self, buildbucket_getjobstatus, buildbucket_put):
    buildbucket_put.return_value = {'build': {'id': '92384098123'}}
    buildbucket_getjobstatus.return_value = {
        'build': {
            'status':
                'COMPLETED',
            'result':
                'SUCCESS',
            'result_details_json':
                """
            {
              "properties": {
                "got_revision_cp": "refs/heads/master@commit_0",
                "isolate_server": "https://isolate.server",
                "swarm_hashes_refs/heads/master(at)commit_0_without_patch":
                  {"performance_telemetry_test": "1283497aaf223e0093"}
              }
            }
            """
        }
    }
    job = job_module.Job.New((), (),
                             comparison_mode='performance',
                             use_execution_engine=True,
                             arguments={'comparison_mode': 'performance'})
    self.PopulateSimpleBisectionGraph(job)
    task_updates.HandleTaskUpdate(
        json.dumps({
            'message': {
                'attributes': {
                    'key': 'value'
                },
                'data':
                    base64.urlsafe_b64encode(
                        json.dumps({
                            'task_id':
                                'some_id',
                            'userdata':
                                json.dumps({
                                    'job_id': job.job_id,
                                    'task': {
                                        # Use an ID that's not real.
                                        'id': '1',
                                        'type': 'build',
                                    }
                                }),
                        }).encode('utf-8')).decode('utf-8')
            }
        }))

  def testPostInvalidData(self, *_):
    with self.assertRaisesRegex(ValueError, 'Failed decoding `data`'):
      task_updates.HandleTaskUpdate(
          json.dumps({
              'message': {
                  'attributes': {
                      'nothing': 'important'
                  },
                  'data': '{"not": "base64-encoded"}',
              },
          }))

    with self.assertRaisesRegex(ValueError, 'Failed JSON parsing `data`'):
      task_updates.HandleTaskUpdate(
          json.dumps({
              'message': {
                  'attributes': {
                      'nothing': 'important'
                  },
                  'data':
                      base64.urlsafe_b64encode(b'not json formatted').decode(
                          'utf-8'),
              },
          }))

  @mock.patch(
      'dashboard.pinpoint.models.isolate.Get', side_effect=FailsWithKeyError)
  @mock.patch('dashboard.services.swarming.Tasks.New')
  @mock.patch('dashboard.services.swarming.Task.Result')
  @mock.patch('dashboard.services.isolate.Retrieve')
  @mock.patch.object(results2_module, 'GetCachedResults2', return_value='')
  @mock.patch(
      'dashboard.services.perf_issue_service_client.GetAlertGroupQuality',
      mock.MagicMock(return_value={'result': 'Good'}))
  def testExecutionEngineJobUpdates(self, _, isolate_retrieve,
                                    swarming_task_result, swarming_tasks_new,
                                    isolate_get, buildbucket_getjobstatus,
                                    buildbucket_put):
    buildbucket_put.return_value = {'build': {'id': '92384098123'}}
    buildbucket_getjobstatus.return_value = {
        'build': {
            'status':
                'COMPLETED',
            'result':
                'SUCCESS',
            'result_details_json':
                """
            {
              "properties": {
                "got_revision_cp": "refs/heads/master@commit_0",
                "isolate_server": "https://isolate.server",
                "swarm_hashes_refs/heads/master(at)commit_0_without_patch":
                  {"performance_telemetry_test": "1283497aaf223e0093"}
              }
            }
            """
        }
    }
    swarming_tasks_new.return_value = {'task_id': 'task id'}

    job = job_module.Job.New((), (),
                             use_execution_engine=True,
                             comparison_mode='performance')
    self.PopulateSimpleBisectionGraph(job)
    self.assertTrue(job.use_execution_engine)
    self.assertFalse(job.running)
    job.Start()

    # We are expecting two builds to be scheduled at the start of a bisection.
    self.assertEqual(2, isolate_get.call_count)
    self.assertEqual(2, buildbucket_put.call_count)

    # We expect no invocations of the job status.
    self.assertEqual(0, buildbucket_getjobstatus.call_count)

    # Expect to see the Build quest in the list of quests here.
    job = job_module.JobFromId(job.job_id)
    self.assertTrue(job.running)
    job_dict = job.AsDict([job_module.OPTION_STATE])
    self.assertEqual(['Build', 'Test'], job_dict.get('quests'))
    self.assertEqual(
        job_dict.get('state'), [{
            'attempts': [{
                'executions': [{
                    'completed': False,
                    'details': mock.ANY,
                    'exception': None,
                }, mock.ANY]
            }] + [mock.ANY] * 9,
            'change':
                self.start_change.AsDict(),
            'result_values': [],
            'comparisons': {
                'prev': None,
                'next': 'pending',
            }
        }, {
            'attempts': [{
                'executions': [{
                    'completed': False,
                    'details': mock.ANY,
                    'exception': None,
                }, mock.ANY]
            }] + [mock.ANY] * 9,
            'change':
                self.end_change.AsDict(),
            'result_values': [],
            'comparisons': {
                'prev': 'pending',
                'next': None,
            }
        }])

    # We then post an update and expect it to succeed.
    before_update_timestamp = job.updated

    task_updates.HandleTaskUpdate(CreateBuildUpdate(job, 5))

    # Here we expect one invocation of the getjobstatus call.
    self.assertEqual(1, buildbucket_getjobstatus.call_count)

    # And we expect that there's more than 1 call to the swarming service for
    # new tasks.
    self.assertGreater(swarming_tasks_new.call_count, 1)

    # Reload the job.
    job = job_module.JobFromId(job.job_id)
    self.assertTrue(job.started)
    self.assertFalse(job.completed)
    self.assertFalse(job.done)

    # Check that we can get the dict representation of the job.
    job_dict = job.AsDict([job_module.OPTION_STATE])
    self.assertEqual(job_dict.get('quests'), ['Build', 'Test'])
    self.assertEqual(
        job_dict.get('state'), [{
            'attempts': [{
                'executions': [{
                    'completed': False,
                    'details': mock.ANY,
                    'exception': None,
                }, mock.ANY]
            }] + [mock.ANY] * 9,
            'change':
                self.start_change.AsDict(),
            'result_values': [],
            'comparisons': {
                'prev': None,
                'next': 'pending',
            }
        }, {
            'attempts': [{
                'executions': [{
                    'completed': True,
                    'details': mock.ANY,
                    'exception': None,
                }, mock.ANY]
            }] + [mock.ANY] * 9,
            'change':
                self.end_change.AsDict(),
            'result_values': [],
            'comparisons': {
                'prev': 'pending',
                'next': None,
            }
        }])

    # Check that we did update the timestamp.
    self.assertNotEqual(before_update_timestamp, job.updated)

    # Then we post an update to complete all the builds.
    before_update_timestamp = job.updated
    task_updates.HandleTaskUpdate(CreateBuildUpdate(job, 0))

    # Reload the job.
    job = job_module.JobFromId(job.job_id)
    self.assertTrue(job.started)
    self.assertFalse(job.completed)
    self.assertFalse(job.done)

    # Check that we can get the dict representation of the job.
    job_dict = job.AsDict([job_module.OPTION_STATE])
    self.assertEqual(job_dict.get('quests'), ['Build', 'Test'])
    self.assertEqual(
        job_dict.get('state'), [{
            'attempts': [{
                'executions': [{
                    'completed': True,
                    'details': mock.ANY,
                    'exception': None,
                }, mock.ANY]
            }] + ([mock.ANY] * 9),
            'change':
                self.start_change.AsDict(),
            'result_values': [],
            'comparisons': {
                'prev': None,
                'next': 'pending',
            }
        }, {
            'attempts': [{
                'executions': [{
                    'completed': True,
                    'details': mock.ANY,
                    'exception': None,
                }, mock.ANY]
            }] + ([mock.ANY] * 9),
            'change':
                self.end_change.AsDict(),
            'result_values': [],
            'comparisons': {
                'prev': 'pending',
                'next': None,
            }
        }])

    # Check that we did update the timestamp.
    self.assertNotEqual(before_update_timestamp, job.updated)

    # Then send an update to all the tests finishing, and retrieving the
    # histograms as output.
    swarming_test_count = swarming_tasks_new.call_count

    def CreateHistogram(commit_id):
      histogram = histogram_module.Histogram('some_chart', 'count')
      histogram.AddSample(0 * commit_id)
      histogram.AddSample(1 * commit_id)
      histogram.AddSample(2 * commit_id)
      histograms = histogram_set.HistogramSet([histogram])
      histograms.AddSharedDiagnosticToAllHistograms(
          reserved_infos.STORY_TAGS.name,
          generic_set.GenericSet(['group:some_grouping_label']))
      histograms.AddSharedDiagnosticToAllHistograms(
          reserved_infos.STORIES.name, generic_set.GenericSet(['some_story']))
      return histograms

    for attempt, commit_id in itertools.chain(
        enumerate([5] * (swarming_test_count // 2)),
        enumerate([0] * (swarming_test_count // 2))):
      isolate_retrieve.side_effect = [
          ('{"files": {"some_benchmark/perf_results.json": '
           '{"h": "394890891823812873798734a"}}}'),
          json.dumps(CreateHistogram(commit_id).AsDicts())
      ]
      swarming_task_result.return_value = {
          'bot_id': 'bot id',
          'exit_code': 0,
          'failure': False,
          'outputs_ref': {
              'isolatedserver': 'https://isolate-server/',
              'isolated': '1298a009e9808f90e09812aad%s' % (attempt,),
          },
          'state': 'COMPLETED',
      }
      before_update_timestamp = job.updated
      task_updates.HandleTaskUpdate(CreateTestUpdate(job, commit_id, attempt))

      # Reload the job.
      job = job_module.JobFromId(job.job_id)

      # Check that we did update the timestamp.
      self.assertNotEqual(before_update_timestamp, job.updated)

    # With all the above done, we're not quite done yet, because we need to
    # ensure we're getting more nodes in the graph.
    job = job_module.JobFromId(job.job_id)
    self.assertTrue(job.started)
    self.assertFalse(job.completed)
    self.assertFalse(job.done)

    # Check that we can get the dict representation of the job.
    job_dict = job.AsDict([job_module.OPTION_STATE])
    self.assertEqual(job_dict.get('quests'), ['Build', 'Test', 'Get results'])
    self.assertEqual(
        job_dict.get('state'), [{
            'attempts': [mock.ANY] * 10,
            'change': self.start_change.AsDict(),
            'comparisons': {
                'prev': None,
                'next': 'pending',
            },
            'result_values': mock.ANY,
        }, {
            'attempts': [mock.ANY] * 10,
            'change': mock.ANY,
            'comparisons': {
                'prev': 'pending',
                'next': 'pending',
            },
            'result_values': mock.ANY,
        }, {
            'attempts': [mock.ANY] * 10,
            'change': mock.ANY,
            'comparisons': {
                'prev': 'pending',
                'next': 'pending',
            },
            'result_values': mock.ANY,
        }, {
            'attempts': [mock.ANY] * 10,
            'change': mock.ANY,
            'comparisons': {
                'prev': 'pending',
                'next': 'pending',
            },
            'result_values': mock.ANY,
        }, {
            'attempts': [mock.ANY] * 10,
            'change': self.end_change.AsDict(),
            'comparisons': {
                'prev': 'pending',
                'next': None,
            },
            'result_values': mock.ANY,
        }])

    for attempt, commit_id in itertools.chain(
        enumerate([2] * (swarming_test_count // 2)),
        enumerate([3] * (swarming_test_count // 2)),
        enumerate([4] * (swarming_test_count // 2))):
      isolate_retrieve.side_effect = [
          ('{"files": {"some_benchmark/perf_results.json": '
           '{"h": "394890891823812873798734a"}}}'),
          json.dumps(CreateHistogram(commit_id).AsDicts())
      ]
      swarming_task_result.return_value = {
          'bot_id': 'bot id',
          'exit_code': 0,
          'failure': False,
          'outputs_ref': {
              'isolatedserver': 'https://isolate-server/',
              'isolated': '1298a009e9808f90e09812aad%s' % (attempt,),
          },
          'state': 'COMPLETED',
      }
      buildbucket_getjobstatus.return_value = {
          'build': {
              'status':
                  'COMPLETED',
              'result':
                  'SUCCESS',
              'result_details_json':
                  """
              {
                "properties": {
                  "got_revision_cp": "refs/heads/master@commit_%s",
                  "isolate_server": "https://isolate.server",
                  "swarm_hashes_refs/heads/master(at)commit_%s_without_patch":
                    {"performance_telemetry_test": "1283497aaf223e0093"}
                }
              }
              """ % (commit_id, commit_id)
          }
      }
      task_updates.HandleTaskUpdate(CreateBuildUpdate(job, commit_id))
      task_updates.HandleTaskUpdate(CreateTestUpdate(job, commit_id, attempt))

    # At this point we'll see that we have commit_1 added to the graph, to build
    # and to test with.
    job = job_module.JobFromId(job.job_id)
    self.assertTrue(job.started)
    self.assertFalse(job.completed)
    self.assertFalse(job.done)

    job_dict = job.AsDict([job_module.OPTION_STATE])
    self.assertEqual(job_dict.get('quests'), ['Build', 'Test', 'Get results'])
    self.assertEqual(
        job_dict.get('state'), [{
            'attempts': [mock.ANY] * 10,
            'change': self.start_change.AsDict(),
            'comparisons': {
                'prev': None,
                'next': 'pending',
            },
            'result_values': mock.ANY,
        }, {
            'attempts': [mock.ANY] * 10,
            'change': mock.ANY,
            'comparisons': {
                'prev': 'pending',
                'next': 'pending',
            },
            'result_values': mock.ANY,
        }, {
            'attempts': [mock.ANY] * 10,
            'change': mock.ANY,
            'comparisons': {
                'prev': 'pending',
                'next': 'different',
            },
            'result_values': mock.ANY,
        }, {
            'attempts': [mock.ANY] * 10,
            'change': mock.ANY,
            'comparisons': {
                'prev': 'different',
                'next': 'different',
            },
            'result_values': mock.ANY,
        }, {
            'attempts': [mock.ANY] * 10,
            'change': mock.ANY,
            'comparisons': {
                'prev': 'different',
                'next': 'different',
            },
            'result_values': mock.ANY,
        }, {
            'attempts': [mock.ANY] * 10,
            'change': self.end_change.AsDict(),
            'comparisons': {
                'prev': 'different',
                'next': None,
            },
            'result_values': mock.ANY,
        }])

    # Run and complete all the pending tests.
    for attempt, commit_id in enumerate([1] * (swarming_test_count // 2)):
      isolate_retrieve.side_effect = [
          ('{"files": {"some_benchmark/perf_results.json": '
           '{"h": "394890891823812873798734a"}}}'),
          json.dumps(CreateHistogram(commit_id).AsDicts())
      ]
      swarming_task_result.return_value = {
          'bot_id': 'bot id',
          'exit_code': 0,
          'failure': False,
          'outputs_ref': {
              'isolatedserver': 'https://isolate-server/',
              'isolated': '1298a009e9808f90e09812aad%s' % (attempt,),
          },
          'state': 'COMPLETED',
      }

      buildbucket_getjobstatus.return_value = {
          'build': {
              'status':
                  'COMPLETED',
              'result':
                  'SUCCESS',
              'result_details_json':
                  """
              {
                "properties": {
                  "got_revision_cp": "refs/heads/master@commit_%s",
                  "isolate_server": "https://isolate.server",
                  "swarm_hashes_refs/heads/master(at)commit_%s_without_patch":
                    {"performance_telemetry_test": "1283497aaf223e0093"}
                }
              }
              """ % (commit_id, commit_id)
          }
      }
      task_updates.HandleTaskUpdate(CreateBuildUpdate(job, commit_id))
      task_updates.HandleTaskUpdate(CreateTestUpdate(job, commit_id, attempt))

    # Then ensure that we can find the culprits and differences.
    job = job_module.JobFromId(job.job_id)
    self.assertTrue(job.started)
    self.assertTrue(job.completed)
    self.assertTrue(job.done)
    self.assertFalse(job.running)

    job_dict = job.AsDict([job_module.OPTION_STATE])
    self.assertEqual(job_dict.get('difference_count'), 5)
    self.ExecuteDeferredTasks('default')
