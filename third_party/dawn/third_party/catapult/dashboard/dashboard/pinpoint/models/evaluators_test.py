# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Test coverage for the Evaluators module."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from unittest import mock
import unittest

from dashboard.pinpoint.models import evaluators
from dashboard.pinpoint.models import event as event_module
from dashboard.pinpoint.models import task as task_module


class EvaluatorsTest(unittest.TestCase):

  def testPayloadLiftingEvaluator_Default(self):
    task = task_module.InMemoryTask(
        id='test_id',
        task_type='test',
        payload={
            'key0': 'value0',
            'key1': 'value1'
        },
        status='pending',
        dependencies=[])
    evaluator = evaluators.TaskPayloadLiftingEvaluator()
    event = event_module.Event(type='test', target_task=None, payload={})
    accumulator = {}
    evaluator(task, event, accumulator)
    self.assertEqual(
        {'test_id': {
            'key0': 'value0',
            'key1': 'value1',
            'status': 'pending'
        }}, accumulator)

  def testPayloadLiftingEvaluator_ExcludeKeys(self):
    task = task_module.InMemoryTask(
        id='test_id',
        task_type='test',
        payload={
            'key_included': 'value0',
            'key_excluded': 'value1'
        },
        status='pending',
        dependencies=[])
    evaluator = evaluators.TaskPayloadLiftingEvaluator(
        exclude_keys={'key_excluded'})
    event = event_module.Event(type='test', target_task=None, payload={})
    accumulator = {}
    evaluator(task, event, accumulator)
    self.assertEqual(
        {'test_id': {
            'key_included': 'value0',
            'status': 'pending'
        }}, accumulator)

  def testPayloadLiftingEvaluator_ExcludeEventTypes(self):
    task = task_module.InMemoryTask(
        id='test_id',
        task_type='test',
        payload={
            'key_must_not_show': 'value0',
        },
        status='pending',
        dependencies=[])
    evaluator = evaluators.TaskPayloadLiftingEvaluator(
        exclude_event_types={'test'})
    event = event_module.Event(type='test', target_task=None, payload={})
    accumulator = {}
    self.assertEqual(None, evaluator(task, event, accumulator))
    self.assertEqual({}, accumulator)

  def testSequenceEvaluator(self):

    def FirstEvaluator(*args):
      args[2].update({'value': 1})
      return ['First Action']

    def SecondEvaluator(*args):
      args[2].update({'value': accumulator.get('value') + 1})
      return ['Second Action']

    task = task_module.InMemoryTask(
        id='test_id',
        task_type='test',
        payload={},
        status='pending',
        dependencies=[])
    evaluator = evaluators.SequenceEvaluator(
        evaluators=(FirstEvaluator, SecondEvaluator))
    event = event_module.Event(type='test', target_task=None, payload={})
    accumulator = {}
    # Test that we're collecting the actions returned by the nested evaluators.
    self.assertEqual(['First Action', 'Second Action'],
                     evaluator(task, event, accumulator))

    # Test that the operations happened in sequence.
    self.assertEqual({'value': 2}, accumulator)

  def testFilteringEvaluator_Matches(self):

    def ThrowingEvaluator(*_):
      raise ValueError('Expect this exception.')

    task = task_module.InMemoryTask(
        id='test_id',
        task_type='test',
        payload={},
        status='pending',
        dependencies=[])
    evaluator = evaluators.FilteringEvaluator(
        predicate=lambda *_: True, delegate=ThrowingEvaluator)
    event = event_module.Event(type='test', target_task=None, payload={})
    accumulator = {}
    with self.assertRaises(ValueError):
      evaluator(task, event, accumulator)

  def testFilteringEvaluator_DoesNotMatch(self):

    def ThrowingEvaluator(*_):
      raise ValueError('This must never be raised.')

    task = task_module.InMemoryTask(
        id='test_id',
        task_type='test',
        payload={},
        status='pending',
        dependencies=[])
    evaluator = evaluators.FilteringEvaluator(
        predicate=lambda *_: False, delegate=ThrowingEvaluator)
    event = event_module.Event(type='test', target_task=None, payload={})
    accumulator = {}
    evaluator(task, event, accumulator)

  def testFilteringEvaluator_DoesNotMatchHasAlternative(self):

    def ThrowingEvaluator(*_):
      raise ValueError('This must never be raised.')

    def AlternativeEvaluator(*_):
      return ['Alternative']

    task = task_module.InMemoryTask(
        id='test_id',
        task_type='test',
        payload={},
        status='pending',
        dependencies=[])
    evaluator = evaluators.FilteringEvaluator(
        predicate=lambda *_: False,
        delegate=ThrowingEvaluator,
        alternative=AlternativeEvaluator)
    event = event_module.Event(type='test', target_task=None, payload={})
    accumulator = {}
    self.assertEqual(['Alternative'], evaluator(task, event, accumulator))

  def testDispatchEvaluator_Matches(self):

    def InitiateEvaluator(*_):
      return [0]

    def UpdateEvaluator(*_):
      return [1]

    task = task_module.InMemoryTask(
        id='test_id',
        task_type='test',
        payload={},
        status='pending',
        dependencies=[])
    evaluator = evaluators.DispatchByEventTypeEvaluator(evaluator_map={
        'initiate': InitiateEvaluator,
        'update': UpdateEvaluator,
    })
    accumulator = {}
    self.assertEqual([0],
                     evaluator(
                         task,
                         event_module.Event(
                             type='initiate', target_task=None, payload={}),
                         accumulator))
    self.assertEqual([1],
                     evaluator(
                         task,
                         event_module.Event(
                             type='update', target_task=None, payload={}),
                         accumulator))

  def testDispatchEvaluator_Default(self):

    def MustNeverCall(*_):
      self.fail('Dispatch failure!')

    def DefaultEvaluator(*_):
      return [0]

    task = task_module.InMemoryTask(
        id='test_id',
        task_type='test',
        payload={},
        status='pending',
        dependencies=[])
    evaluator = evaluators.DispatchByEventTypeEvaluator(
        evaluator_map={
            'match_nothing': MustNeverCall,
        },
        default_evaluator=DefaultEvaluator)
    accumulator = {}
    self.assertEqual([0],
                     evaluator(
                         task,
                         event_module.Event(
                             type='unrecognised', target_task=None, payload={}),
                         accumulator))

  def testSelector_TaskType(self):
    task = task_module.InMemoryTask(
        id='test_id',
        task_type='test',
        payload={},
        status='pending',
        dependencies=[])
    accumulator = {}
    evaluators.Selector(task_type='test')(task,
                                          event_module.Event(
                                              type='undefined',
                                              target_task=None,
                                              payload={}), accumulator)
    self.assertEqual({'test_id': mock.ANY}, accumulator)

  def testSelector_EventType(self):
    task = task_module.InMemoryTask(
        id='test_id',
        task_type='test',
        payload={},
        status='pending',
        dependencies=[])
    accumulator = {}
    evaluators.Selector(event_type='select')(task,
                                             event_module.Event(
                                                 type='unmatched',
                                                 target_task=None,
                                                 payload={}), accumulator)
    self.assertEqual({}, accumulator)
    evaluators.Selector(event_type='select')(task,
                                             event_module.Event(
                                                 type='select',
                                                 target_task=None,
                                                 payload={}), accumulator)
    self.assertEqual({'test_id': mock.ANY}, accumulator)

  def testSelector_Predicate(self):
    task = task_module.InMemoryTask(
        id='test_id',
        task_type='test',
        payload={},
        status='pending',
        dependencies=[])
    accumulator = {}
    evaluators.Selector(predicate=lambda *_: True)(task,
                                                   event_module.Event(
                                                       type='unimportant',
                                                       target_task=None,
                                                       payload={}), accumulator)
    self.assertEqual({'test_id': mock.ANY}, accumulator)

  def testSelector_Combinations(self):
    matching_task_types = (None, 'test')
    matching_event_types = (None, 'select')
    task = task_module.InMemoryTask(
        id='test_id',
        task_type='test',
        payload={},
        status='pending',
        dependencies=[])
    for task_type in matching_task_types:
      for event_type in matching_event_types:
        if not task_type and not event_type:
          continue
        accumulator = {}
        evaluators.Selector(
            task_type=task_type, event_type=event_type)(task,
                                                        event_module.Event(
                                                            type='select',
                                                            target_task=None,
                                                            payload={}),
                                                        accumulator)
        self.assertEqual({'test_id': mock.ANY}, accumulator,
                         'task_type = %s, event_type = %s')

    non_matching_task_types = ('unmatched_task',)
    non_matching_event_types = ('unmatched_event',)

    # Because the Selector's default behaviour is a logical disjunction of
    # matchers, we ensure that we will always find the tasks and handle events
    # if either (or both) match.
    for task_type in [t for t in matching_task_types if t is not None]:
      for event_type in non_matching_event_types:
        accumulator = {}
        evaluators.Selector(
            task_type=task_type, event_type=event_type)(task,
                                                        event_module.Event(
                                                            type='select',
                                                            target_task=None,
                                                            payload={}),
                                                        accumulator)
        self.assertEqual({'test_id': mock.ANY}, accumulator,
                         'task_type = %s, event_type = %s')
    for task_type in non_matching_task_types:
      for event_type in [t for t in matching_event_types if t is not None]:
        accumulator = {}
        evaluators.Selector(
            task_type=task_type, event_type=event_type)(task,
                                                        event_module.Event(
                                                            type='select',
                                                            target_task=None,
                                                            payload={}),
                                                        accumulator)
        self.assertEqual({'test_id': mock.ANY}, accumulator,
                         'task_type = %s, event_type = %s')
