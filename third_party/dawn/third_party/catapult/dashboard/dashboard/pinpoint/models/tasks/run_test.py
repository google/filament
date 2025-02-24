# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import itertools
import json
import logging

from dashboard.pinpoint.models import evaluators
from dashboard.pinpoint.models import task as task_module
from dashboard.pinpoint.models.quest import run_test as run_test_quest
from dashboard.pinpoint.models.tasks import find_isolate
from dashboard.services import swarming
from dashboard.services import request


class MarkTaskFailedAction(
    collections.namedtuple('MarkTaskFailedAction', ('job', 'task'))):
  __slots__ = ()

  def __str__(self):
    return 'MarkTaskFailedAction(task = %s)' % (self.task.id,)

  @task_module.LogStateTransitionFailures
  def __call__(self, _):
    task_module.UpdateTask(
        self.job, self.task.id, new_state='failed', payload=self.task.payload)


class ScheduleTestAction(
    collections.namedtuple('ScheduleTestAction',
                           ('job', 'task', 'properties'))):
  __slots__ = ()

  def __str__(self):
    return 'ScheduleTestAction(job = %s, task = %s)' % (self.job.job_id,
                                                        self.task.id)

  @task_module.LogStateTransitionFailures
  def __call__(self, _):
    logging.debug('Scheduling a Swarming task to run a test.')
    body = {
        'name':
            'Pinpoint job',
        'user':
            'Pinpoint',
        # TODO(dberris): Make these constants configurable?
        'priority':
            '100',
        'task_slices': [{
            'properties': self.properties,
            'expiration_secs': '86400',  # 1 day.
        }],

        # Since we're always going to be using the PubSub handling, we add the
        # tags unconditionally.
        'tags': [
            '%s:%s' % (k, v)
            for k, v in run_test_quest.SwarmingTagsFromJob(self.job).items()
        ],

        # Use an explicit service account.
        'service_account':
            run_test_quest._TESTER_SERVICE_ACCOUNT,

        # TODO(dberris): Consolidate constants in environment vars?
        'pubsub_topic':
            'projects/chromeperf/topics/pinpoint-swarming-updates',
        'pubsub_auth_token':
            'UNUSED',
        'pubsub_userdata':
            json.dumps({
                'job_id': self.job.job_id,
                'task': {
                    'type': 'run_test',
                    'id': self.task.id,
                },
            }),
    }
    self.task.payload.update({
        'swarming_request_body': body,
    })

    # Ensure that this thread/process/handler is the first to mark this task
    # 'ongoing'. Only proceed in scheduling a Swarming request if we're the
    # first one to do so.
    task_module.UpdateTask(
        self.job, self.task.id, new_state='ongoing', payload=self.task.payload)

    # At this point we know we were successful in transitioning to 'ongoing'.
    try:
      response = swarming.Swarming(
          self.task.payload.get('swarming_server')).Tasks().New(body)
      self.task.payload.update({
          'swarming_task_id': response.get('task_id'),
          'tries': self.task.payload.get('tries', 0) + 1
      })
    except request.RequestError as e:
      self.task.payload.update({
          'errors':
              self.task.payload.get('errors', []) + [{
                  'reason':
                      type(e).__name__,
                  'message':
                      'Encountered failure in swarming request: %s' % (e,),
              }]
      })

    # Update the payload with the task id from the Swarming request. Note that
    # this could also fail to commit.
    task_module.UpdateTask(self.job, self.task.id, payload=self.task.payload)


class PollSwarmingTaskAction(
    collections.namedtuple('PollSwarmingTaskAction', ('job', 'task'))):
  __slots__ = ()

  def __str__(self):
    return 'PollSwarmingTaskAction(job = %s, task = %s)' % (self.job.job_id,
                                                            self.task.id)

  @task_module.LogStateTransitionFailures
  def __call__(self, _):
    swarming_server = self.task.payload.get('swarming_server')
    task_id = self.task.payload.get('swarming_task_id')
    swarming_task = swarming.Swarming(swarming_server).Task(task_id)
    result = swarming_task.Result()
    self.task.payload.update({
        'swarming_task_result': {
            k: v
            for k, v in result.items()
            if k in {'bot_id', 'state', 'failure'}
        }
    })

    task_state = result.get('state')
    if task_state in {'PENDING', 'RUNNING'}:
      # Commit the task payload still.
      task_module.UpdateTask(self.job, self.task.id, payload=self.task.payload)
      return

    if task_state == 'EXPIRED':
      # TODO(dberris): Do a retry, reset the payload and run an "initiate"?
      self.task.payload.update({
          'errors': [{
              'reason': 'SwarmingExpired',
              'message': 'Request to the Swarming service expired.',
          }]
      })
      task_module.UpdateTask(
          self.job, self.task.id, new_state='failed', payload=self.task.payload)
      return

    if task_state != 'COMPLETED':
      task_module.UpdateTask(
          self.job, self.task.id, new_state='failed', payload=self.task.payload)
      return

    if 'outputs_ref' in result:
      self.task.payload.update({
          'isolate_server': result.get('outputs_ref').get('isolatedserver'),
          'isolate_hash': result.get('outputs_ref').get('isolated'),
      })
    elif 'cas_output_root' in result:
      self.task.payload.update({
          'cas_root_ref': result.get('cas_output_root'),
      })

    new_state = 'completed'
    if result.get('failure', False):
      new_state = 'failed'
      self.task.payload.update({
          'errors':
              self.task.payload.get('errors', []) + [{
                  'reason':
                      'RunTestFailed',
                  'message': ('Running the test failed, see isolate output: '
                              'https://%s/browse?digest=%s' % (
                                  self.task.payload.get('isolate_server'),
                                  self.task.payload.get('isolate_hash'),
                              ))
              }]
      })
    task_module.UpdateTask(
        self.job, self.task.id, new_state=new_state, payload=self.task.payload)


# Everything after this point aims to define an evaluator for the 'run_test'
# tasks.
class InitiateEvaluator:

  def __init__(self, job):
    self.job = job

  def __call__(self, task, event, accumulator):
    # Outline:
    #   - Check dependencies to see if they're 'completed', looking for:
    #     - Isolate server
    #     - Isolate hash
    dep_map = {
        dep: {
            'isolate_server': accumulator.get(dep, {}).get('isolate_server'),
            'isolate_hash': accumulator.get(dep, {}).get('isolate_hash'),
            'status': accumulator.get(dep, {}).get('status'),
        } for dep in task.dependencies
    }

    if not dep_map:
      logging.error(
          'No dependencies for "run_test" task, unlikely to proceed; task = %s',
          task)
      return None

    dep_value = {}
    if len(dep_map) > 1:
      # TODO(dberris): Figure out whether it's a valid use-case to have multiple
      # isolate inputs to Swarming.
      logging.error(('Found multiple dependencies for run_test; '
                     'picking a random input; task = %s'), task)
    dep_value.update(list(dep_map.values())[0])

    if dep_value.get('status') == 'failed':
      task.payload.update({
          'errors': [{
              'reason':
                  'BuildIsolateNotFound',
              'message': ('The build task this depends on failed, '
                          'so we cannot proceed to running the tests.')
          }]
      })
      return [MarkTaskFailedAction(self.job, task)]

    if dep_value.get('status') == 'completed':
      properties = {
          'inputs_ref': {
              'isolatedserver': dep_value.get('isolate_server'),
              'isolated': dep_value.get('isolate_hash'),
          },
          'extra_args': task.payload.get('extra_args'),
          'dimensions': task.payload.get('dimensions'),
          # TODO(dberris): Make these hard-coded-values configurable?
          'execution_timeout_secs': '21600',  # 6 hours, for rendering.mobile.
          'io_timeout_secs': '14400',  # 4 hours, to match the perf bots.
      }
      return [
          ScheduleTestAction(job=self.job, task=task, properties=properties)
      ]
    return None


class UpdateEvaluator:

  def __init__(self, job):
    self.job = job

  def __call__(self, task, event, accumulator):
    if event.target_task is None or event.target_task != task.id:
      return None

    # Check that the task has the required information to poll Swarming. In this
    # handler we're going to look for the 'swarming_task_id' key in the payload.
    # TODO(dberris): Move this out, when we incorporate validation properly.
    required_payload_keys = {'swarming_task_id', 'swarming_server'}
    missing_keys = required_payload_keys - set(task.payload)
    if missing_keys:
      logging.error('Failed to find required keys from payload: %s; task = %s',
                    missing_keys, task.payload)
      # See if the event has the data we want.
      task_id = event.payload.get('task_id')

      # If it doesn't (which is unlikely) we ought to fail the task.
      if task_id is None:
        return [MarkTaskFailedAction(self.job, task)]
      task.payload.update({'swarming_task_id': task_id})
    return [PollSwarmingTaskAction(job=self.job, task=task)]


class Evaluator(evaluators.SequenceEvaluator):

  def __init__(self, job):
    super().__init__(
        evaluators=(
            evaluators.FilteringEvaluator(
                predicate=evaluators.All(evaluators.TaskTypeEq('run_test'),),
                delegate=evaluators.DispatchByEventTypeEvaluator({
                    'initiate':
                        evaluators.FilteringEvaluator(
                            predicate=evaluators.Not(
                                evaluators.TaskStatusIn(
                                    {'ongoing', 'failed', 'completed'})),
                            delegate=InitiateEvaluator(job)),
                    # For updates, we want to ensure that the initiate evaluator
                    # has a chance to run on 'pending' tasks.
                    'update':
                        evaluators.SequenceEvaluator([
                            evaluators.FilteringEvaluator(
                                predicate=evaluators.Not(
                                    evaluators.TaskStatusIn(
                                        {'ongoing', 'failed', 'completed'})),
                                delegate=InitiateEvaluator(job)),
                            evaluators.FilteringEvaluator(
                                predicate=evaluators.TaskStatusIn({'ongoing'}),
                                delegate=UpdateEvaluator(job)),
                        ])
                })),
            evaluators.TaskPayloadLiftingEvaluator(),
        ))


def ReportError(task, _, accumulator):
  # TODO(dberris): Factor this out into smaller pieces?
  task_errors = []
  logging.debug('Validating task: %s', task)
  if len(task.dependencies) != 1:
    task_errors.append({
        'cause':
            'DependencyError',
        'message':
            'Task must have exactly 1 dependency; has %s' %
            (len(task.dependencies),)
    })

  if task.status == 'ongoing':
    required_payload_keys = {'swarming_task_id', 'swarming_server'}
    missing_keys = required_payload_keys - (
        set(task.payload) & required_payload_keys)
    if missing_keys:
      task_errors.append({
          'cause': 'MissingRequirements',
          'message': 'Missing required keys %s in task payload.' % missing_keys
      })
  elif task.status == 'pending' and task.dependencies and all(
      accumulator.get(dep, {}).get('status') == 'completed'
      for dep in task.dependencies):
    required_dependency_keys = {'isolate_server', 'isolate_hash'}
    dependency_keys = set(
        itertools.chain(
            *[accumulator.get(dep, []) for dep in task.dependencies]))
    missing_keys = required_dependency_keys - (
        dependency_keys & required_dependency_keys)
    if missing_keys:
      task_errors.append({
          'cause':
              'MissingDependencyInputs',
          'message':
              'Missing keys from dependency payload: %s' % (missing_keys,)
      })

  if task_errors:
    accumulator.update({task.id: {'errors': task_errors}})


class Validator(evaluators.FilteringEvaluator):

  def __init__(self):
    super().__init__(
        predicate=evaluators.TaskTypeEq('run_test'), delegate=ReportError)


def TestSerializer(task, _, accumulator):
  results = accumulator.setdefault(task.id, {})
  results.update({
      'completed':
          task.status in {'completed', 'failed', 'cancelled'},
      'exception':
          ','.join(e.get('reason') for e in task.payload.get('errors', [])) or
          None,
      'details': []
  })

  swarming_task_result = task.payload.get('swarming_task_result', {})
  swarming_server = task.payload.get('swarming_server')
  bot_id = swarming_task_result.get('bot_id')
  if bot_id:
    results['details'].append({
        'key': 'bot',
        'value': bot_id,
        'url': swarming_server + '/bot?id=' + bot_id
    })
  task_id = task.payload.get('swarming_task_id')
  if task_id:
    results['details'].append({
        'key': 'task',
        'value': task_id,
        'url': swarming_server + '/task?id=' + task_id
    })
  isolate_hash = task.payload.get('isolate_hash')
  isolate_server = task.payload.get('isolate_server')
  if isolate_hash and isolate_server:
    results['details'].append({
        'key': 'isolate',
        'value': isolate_hash,
        'url': '%s/browse?digest=%s' % (isolate_server, isolate_hash)
    })


class Serializer(evaluators.FilteringEvaluator):

  def __init__(self):
    super().__init__(
        predicate=evaluators.TaskTypeEq('run_test'), delegate=TestSerializer)


def TaskId(change, attempt):
  return 'run_test_%s_%s' % (change, attempt)


TaskOptions = collections.namedtuple('TaskOptions',
                                     ('build_options', 'swarming_server',
                                      'dimensions', 'extra_args', 'attempts'))


def CreateGraph(options):
  if not isinstance(options, TaskOptions):
    raise ValueError('options is not an instance of run_test.TaskOptions')
  subgraph = find_isolate.CreateGraph(options.build_options)
  find_isolate_tasks = [
      task for task in subgraph.vertices if task.vertex_type == 'find_isolate'
  ]
  assert len(find_isolate_tasks) == 1
  find_isolate_task = find_isolate_tasks[0]
  subgraph.vertices.extend([
      task_module.TaskVertex(
          id=TaskId(
              find_isolate.ChangeId(options.build_options.change), attempt),
          vertex_type='run_test',
          payload={
              'swarming_server': options.swarming_server,
              'dimensions': options.dimensions,
              'extra_args': options.extra_args,
              'change': options.build_options.change.AsDict(),
              'index': attempt,
          }) for attempt in range(options.attempts)
  ])
  subgraph.edges.extend([
      task_module.Dependency(from_=task.id, to=find_isolate_task.id)
      for task in subgraph.vertices
      if task.vertex_type == 'run_test'
  ])
  return subgraph
