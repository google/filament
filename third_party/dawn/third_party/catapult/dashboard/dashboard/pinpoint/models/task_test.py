# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import logging
from unittest import mock
import functools
import unittest

from dashboard.pinpoint.models import task as task_module
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import exploration
from dashboard.pinpoint import test

FakeEvent = collections.namedtuple('Event', ('type', 'status', 'payload'))


def TaskStatusGetter(task_status, task, event, _):
  if event.type == 'test':
    task_status[task.id] = task.status


def UpdateTask(job, task_id, new_state, _):
  logging.debug('Updating task "%s" to "%s"', task_id, new_state)
  task_module.UpdateTask(job, task_id, new_state=new_state)


@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
class PopulateTests(test.TestCase):

  def setUp(self):
    super().setUp()
    self.maxDiff = None

  def testPopulateAndEvaluateAdderGraph(self):
    job = job_module.Job.New((), ())
    task_graph = task_module.TaskGraph(
        vertices=[
            task_module.TaskVertex(
                id='input0', vertex_type='constant', payload={'value': 0}),
            task_module.TaskVertex(
                id='input1', vertex_type='constant', payload={'value': 1}),
            task_module.TaskVertex(
                id='plus', vertex_type='operator+', payload={}),
        ],
        edges=[
            task_module.Dependency(from_='plus', to='input0'),
            task_module.Dependency(from_='plus', to='input1'),
        ],
    )
    task_module.PopulateTaskGraph(job, task_graph)

    def AdderEvaluator(task, _, accumulator):
      if task.task_type == 'constant':
        accumulator[task.id] = task.payload.get('value', 0)
      elif task.task_type == 'operator+':
        inputs = [accumulator.get(dep) for dep in task.dependencies]
        accumulator[task.id] = functools.reduce(lambda a, v: a + v, inputs)

    accumulator = task_module.Evaluate(job, {}, AdderEvaluator)
    self.assertEqual(1, accumulator.get('plus'))

  def testPouplateAndEvaluateGrowingGraph(self):
    with mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
                    mock.MagicMock(return_value=["a"])):
      job = job_module.Job.New((), ())
    task_module.PopulateTaskGraph(
        job,
        task_module.TaskGraph(
            vertices=[
                task_module.TaskVertex(
                    id='rev_0',
                    vertex_type='revision',
                    payload={
                        'revision': '0',
                        'position': 0
                    }),
                task_module.TaskVertex(
                    id='rev_100',
                    vertex_type='revision',
                    payload={
                        'revision': '100',
                        'position': 100
                    }),
                task_module.TaskVertex(
                    id='bisection', vertex_type='bisection', payload={}),
            ],
            edges=[
                task_module.Dependency(from_='bisection', to='rev_0'),
                task_module.Dependency(from_='bisection', to='rev_100'),
            ]))

    def FindMidpoint(a, b):
      offset = (b - a) // 2
      if offset == 0:
        return None
      return a + offset

    def ExplorationEvaluator(task, event, accumulator):
      logging.debug('Evaluating: %s, %s, %s', task, event, accumulator)
      if task.task_type == 'revision':
        accumulator[task.id] = task.payload
        return None

      if task.task_type == 'bisection':
        rev_positions = list(
            sorted(
                accumulator.get(dep).get('position')
                for dep in task.dependencies))
        results = list(rev_positions)
        insertion_list = exploration.Speculate(
            rev_positions,
            # Assume we always find a difference between two positions.
            lambda *_: True,

            # Do nothing when we encounter an unknown error.
            lambda _: None,

            # Provide the function that will find the midpoint between two
            # revisions.
            FindMidpoint,

            # Speculate two levels deep in the bisection space.
            levels=2)
        for index, change in insertion_list:
          results.insert(index, change)

        new_positions = set(results) - set(rev_positions)
        if new_positions:

          def GraphExtender(_):
            logging.debug('New revisions: %s', new_positions)
            task_module.ExtendTaskGraph(job, [
                task_module.TaskVertex(
                    id='rev_%s' % (rev,),
                    vertex_type='revision',
                    payload={
                        'revision': '%s' % (rev,),
                        'position': rev
                    }) for rev in new_positions
            ], [
                task_module.Dependency(from_='bisection', to='rev_%s' % (rev,))
                for rev in new_positions
            ])

          return [GraphExtender]
      return None

    accumulator = task_module.Evaluate(job, None, ExplorationEvaluator)
    self.assertEqual(
        list(sorted(accumulator)),
        sorted(['rev_%s' % (rev,) for rev in range(0, 101)]))

  def testPopulateEvaluateCallCounts(self):
    job = job_module.Job.New((), ())
    task_module.PopulateTaskGraph(
        job,
        task_module.TaskGraph(
            vertices=[
                task_module.TaskVertex(
                    id='leaf_0', vertex_type='node', payload={}),
                task_module.TaskVertex(
                    id='leaf_1', vertex_type='node', payload={}),
                task_module.TaskVertex(
                    id='parent', vertex_type='node', payload={}),
            ],
            edges=[
                task_module.Dependency(from_='parent', to='leaf_0'),
                task_module.Dependency(from_='parent', to='leaf_1'),
            ]))
    calls = {}

    def CallCountEvaluator(task, event, accumulator):
      logging.debug('Evaluate(%s, %s, %s) called.', task.id, event, accumulator)
      calls[task.id] = calls.get(task.id, 0) + 1

    task_module.Evaluate(job, 'test', CallCountEvaluator)
    self.assertDictEqual({
        'leaf_0': 1,
        'leaf_1': 1,
        'parent': 1,
    }, calls)

  def testPopulateEmptyGraph(self):
    job = job_module.Job.New((), ())
    task_graph = task_module.TaskGraph(vertices=[], edges=[])
    task_module.PopulateTaskGraph(job, task_graph)
    evaluator = mock.MagicMock()
    evaluator.assert_not_called()
    task_module.Evaluate(job, 'test', evaluator)

  def testPopulateCycles(self):
    job = job_module.Job.New((), ())
    task_graph = task_module.TaskGraph(
        vertices=[
            task_module.TaskVertex(
                id='node_0', vertex_type='process', payload={}),
            task_module.TaskVertex(
                id='node_1', vertex_type='process', payload={})
        ],
        edges=[
            task_module.Dependency(from_='node_0', to='node_1'),
            task_module.Dependency(from_='node_1', to='node_0')
        ])
    task_module.PopulateTaskGraph(job, task_graph)
    calls = {}

    def CycleEvaluator(task, event, accumulator):
      logging.debug('Evaluate(%s, %s, %s) called.', task.id, event, accumulator)
      calls[task.id] = calls.get(task.id, 0) + 1

    task_module.Evaluate(job, 'test', CycleEvaluator)
    self.assertDictEqual({'node_0': 1, 'node_1': 1}, calls)

  def testPopulateIslands(self):
    self.skipTest('Not implemented yet')


def TransitionEvaluator(job, task, event, accumulator):
  accumulator[task.id] = task.status
  if task.id != event.get('target'):
    if task.dependencies and any(
        accumulator.get(dep) == 'ongoing'
        for dep in task.dependencies) and task.status != 'ongoing':
      return [functools.partial(UpdateTask, job, task.id, 'ongoing')]
    if len(task.dependencies) and all(
        accumulator.get(dep) == 'completed'
        for dep in task.dependencies) and task.status != 'completed':
      return [functools.partial(UpdateTask, job, task.id, 'completed')]
    return None

  if task.status == event.get('current_state'):
    return [functools.partial(UpdateTask, job, task.id, event.get('new_state'))]
  return None


class EvaluateTest(test.TestCase):

  def setUp(self):
    super().setUp()
    self.maxDiff = None
    with mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
                    mock.MagicMock(return_value=["a"])):
      self.job = job_module.Job.New((), ())
    task_module.PopulateTaskGraph(
        self.job,
        task_module.TaskGraph(
            vertices=[
                task_module.TaskVertex(
                    id='task_0', vertex_type='task', payload={}),
                task_module.TaskVertex(
                    id='task_1', vertex_type='task', payload={}),
                task_module.TaskVertex(
                    id='task_2', vertex_type='task', payload={}),
            ],
            edges=[
                task_module.Dependency(from_='task_2', to='task_0'),
                task_module.Dependency(from_='task_2', to='task_1'),
            ]))

  def testEvaluateStateTransitionProgressions(self):
    self.assertDictEqual(
        {
            'task_0': 'ongoing',
            'task_1': 'pending',
            'task_2': 'ongoing'
        },
        task_module.Evaluate(self.job, {
            'target': 'task_0',
            'current_state': 'pending',
            'new_state': 'ongoing'
        }, functools.partial(TransitionEvaluator, self.job)))
    self.assertDictEqual(
        {
            'task_0': 'ongoing',
            'task_1': 'ongoing',
            'task_2': 'ongoing'
        },
        task_module.Evaluate(self.job, {
            'target': 'task_1',
            'current_state': 'pending',
            'new_state': 'ongoing'
        }, functools.partial(TransitionEvaluator, self.job)))
    self.assertDictEqual(
        {
            'task_0': 'completed',
            'task_1': 'ongoing',
            'task_2': 'ongoing'
        },
        task_module.Evaluate(
            self.job, {
                'target': 'task_0',
                'current_state': 'ongoing',
                'new_state': 'completed'
            }, functools.partial(TransitionEvaluator, self.job)))
    self.assertDictEqual(
        {
            'task_0': 'completed',
            'task_1': 'completed',
            'task_2': 'completed'
        },
        task_module.Evaluate(
            self.job, {
                'target': 'task_1',
                'current_state': 'ongoing',
                'new_state': 'completed'
            }, functools.partial(TransitionEvaluator, self.job)))

  def testEvaluateInvalidTransition(self):
    with self.assertRaises(task_module.InvalidTransition):
      self.assertDictEqual(
          {
              'task_0': 'failed',
              'task_1': 'pending',
              'task_2': 'pending',
          },
          task_module.Evaluate(self.job, {
              'target': 'task_0',
              'current_state': 'pending',
              'new_state': 'failed'
          }, functools.partial(TransitionEvaluator, self.job)))
      task_module.Evaluate(self.job, {
          'target': 'task_0',
          'current_state': 'failed',
          'new_state': 'ongoing'
      }, functools.partial(TransitionEvaluator, self.job))

  def testEvaluateInvalidAmendment_ExistingTask(self):
    with self.assertRaises(task_module.InvalidAmendment):

      def AddExistingTaskEvaluator(task, event, _):
        if event.get('target') == task.id:

          def GraphExtender(_):
            task_module.ExtendTaskGraph(
                self.job,
                vertices=[
                    task_module.TaskVertex(
                        id=task.id, vertex_type='duplicate', payload={})
                ],
                dependencies=[
                    task_module.Dependency(from_='task_2', to=task.id)
                ])

          return [GraphExtender]
        return None

      task_module.Evaluate(self.job, {'target': 'task_0'},
                           AddExistingTaskEvaluator)

  def testEvaluateInvalidAmendment_BrokenDependency(self):
    with self.assertRaises(task_module.InvalidAmendment):

      def AddExistingTaskEvaluator(task, event, _):
        if event.get('target') == task.id:

          def GraphExtender(_):
            task_module.ExtendTaskGraph(
                self.job,
                vertices=[],
                dependencies=[
                    task_module.Dependency(from_='unknown', to=task.id)
                ])

          return [GraphExtender]
        return None

      task_module.Evaluate(self.job, {'target': 'task_0'},
                           AddExistingTaskEvaluator)

  def testEvaluateConverges(self):
    self.skipTest('Not implemented yet')


class DecoratorTest(unittest.TestCase):

  @classmethod
  @task_module.LogStateTransitionFailures
  def ThrowsInvalidTransition(cls, *_):
    raise task_module.InvalidTransition('This must not escape. %s' % (cls,))

  @classmethod
  @task_module.LogStateTransitionFailures
  def ThrowsSomethingElse(cls, *_):
    raise ValueError('This is expected. %s' % (cls,))

  def testHandlesLogTransitionFailures(self):
    try:
      DecoratorTest.ThrowsInvalidTransition(None)
    except task_module.InvalidTransition:
      self.fail('Invalid transition failure not captured by decorator!')

  def testLeavesNonTransitionFailuresAlone(self):
    with self.assertRaises(ValueError):
      DecoratorTest.ThrowsSomethingElse(None)
