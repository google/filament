# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import functools
from unittest import mock

from dashboard.pinpoint import test
from dashboard.pinpoint.models import change as change_module
from dashboard.pinpoint.models import evaluators
from dashboard.pinpoint.models import event as event_module
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import task as task_module
from dashboard.pinpoint.models.tasks import find_isolate
from dashboard.pinpoint.models.tasks import run_test
from dashboard.pinpoint.models.tasks import bisection_test_util

DIMENSIONS = [
    {
        'key': 'pool',
        'value': 'Chrome-perf-pinpoint'
    },
    {
        'key': 'key',
        'value': 'value'
    },
]


@mock.patch('dashboard.services.swarming.Tasks.New')
@mock.patch('dashboard.services.swarming.Task.Result')
class EvaluatorTest(test.TestCase):

  def setUp(self):
    super().setUp()
    self.maxDiff = None
    with mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
                    mock.MagicMock(return_value=["a"])):
      self.job = job_module.Job.New((), ())
    task_module.PopulateTaskGraph(
        self.job,
        run_test.CreateGraph(
            run_test.TaskOptions(
                build_options=find_isolate.TaskOptions(
                    builder='Some Builder',
                    target='telemetry_perf_tests',
                    bucket='luci.bucket',
                    change=change_module.Change.FromDict({
                        'commits': [{
                            'repository': 'chromium',
                            'git_hash': 'aaaaaaa',
                        }]
                    })),
                swarming_server='some_server',
                dimensions=DIMENSIONS,
                extra_args=[],
                attempts=10)))

  def testEvaluateToCompletion(self, swarming_task_result, swarming_tasks_new):
    swarming_tasks_new.return_value = {'task_id': 'task id'}
    evaluator = evaluators.SequenceEvaluator(
        evaluators=(
            evaluators.FilteringEvaluator(
                predicate=evaluators.TaskTypeEq('find_isolate'),
                delegate=evaluators.SequenceEvaluator(
                    evaluators=(bisection_test_util.FakeFoundIsolate(self.job),
                                evaluators.TaskPayloadLiftingEvaluator()))),
            run_test.Evaluator(self.job),
        ))
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            evaluator))

    # Ensure that we've found all the 'run_test' tasks.
    self.assertEqual(
        {
            'run_test_chromium@aaaaaaa_%s' % (attempt,): {
                'status': 'ongoing',
                'swarming_server': 'some_server',
                'dimensions': DIMENSIONS,
                'extra_args': [],
                'swarming_request_body': {
                    'name': mock.ANY,
                    'user': mock.ANY,
                    'priority': mock.ANY,
                    'task_slices': mock.ANY,
                    'tags': mock.ANY,
                    'pubsub_auth_token': mock.ANY,
                    'pubsub_topic': mock.ANY,
                    'pubsub_userdata': mock.ANY,
                    'service_account': mock.ANY,
                },
                'swarming_task_id': 'task id',
                'tries': 1,
                'change': mock.ANY,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='run_test')))

    # Ensure that we've actually made the calls to the Swarming service.
    swarming_tasks_new.assert_called()
    self.assertGreaterEqual(swarming_tasks_new.call_count, 10)

    # Then we propagate an event for each of the run_test tasks in the graph.
    swarming_task_result.return_value = {
        'bot_id': 'bot id',
        'exit_code': 0,
        'failure': False,
        'outputs_ref': {
            'isolatedserver': 'output isolate server',
            'isolated': 'output isolate hash',
        },
        'state': 'COMPLETED',
    }
    for attempt in range(10):
      self.assertNotEqual(
          {},
          task_module.Evaluate(
              self.job,
              event_module.Event(
                  type='update',
                  target_task='run_test_chromium@aaaaaaa_%s' % (attempt,),
                  payload={}), evaluator), 'Attempt #%s' % (attempt,))

    # Ensure that we've polled the status of each of the tasks, and that we've
    # marked the tasks completed.
    self.assertEqual(
        {
            'run_test_chromium@aaaaaaa_%s' % (attempt,): {
                'status': 'completed',
                'swarming_server': 'some_server',
                'dimensions': DIMENSIONS,
                'extra_args': [],
                'swarming_request_body': {
                    'name': mock.ANY,
                    'user': mock.ANY,
                    'priority': mock.ANY,
                    'task_slices': mock.ANY,
                    'tags': mock.ANY,
                    'pubsub_auth_token': mock.ANY,
                    'pubsub_topic': mock.ANY,
                    'pubsub_userdata': mock.ANY,
                    'service_account': mock.ANY,
                },
                'swarming_task_result': {
                    'bot_id': mock.ANY,
                    'state': 'COMPLETED',
                    'failure': False,
                },
                'isolate_server': 'output isolate server',
                'isolate_hash': 'output isolate hash',
                'swarming_task_id': 'task id',
                'tries': 1,
                'change': mock.ANY,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='run_test')))

    # Ensure that we've actually made the calls to the Swarming service.
    swarming_task_result.assert_called()
    self.assertGreaterEqual(swarming_task_result.call_count, 10)

  def testEvaluateToCompletion_CAS(self, swarming_task_result,
                                   swarming_tasks_new):
    swarming_tasks_new.return_value = {'task_id': 'task id'}
    evaluator = evaluators.SequenceEvaluator(
        evaluators=(
            evaluators.FilteringEvaluator(
                predicate=evaluators.TaskTypeEq('find_isolate'),
                delegate=evaluators.SequenceEvaluator(
                    evaluators=(bisection_test_util.FakeFoundIsolate(self.job),
                                evaluators.TaskPayloadLiftingEvaluator()))),
            run_test.Evaluator(self.job),
        ))
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            evaluator))

    # Ensure that we've found all the 'run_test' tasks.
    self.assertEqual(
        {
            'run_test_chromium@aaaaaaa_%s' % (attempt,): {
                'status': 'ongoing',
                'swarming_server': 'some_server',
                'dimensions': DIMENSIONS,
                'extra_args': [],
                'swarming_request_body': {
                    'name': mock.ANY,
                    'user': mock.ANY,
                    'priority': mock.ANY,
                    'task_slices': mock.ANY,
                    'tags': mock.ANY,
                    'pubsub_auth_token': mock.ANY,
                    'pubsub_topic': mock.ANY,
                    'pubsub_userdata': mock.ANY,
                    'service_account': mock.ANY,
                },
                'swarming_task_id': 'task id',
                'tries': 1,
                'change': mock.ANY,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='run_test')))

    # Ensure that we've actually made the calls to the Swarming service.
    swarming_tasks_new.assert_called()
    self.assertGreaterEqual(swarming_tasks_new.call_count, 10)

    # Then we propagate an event for each of the run_test tasks in the graph.
    swarming_task_result.return_value = {
        'bot_id': 'bot id',
        'exit_code': 0,
        'failure': False,
        'cas_output_root': {
            'cas_instance': 'output cas server',
            'digest': {
                'hash': 'output cas hash',
                "byteSize": 91,
            },
        },
        'state': 'COMPLETED',
    }
    for attempt in range(10):
      self.assertNotEqual(
          {},
          task_module.Evaluate(
              self.job,
              event_module.Event(
                  type='update',
                  target_task='run_test_chromium@aaaaaaa_%s' % (attempt,),
                  payload={}), evaluator), 'Attempt #%s' % (attempt,))

    # Ensure that we've polled the status of each of the tasks, and that we've
    # marked the tasks completed.
    self.assertEqual(
        {
            'run_test_chromium@aaaaaaa_%s' % (attempt,): {
                'status': 'completed',
                'swarming_server': 'some_server',
                'dimensions': DIMENSIONS,
                'extra_args': [],
                'swarming_request_body': {
                    'name': mock.ANY,
                    'user': mock.ANY,
                    'priority': mock.ANY,
                    'task_slices': mock.ANY,
                    'tags': mock.ANY,
                    'pubsub_auth_token': mock.ANY,
                    'pubsub_topic': mock.ANY,
                    'pubsub_userdata': mock.ANY,
                    'service_account': mock.ANY,
                },
                'swarming_task_result': {
                    'bot_id': mock.ANY,
                    'state': 'COMPLETED',
                    'failure': False,
                },
                'cas_root_ref': {
                    'cas_instance': 'output cas server',
                    'digest': {
                        'hash': 'output cas hash',
                        "byteSize": 91,
                    },
                },
                'swarming_task_id': 'task id',
                'tries': 1,
                'change': mock.ANY,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='run_test')))

    # Ensure that we've actually made the calls to the Swarming service.
    swarming_task_result.assert_called()
    self.assertGreaterEqual(swarming_task_result.call_count, 10)

  def testEvaluateFailedDependency(self, *_):
    evaluator = evaluators.SequenceEvaluator(
        evaluators=(
            evaluators.FilteringEvaluator(
                predicate=evaluators.TaskTypeEq('find_isolate'),
                delegate=evaluators.SequenceEvaluator(
                    evaluators=(
                        bisection_test_util.FakeFindIsolateFailed(self.job),
                        evaluators.TaskPayloadLiftingEvaluator()))),
            run_test.Evaluator(self.job),
        ))

    # When we initiate the run_test tasks, we should immediately see the tasks
    # failing because the dependency has a hard failure status.
    self.assertEqual(
        dict([('find_isolate_chromium@aaaaaaa', mock.ANY)] +
             [('run_test_chromium@aaaaaaa_%s' % (attempt,), {
                 'status': 'failed',
                 'errors': mock.ANY,
                 'dimensions': DIMENSIONS,
                 'extra_args': [],
                 'swarming_server': 'some_server',
                 'change': mock.ANY,
                 'index': attempt,
             }) for attempt in range(10)]),
        task_module.Evaluate(
            self.job,
            event_module.Event(type='initiate', target_task=None, payload={}),
            evaluator))

  def testEvaluatePendingDependency(self, *_):
    # Ensure that tasks stay pending in the event of an update.
    self.assertEqual(
        dict([('find_isolate_chromium@aaaaaaa', {
            'builder': 'Some Builder',
            'target': 'telemetry_perf_tests',
            'bucket': 'luci.bucket',
            'change': mock.ANY,
            'status': 'pending',
        })] + [('run_test_chromium@aaaaaaa_%s' % (attempt,), {
            'status': 'pending',
            'dimensions': DIMENSIONS,
            'extra_args': [],
            'swarming_server': 'some_server',
            'change': mock.ANY,
            'index': attempt,
        }) for attempt in range(10)]),
        task_module.Evaluate(
            self.job,
            event_module.Event(
                type='update',
                target_task=None,
                payload={
                    'kind': 'synthetic',
                    'action': 'poll'
                }), run_test.Evaluator(self.job)))

  @mock.patch('dashboard.services.swarming.Task.Stdout')
  def testEvaluateHandleFailures_Hard(self, swarming_task_stdout,
                                      swarming_task_result, swarming_tasks_new):
    swarming_tasks_new.return_value = {'task_id': 'task id'}
    evaluator = evaluators.SequenceEvaluator(
        evaluators=(
            evaluators.FilteringEvaluator(
                predicate=evaluators.TaskTypeEq('find_isolate'),
                delegate=evaluators.SequenceEvaluator(
                    evaluators=(bisection_test_util.FakeFoundIsolate(self.job),
                                evaluators.TaskPayloadLiftingEvaluator()))),
            run_test.Evaluator(self.job),
        ))
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            evaluator))

    # We set it up so that when we poll the swarming task, that we're going to
    # get an error status. We're expecting that hard failures are detected.
    swarming_task_stdout.return_value = {
        'output':
            """Traceback (most recent call last):
  File "../../testing/scripts/run_performance_tests.py", line 282, in <module>
    sys.exit(main())
  File "../../testing/scripts/run_performance_tests.py", line 226, in main
    benchmarks = args.benchmark_names.split(',')
AttributeError: 'Namespace' object has no attribute 'benchmark_names'"""
    }
    swarming_task_result.return_value = {
        'bot_id': 'bot id',
        'exit_code': 1,
        'failure': True,
        'outputs_ref': {
            'isolatedserver': 'output isolate server',
            'isolated': 'output isolate hash',
        },
        'state': 'COMPLETED',
    }
    for attempt in range(10):
      self.assertNotEqual({},
                          task_module.Evaluate(
                              self.job,
                              event_module.Event(
                                  type='update',
                                  target_task='run_test_chromium@aaaaaaa_%s' %
                                  (attempt,),
                                  payload={
                                      'kind': 'pubsub_message',
                                      'action': 'poll'
                                  }), evaluator), 'Attempt #%s' % (attempt,))
    self.assertEqual(
        {
            'run_test_chromium@aaaaaaa_%s' % (attempt,): {
                'status': 'failed',
                'swarming_server': 'some_server',
                'dimensions': DIMENSIONS,
                'errors': mock.ANY,
                'extra_args': [],
                'swarming_request_body': {
                    'name': mock.ANY,
                    'user': mock.ANY,
                    'priority': mock.ANY,
                    'task_slices': mock.ANY,
                    'tags': mock.ANY,
                    'pubsub_auth_token': mock.ANY,
                    'pubsub_topic': mock.ANY,
                    'pubsub_userdata': mock.ANY,
                    'service_account': mock.ANY,
                },
                'swarming_task_result': {
                    'bot_id': mock.ANY,
                    'state': 'COMPLETED',
                    'failure': True,
                },
                'isolate_server': 'output isolate server',
                'isolate_hash': 'output isolate hash',
                'swarming_task_id': 'task id',
                'tries': 1,
                'change': mock.ANY,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='run_test')))

  def testEvaluateHandleFailures_Expired(self, swarming_task_result,
                                         swarming_tasks_new):
    swarming_tasks_new.return_value = {'task_id': 'task id'}
    evaluator = evaluators.SequenceEvaluator(
        evaluators=(
            evaluators.FilteringEvaluator(
                predicate=evaluators.TaskTypeEq('find_isolate'),
                delegate=evaluators.SequenceEvaluator(
                    evaluators=(bisection_test_util.FakeFoundIsolate(self.job),
                                evaluators.TaskPayloadLiftingEvaluator()))),
            run_test.Evaluator(self.job),
        ))
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            evaluator))
    swarming_task_result.return_value = {
        'state': 'EXPIRED',
    }
    for attempt in range(10):
      self.assertNotEqual({},
                          task_module.Evaluate(
                              self.job,
                              event_module.Event(
                                  type='update',
                                  target_task='run_test_chromium@aaaaaaa_%s' %
                                  (attempt,),
                                  payload={
                                      'kind': 'pubsub_message',
                                      'action': 'poll'
                                  }), evaluator), 'Attempt #%s' % (attempt,))

    self.assertEqual(
        {
            'run_test_chromium@aaaaaaa_%s' % (attempt,): {
                'status': 'failed',
                'swarming_server': 'some_server',
                'dimensions': DIMENSIONS,
                'errors': [{
                    'reason': 'SwarmingExpired',
                    'message': mock.ANY
                },],
                'extra_args': [],
                'swarming_request_body': {
                    'name': mock.ANY,
                    'user': mock.ANY,
                    'priority': mock.ANY,
                    'task_slices': mock.ANY,
                    'tags': mock.ANY,
                    'pubsub_auth_token': mock.ANY,
                    'pubsub_topic': mock.ANY,
                    'pubsub_userdata': mock.ANY,
                    'service_account': mock.ANY,
                },
                'swarming_task_result': {
                    'state': 'EXPIRED',
                },
                'swarming_task_id': 'task id',
                'tries': 1,
                'change': mock.ANY,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='run_test')))

  def testEvaluateHandleFailures_Retry(self, *_):
    self.skipTest('Deferring implementation pending design.')


@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
class ValidatorTest(test.TestCase):

  def setUp(self):
    super().setUp()
    self.maxDiff = None

  def testMissingDependency(self):
    job = job_module.Job.New((), ())
    task_module.PopulateTaskGraph(
        job,
        task_module.TaskGraph(
            vertices=[
                task_module.TaskVertex(
                    id='run_test_bbbbbbb_0',
                    vertex_type='run_test',
                    payload={
                        'swarming_server': 'some_server',
                        'dimensions': DIMENSIONS,
                        'extra_args': [],
                    }),
            ],
            edges=[]))
    self.assertEqual(
        {
            'run_test_bbbbbbb_0': {
                'errors': [{
                    'cause': 'DependencyError',
                    'message': mock.ANY
                }]
            }
        },
        task_module.Evaluate(
            job,
            event_module.Event(type='validate', target_task=None, payload={}),
            run_test.Validator()))

  def testMissingDependencyInputs(self):
    job = job_module.Job.New((), ())
    task_module.PopulateTaskGraph(
        job,
        task_module.TaskGraph(
            vertices=[
                task_module.TaskVertex(
                    id='find_isolate_chromium@aaaaaaa',
                    vertex_type='find_isolate',
                    payload={
                        'builder': 'Some Builder',
                        'target': 'telemetry_perf_tests',
                        'bucket': 'luci.bucket',
                        'change': {
                            'commits': [{
                                'repository': 'chromium',
                                'git_hash': 'aaaaaaa',
                            }]
                        }
                    }),
                task_module.TaskVertex(
                    id='run_test_chromium@aaaaaaa_0',
                    vertex_type='run_test',
                    payload={
                        'swarming_server': 'some_server',
                        'dimensions': DIMENSIONS,
                        'extra_args': [],
                    }),
            ],
            edges=[
                task_module.Dependency(
                    from_='run_test_chromium@aaaaaaa_0',
                    to='find_isolate_chromium@aaaaaaa')
            ],
        ))

    # This time we're fine, there should be no errors.
    self.assertEqual({},
                     task_module.Evaluate(
                         job,
                         event_module.Event(
                             type='validate', target_task=None, payload={}),
                         run_test.Validator()))

    # Send an initiate message then catch that we've not provided the required
    # payload in the task when it's ongoing.
    self.assertEqual(
        {
            'find_isolate_chromium@aaaaaaa': mock.ANY,
            'run_test_chromium@aaaaaaa_0': {
                'errors': [{
                    'cause': 'MissingDependencyInputs',
                    'message': mock.ANY
                }]
            }
        },
        task_module.Evaluate(
            job,
            event_module.Event(type='initiate', target_task=None, payload={}),
            evaluators.FilteringEvaluator(
                predicate=evaluators.TaskTypeEq('find_isolate'),
                delegate=evaluators.SequenceEvaluator(
                    evaluators=(
                        functools.partial(
                            bisection_test_util.FakeNotFoundIsolate, job),
                        evaluators.TaskPayloadLiftingEvaluator(),
                    )),
                alternative=run_test.Validator()),
        ))

  def testMissingExecutionOutputs(self):
    self.skipTest('Not implemented yet.')
