# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import json
from unittest import mock
import unittest
import logging

from dashboard.pinpoint.models.change import change
from dashboard.pinpoint.models import errors
from dashboard.pinpoint.models.quest import run_test

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
_BASE_ARGUMENTS = {
    'swarming_server': 'server',
    'dimensions': DIMENSIONS,
}

_BASE_SWARMING_TAGS = {}

FakeJob = collections.namedtuple(
    'Job', ['job_id', 'url', 'comparison_mode', 'user', 'state', 'bots',
            'batch_id'])
State = collections.namedtuple('State', ['attempt_count'])


class StartTest(unittest.TestCase):

  def testStart(self):
    quest = run_test.RunTest('server', DIMENSIONS, ['arg'], _BASE_SWARMING_TAGS,
                             None, None)
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'performance',
                'user@example.com', State(1), ['a'], 'some_batch_id'))
    execution = quest.Start('change', 'https://isolate.server', 'isolate hash')
    self.assertEqual(execution._extra_args, ['arg'])


@mock.patch('random.shuffle')
class ABOrderingsTest(unittest.TestCase):

  def testGetABOrderingsEven(self, random_shuffle):
    quest = run_test.RunTest.FromDict(_BASE_ARGUMENTS)
    orderings = quest._GetABOrderings(4)
    random_shuffle.assert_called()
    self.assertTrue(1 in orderings)
    self.assertTrue(0 in orderings)
    self.assertEqual(len(orderings), 4)

  def testGetABOrderingsOdd(self, random_shuffle):
    quest = run_test.RunTest.FromDict(_BASE_ARGUMENTS)
    orderings = quest._GetABOrderings(5)
    random_shuffle.assert_called()
    self.assertTrue(1 in orderings)
    self.assertTrue(0 in orderings)
    self.assertEqual(len(orderings), 5)


class FromDictTest(unittest.TestCase):

  def testMinimumArguments(self):
    quest = run_test.RunTest.FromDict(_BASE_ARGUMENTS)
    expected = run_test.RunTest('server', DIMENSIONS, [], _BASE_SWARMING_TAGS,
                                None, None)
    self.assertEqual(quest, expected)

  def testAllArguments(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['extra_test_args'] = '["--custom-arg", "custom value"]'
    quest = run_test.RunTest.FromDict(arguments)

    extra_args = ['--custom-arg', 'custom value']
    expected = run_test.RunTest('server', DIMENSIONS, extra_args,
                                _BASE_SWARMING_TAGS, None, None)
    self.assertEqual(quest, expected)

  def testMissingSwarmingServer(self):
    arguments = dict(_BASE_ARGUMENTS)
    del arguments['swarming_server']
    with self.assertRaises(TypeError):
      run_test.RunTest.FromDict(arguments)

  def testMissingDimensions(self):
    arguments = dict(_BASE_ARGUMENTS)
    del arguments['dimensions']
    with self.assertRaises(TypeError):
      run_test.RunTest.FromDict(arguments)

  def testStringDimensions(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['dimensions'] = json.dumps(DIMENSIONS)
    quest = run_test.RunTest.FromDict(arguments)
    expected = run_test.RunTest('server', DIMENSIONS, [], _BASE_SWARMING_TAGS,
                                None, None)
    self.assertEqual(quest, expected)

  def testInvalidExtraTestArgs(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['extra_test_args'] = '"this is a json-encoded string"'
    with self.assertRaises(TypeError):
      run_test.RunTest.FromDict(arguments)

  def testStringExtraTestArgs(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['extra_test_args'] = '--custom-arg "custom value"'
    quest = run_test.RunTest.FromDict(arguments)

    extra_args = ['--custom-arg', 'custom value']
    expected = run_test.RunTest('server', DIMENSIONS, extra_args,
                                _BASE_SWARMING_TAGS, None, None)
    self.assertEqual(quest, expected)

class _RunTestExecutionTest(unittest.TestCase):

  def GetNewTaskBase(self):
    return {
        'realm':
            'chrome:pinpoint',
        'name':
            'Pinpoint job',
        'user':
            'Pinpoint',
        'tags':
            mock.ANY,
        'priority':
            '100',
        'serviceAccount':
            mock.ANY,
        'taskSlices': [{
            'expirationSecs': '86400',
            'properties': {
                'inputsRef': {
                    'isolatedserver': 'isolate server',
                    'isolated': 'input isolate hash',
                },
                'extraArgs': ['arg'],
                'dimensions': DIMENSIONS,
                'executionTimeoutSecs': mock.ANY,
                'ioTimeoutSecs': mock.ANY,
            }
        },],
    }

  def assertNewTaskHasDimensionsMulti(self, swarming_tasks_new, patches):
    tasks = []
    for p in patches:
      task = self.GetNewTaskBase()
      task.update(p)
      tasks.append(mock.call(task))
    print(str(tasks))
    swarming_tasks_new.assert_has_calls(tasks)

  def assertNewTaskHasDimensions(self, swarming_tasks_new, patch=None):
    body = self.GetNewTaskBase()
    if patch:
      body.update(patch)
    swarming_tasks_new.assert_called_with(body)

@mock.patch('dashboard.services.swarming.Tasks.New')
@mock.patch('dashboard.services.swarming.Task.Result')
class RunTestFullTest(_RunTestExecutionTest):

  @mock.patch('dashboard.services.swarming.IsBotAlive',
              mock.MagicMock(return_value=True))
  @mock.patch('dashboard.services.swarming.Tasks.Count',
              mock.MagicMock(return_value={'count': 0}))
  @mock.patch(
      'dashboard.common.cloud_metric.PublishSwarmingBotPendingTasksMetric',
      mock.MagicMock())
  def testSuccess(self, swarming_task_result, swarming_tasks_new):
    # Goes through a full run of two Executions.

    # Call RunTest.Start() to create an Execution.
    quest = run_test.RunTest('server', DIMENSIONS, ['arg'], _BASE_SWARMING_TAGS,
                             None, None)

    # Propagate a thing that looks like a job.
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'try',
                'user@example.com', State(1), ['a'], 'some_batch_id'))

    execution = quest.Start(
        change.Change("a", variant=0), 'isolate server', 'input isolate hash')
    quest.Start(
        change.Change("a", variant=1), 'isolate server', 'input isolate hash')

    swarming_task_result.assert_not_called()
    swarming_tasks_new.assert_not_called()

    # Call the first Poll() to start the swarming task.
    swarming_tasks_new.return_value = {'taskId': 'task id'}
    execution.Poll()

    swarming_task_result.assert_not_called()
    self.assertEqual(swarming_tasks_new.call_count, 2)
    self.assertNewTaskHasDimensions(
        swarming_tasks_new, {
            'taskSlices': [{
                'expirationSecs': '86400',
                'properties': {
                    'inputsRef': {
                        'isolatedserver': 'isolate server',
                        'isolated': 'input isolate hash'
                    },
                    'extraArgs': ['arg'],
                    'dimensions': DIMENSIONS + [{
                        "key": "id",
                        "value": "a"
                    }],
                    'executionTimeoutSecs': mock.ANY,
                    'ioTimeoutSecs': mock.ANY,
                }
            }]
        })
    self.assertFalse(execution.completed)
    self.assertFalse(execution.failed)

    # Call subsequent Poll()s to check the task status.
    swarming_task_result.return_value = {'state': 'PENDING'}
    execution.Poll()

    self.assertFalse(execution.completed)
    self.assertFalse(execution.failed)

    swarming_task_result.return_value = {
        'botId': 'a',
        'exitCode': 0,
        'failure': False,
        'outputsRef': {
            'isolatedserver': 'output isolate server',
            'isolated': 'output isolate hash',
        },
        'state': 'COMPLETED',
    }
    execution.Poll()

    self.assertTrue(execution.completed)
    self.assertFalse(execution.failed)
    self.assertEqual(execution.result_values, ())
    self.assertEqual(
        execution.result_arguments, {
            'isolate_server': 'output isolate server',
            'isolate_hash': 'output isolate hash',
        })
    self.assertEqual(
        execution.AsDict(), {
            'completed':
                True,
            'exception':
                None,
            'details': [
                {
                    'key': 'bot',
                    'value': 'a',
                    'url': 'server/bot?id=a',
                },
                {
                    'key': 'task',
                    'value': 'task id',
                    'url': 'server/task?id=task id',
                },
                {
                    'key': 'isolate',
                    'value': 'output isolate hash',
                    'url': 'output isolate server/browse?'
                           'digest=output isolate hash',
                },
            ],
        })

  @mock.patch('dashboard.pinpoint.models.quest.run_test.RunTest._GetABOrderings'
             )
  def testBotAllocationAndScheduling(self, get_ab_orderings,
                                     swarming_task_result, swarming_tasks_new):
    # List of lists because side_effects returns one list element per call
    get_ab_orderings.side_effect = [[1, 0]]
    # Goes through a full run of two Executions.

    # Call RunTest.Start() to create an Execution.
    quest = run_test.RunTest('server', DIMENSIONS, ['arg'], _BASE_SWARMING_TAGS,
                             None, None)

    # Propagate a thing that looks like a job.
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'try',
                'user@example.com', State(2), ['b', 'a'], 'some_batch_id'))

    change_a = change.Change("a", variant=0)
    change_b = change.Change("b", variant=1)
    execution = quest.Start(change_a, 'cas-instance', 'hasha/111')
    quest.Start(change_a, 'cas-instance', 'hasha/111')
    quest.Start(change_b, 'cas-instance', 'hashb/111')
    quest.Start(change_b, 'cas-instance', 'hashb/111')

    swarming_task_result.assert_not_called()

    # Call the first Poll() to start the swarming task.
    swarming_tasks_new.return_value = {'taskId': 'task id'}
    execution.Poll()

    swarming_task_result.assert_not_called()
    self.assertEqual(swarming_tasks_new.call_count, 4)

    def GetPatch(iso_hash, bot):
      return {
          'taskSlices': [{
              'expirationSecs': '86400',
              'properties': {
                  'casInputRoot': {
                      'casInstance': 'cas-instance',
                      'digest': {
                          'hash': iso_hash,
                          'sizeBytes': 111,
                      },
                  },
                  'extraArgs': ['arg'],
                  'dimensions': DIMENSIONS + [{
                      "key": "id",
                      "value": bot
                  }],
                  'executionTimeoutSecs': mock.ANY,
                  'ioTimeoutSecs': mock.ANY,
              }
          }]
      }

    self.assertNewTaskHasDimensionsMulti(swarming_tasks_new, [
        GetPatch("hasha", "b"),
        GetPatch("hashb", "b"),
        GetPatch("hashb", "a"),
        GetPatch("hasha", "a")
    ])

  @mock.patch('dashboard.pinpoint.models.quest.run_test.RunTest._GetABOrderings'
             )
  def testBotAllocationAndSchedulingNotAllSpawned(self, get_ab_orderings,
                                                  swarming_task_result,
                                                  swarming_tasks_new):
    # List of lists because side_effects returns one list element per call
    get_ab_orderings.side_effect = [[1, 0]]
    # Goes through a full run of two Executions.

    # Call RunTest.Start() to create an Execution.
    quest = run_test.RunTest('server', DIMENSIONS, ['arg'], _BASE_SWARMING_TAGS,
                             None, None)

    # Propagate a thing that looks like a job.
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'try',
                'user@example.com', State(2), ['b', 'a'], 'some_batch_id'))

    change_a = change.Change("a", variant=0)
    change_b = change.Change("b", variant=1)
    execution = quest.Start(change_a, 'cas-instance', 'hasha/111')
    quest.Start(change_a, 'cas-instance', 'hasha/111')
    quest.Start(change_b, 'cas-instance', 'hashb/111')

    swarming_task_result.assert_not_called()

    # Call the first Poll() to start the swarming task.
    swarming_tasks_new.return_value = {'taskId': 'task id'}
    execution.Poll()

    swarming_task_result.assert_not_called()
    self.assertEqual(swarming_tasks_new.call_count, 0)


  @mock.patch('dashboard.services.swarming.IsBotAlive',
              mock.MagicMock(return_value=True))
  @mock.patch('dashboard.services.swarming.Tasks.Count',
              mock.MagicMock(return_value={'count': 0}))
  @mock.patch(
      'dashboard.common.cloud_metric.PublishSwarmingBotPendingTasksMetric',
      mock.MagicMock())
  def testSuccess_Cas(self, swarming_task_result, swarming_tasks_new):
    # Goes through a full run of two Executions.

    # Call RunTest.Start() to create an Execution.
    quest = run_test.RunTest('server', DIMENSIONS, ['arg'], _BASE_SWARMING_TAGS,
                             None, None)

    # Propagate a thing that looks like a job.
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'try',
                'user@example.com', State(1), ['a'], 'some_batch_id'))

    execution_a = quest.Start(
        change.Change("a", variant=0), 'cas-instance', 'xxxxxxxx/111')
    execution_b = quest.Start(
        change.Change("b", variant=1), 'cas-instance', 'xxxxxxxx/111')

    swarming_task_result.assert_not_called()
    swarming_tasks_new.assert_not_called()

    # Call the first Poll() to start the swarming task.
    swarming_tasks_new.return_value = {'taskId': 'task id'}
    execution_b.Poll()
    execution_a.Poll()

    swarming_task_result.assert_not_called()
    self.assertEqual(swarming_tasks_new.call_count, 2)
    self.assertNewTaskHasDimensions(
        swarming_tasks_new, {
            'taskSlices': [{
                'expirationSecs': '86400',
                'properties': {
                    'casInputRoot': {
                        'casInstance': 'cas-instance',
                        'digest': {
                            'hash': 'xxxxxxxx',
                            'sizeBytes': 111,
                        },
                    },
                    'extraArgs': ['arg'],
                    'dimensions': DIMENSIONS + [{
                        "key": "id",
                        "value": "a"
                    }],
                    'executionTimeoutSecs': mock.ANY,
                    'ioTimeoutSecs': mock.ANY,
                }
            }]
        })
    self.assertFalse(execution_b.completed)
    self.assertFalse(execution_b.failed)

    # Call subsequent Poll()s to check the task status.
    swarming_task_result.return_value = {'state': 'PENDING'}
    execution_b.Poll()

    self.assertFalse(execution_b.completed)
    self.assertFalse(execution_b.failed)

    swarming_task_result.return_value = {
        'botId': 'a',
        'exitCode': 0,
        'failure': False,
        'casOutputRoot': {
            'casInstance': 'projects/x/instances/default_instance',
            'digest': {
                'hash': 'e3b0c44298fc1c149afbf4c8996fb',
                'sizeBytes': 1,
            },
        },
        'state': 'COMPLETED',
    }
    execution_b.Poll()

    self.assertTrue(execution_b.completed)
    self.assertFalse(execution_b.failed)
    self.assertEqual(execution_b.result_values, ())
    self.assertEqual(
        execution_b.result_arguments, {
            'cas_root_ref': {
                'casInstance': 'projects/x/instances/default_instance',
                'digest': {
                    'hash': 'e3b0c44298fc1c149afbf4c8996fb',
                    'sizeBytes': 1,
                },
            }
        })
    self.assertEqual(
        execution_b.AsDict(), {
            'completed':
                True,
            'exception':
                None,
            'details': [
                {
                    'key': 'bot',
                    'value': 'a',
                    'url': 'server/bot?id=a',
                },
                {
                    'key': 'task',
                    'value': 'task id',
                    'url': 'server/task?id=task id',
                },
                {
                    'key': 'isolate',
                    'value': 'e3b0c44298fc1c149afbf4c8996fb/1',
                    'url': 'https://cas-viewer.appspot.com/'
                           'projects/x/instances/default_instance/blobs/'
                           'e3b0c44298fc1c149afbf4c8996fb/1/tree',
                },
            ],
        })

  @mock.patch('dashboard.services.swarming.IsBotAlive',
              mock.MagicMock(return_value=False))
  def testDeadBot(self, swarming_task_result, swarming_tasks_new):
    quest = run_test.RunTest('server', DIMENSIONS, ['arg'], _BASE_SWARMING_TAGS,
                             None, None)

    # Propagate a thing that looks like a job.
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'try',
                'user@example.com', State(1), ['a'], 'some_batch_id'))

    execution_a = quest.Start(
        change.Change("a", variant=0), 'cas-instance', 'xxxxxxxx/111')
    execution_b = quest.Start(
        change.Change("b", variant=1), 'cas-instance', 'xxxxxxxx/111')

    swarming_task_result.assert_not_called()
    swarming_tasks_new.assert_not_called()

    # Call the first Poll() to start the swarming task.
    swarming_tasks_new.return_value = {'taskId': 'task id'}
    execution_b.Poll()
    execution_a.Poll()

    swarming_task_result.assert_not_called()
    self.assertEqual(swarming_tasks_new.call_count, 2)
    self.assertNewTaskHasDimensions(
        swarming_tasks_new, {
            'taskSlices': [{
                'expirationSecs': '86400',
                'properties': {
                    'casInputRoot': {
                        'casInstance': 'cas-instance',
                        'digest': {
                            'hash': 'xxxxxxxx',
                            'sizeBytes': 111,
                        },
                    },
                    'extraArgs': ['arg'],
                    'dimensions': DIMENSIONS + [{
                        "key": "id",
                        "value": "a"
                    }],
                    'executionTimeoutSecs': mock.ANY,
                    'ioTimeoutSecs': mock.ANY,
                }
            }]
        })
    self.assertFalse(execution_b.completed)
    self.assertFalse(execution_b.failed)

    swarming_task_result.return_value = {
        'botId': 'a',
        'casOutputRoot': {
            'casInstance': 'projects/x/instances/default_instance',
            'digest': {
                'hash': 'e3b0c44298fc1c149afbf4c8996fb',
                'sizeBytes': 1,
            },
        },
        'state': 'PENDING',
    }
    execution_b.Poll()

    self.assertTrue(execution_b.completed)
    self.assertTrue(execution_b.failed)

  def testStart_NoSwarmingTags(self, swarming_task_result, swarming_tasks_new):
    del swarming_task_result
    del swarming_tasks_new

    quest = run_test.RunTest('server', DIMENSIONS, ['arg'], None, None, None)
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'performance',
                'user@example.com', State(1), ['a'] , 'some_batch_id'))
    quest.Start('change_1', 'isolate server', 'input isolate hash')


@mock.patch('dashboard.services.swarming.IsBotAlive',
            mock.MagicMock(return_value=True))
@mock.patch('dashboard.services.swarming.Tasks.New')
@mock.patch('dashboard.services.swarming.Task.Result')
class SwarmingTaskStatusTest(_RunTestExecutionTest):

  def testSwarmingError(self, swarming_task_result, swarming_tasks_new):
    swarming_task_result.return_value = {'state': 'BOT_DIED'}
    swarming_tasks_new.return_value = {'taskId': 'task id'}

    quest = run_test.RunTest('server', DIMENSIONS, ['arg'], _BASE_SWARMING_TAGS,
                             None, None)
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'try',
                'user@example.com', State(1), ['a'], 'some_batch_id'))
    execution = quest.Start(
        change.Change("a", variant=0), 'isolate server', 'input isolate hash')
    quest.Start(
        change.Change("a", variant=1), 'isolate server', 'input isolate hash')
    execution.Poll()
    execution.Poll()

    self.assertTrue(execution.completed)
    self.assertTrue(execution.failed)
    last_exception_line = execution.exception['traceback'].splitlines()[-1]
    expected_exception = 'dashboard.pinpoint.models.errors.SwarmingTaskError'
    self.assertTrue(last_exception_line.startswith(expected_exception))

  @mock.patch('dashboard.services.swarming.IsBotAlive',
              mock.MagicMock(return_value=True))
  @mock.patch('dashboard.services.swarming.Task.Stdout')
  def testTestError(self, swarming_task_stdout, swarming_task_result,
                    swarming_tasks_new):
    swarming_task_stdout.return_value = {'output': ''}
    swarming_task_result.return_value = {
        'botId': 'a',
        'exitCode': 1,
        'failure': True,
        'state': 'COMPLETED',
        'outputsRef': {
            'isolatedserver': 'https://server',
            'isolated': 'deadc0de',
        },
    }
    swarming_tasks_new.return_value = {'taskId': 'task id'}

    quest = run_test.RunTest('server', DIMENSIONS, ['arg'], _BASE_SWARMING_TAGS,
                             None, None)
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'try',
                'user@example.com', State(1), ['a'], 'some_batch_id'))
    execution = quest.Start(
        change.Change("a", variant=0), 'isolate server', 'input isolate hash')
    quest.Start(
        change.Change("a", variant=1), 'isolate server', 'input isolate hash')
    execution.Poll()
    execution.Poll()

    self.assertTrue(execution.completed)
    self.assertTrue(execution.failed)
    logging.debug('Exception: %s', execution.exception)
    self.assertIn('https://server/browse?digest=deadc0de',
                  execution.exception['traceback'])


@mock.patch('dashboard.services.swarming.Tasks.New')
@mock.patch('dashboard.services.swarming.Task.Result')
class BotIdHandlingTest(_RunTestExecutionTest):

  @mock.patch('dashboard.services.swarming.IsBotAlive',
              mock.MagicMock(return_value=True))
  def testExecutionExpired(self, swarming_task_result, swarming_tasks_new):
    # If the Swarming task expires, the bots are overloaded or the dimensions
    # don't correspond to any bot. Raise an error that's fatal to the Job.
    swarming_tasks_new.return_value = {'taskId': 'task id'}
    swarming_task_result.return_value = {'state': 'EXPIRED'}

    quest = run_test.RunTest('server', DIMENSIONS, ['arg'], _BASE_SWARMING_TAGS,
                             None, None)
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'try',
                'user@example.com', State(1), ['a'], 'some_batch_id'))
    execution_a = quest.Start(
        change.Change("a", variant=0), 'isolate server', 'input isolate hash')
    quest.Start(
        change.Change("a", variant=1), 'isolate server', 'input isolate hash')
    execution_a.Poll()
    with self.assertRaises(errors.SwarmingExpired):
      execution_a.Poll()
    self.assertEqual(execution_a._bot_id, 'a')

  def testBisectsDontUsePairing(self, swarming_task_result, swarming_tasks_new):
    swarming_tasks_new.return_value = {'taskId': 'task id'}
    swarming_task_result.return_value = {'state': 'CANCELED'}

    quest = run_test.RunTest('server', DIMENSIONS, ['arg'], _BASE_SWARMING_TAGS,
                             None, None)
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'performance',
                'user@example.com', State(1), ['a'], 'some_batch_id'))
    quest.Start('change_1', 'isolate server', 'input isolate hash')

  @mock.patch('dashboard.services.swarming.IsBotAlive',
              mock.MagicMock(return_value=True))
  @mock.patch('dashboard.services.swarming.Tasks.Count',
              mock.MagicMock(return_value={'count': 0}))
  @mock.patch(
      'dashboard.common.cloud_metric.PublishSwarmingBotPendingTasksMetric',
      mock.MagicMock())
  def testSimultaneousExecutions(self, swarming_task_result,
                                 swarming_tasks_new):
    quest = run_test.RunTest('server', DIMENSIONS, ['arg'], _BASE_SWARMING_TAGS,
                             None, None)
    quest.PropagateJob(
        FakeJob('cafef00d', 'https://pinpoint/cafef00d', 'try',
                'user@example.com', State(1), ['a'], 'some_batch_id'))
    execution_1 = quest.Start(
        change.Change("a", variant=0), 'input isolate server',
        'input isolate hash')
    execution_2 = quest.Start(
        change.Change("a", variant=1), 'input isolate server',
        'input isolate hash')

    swarming_tasks_new.return_value = {'taskId': 'task id'}
    swarming_task_result.return_value = {'state': 'PENDING'}
    execution_1.Poll()
    execution_2.Poll()

    self.assertEqual(swarming_tasks_new.call_count, 2)

    swarming_task_result.return_value = {
        'botId': 'a',
        'exitCode': 0,
        'failure': False,
        'outputsRef': {
            'isolatedserver': 'output isolate server',
            'isolated': 'output isolate hash',
        },
        'state': 'COMPLETED',
    }
    execution_1.Poll()
    execution_2.Poll()

    self.assertEqual(swarming_tasks_new.call_count, 2)
