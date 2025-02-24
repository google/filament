# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import json
import logging

from dashboard.pinpoint.models import change as change_module
from dashboard.pinpoint.models import isolate
from dashboard.pinpoint.models import task as task_module
from dashboard.pinpoint.models import evaluators
from dashboard.pinpoint.models.change import commit as commit_module
from dashboard.pinpoint.models.quest import find_isolate as find_isolate_quest
from dashboard.services import buildbucket_service
from dashboard.services import request

FAILURE_MAPPING = {'FAILURE': 'failed', 'CANCELLED': 'cancelled'}


class ScheduleBuildAction:
  """Action to schedule a build via BuildBucket.

  This action will schedule a build via the BuildBucket API, and ensure that
  Pinpoint is getting updates via PubSub on request completions.

  Side Effects:

    - The Action will update the Task payload to include the build request
      information and the response from the BuildBucket service, and set the
      state to 'ongoing' on success, 'failed' otherwise.

  """

  def __init__(self, job, task, change):
    self.job = job
    self.task = task
    self.change = change

  @task_module.LogStateTransitionFailures
  def __call__(self, accumulator):
    # TODO(dberris): Maybe use a value in the accumulator to check whether we
    # should bail?
    self.task.payload.update({'tries': self.task.payload.get('tries', 0) + 1})
    task_module.UpdateTask(
        self.job, self.task.id, new_state='ongoing', payload=self.task.payload)
    result = find_isolate_quest.RequestBuild(
        self.task.payload.get('builder'), self.change,
        self.task.payload.get('bucket'),
        find_isolate_quest.BuildTagsFromJob(self.job), self.task)
    self.task.payload.update({'buildbucket_result': result})

    # TODO(dberris): Poll the ongoing build if the attempt to update fails, if
    # we have the data in payload?
    task_module.UpdateTask(self.job, self.task.id, payload=self.task.payload)

  def __str__(self):
    return 'Build Action(job = %s, task = %s)' % (self.job.job_id, self.task.id)


class UpdateBuildStatusAction(
    collections.namedtuple('UpdateBuildStatusAction',
                           ('job', 'task', 'change', 'event'))):
  __slots__ = ()

  @task_module.LogStateTransitionFailures
  def __call__(self, accumulator):
    # The task contains the buildbucket_result which we need to update by
    # polling the status of the id.
    build_details = self.task.payload.get('buildbucket_result')
    if not build_details:
      logging.error(
          'No build details in attempt to update build status; task = %s',
          self.task)
      task_module.UpdateTask(self.job, self.task.id, new_state='failed')
      return

    # Attempt to use the payload in a buildbucket pub/sub update to handle the
    # update without polling. Only poll as a last resort.
    build = self.event.payload
    if build is None or 'id' not in build:
      try:
        build_id = build_details.get('build', {}).get('id')
        if build_id is None:
          logging.error('No build details stored in task payload; task = %s',
                        self.task)
          self.task.payload.update({
              'errors':
                  self.task.payload.get('errors', []) + [{
                      'reason': 'MissingBuildDetails',
                      'message': 'Cannot find build details in task.',
                  }]
          })
          task_module.UpdateTask(
              self.job,
              self.task.id,
              new_state='failed',
              payload=self.task.payload)
          return

        build = buildbucket_service.GetJobStatus(build_id).get('build', {})
      except request.RequestError as e:
        logging.error('Failed getting Buildbucket Job status: %s', e)
        self.task.payload.update({
            'errors':
                self.task.payload.get('errors', []) + [{
                    'reason': type(e).__name__,
                    'message': 'Service request error response: %s' % (e,),
                }]
        })
        task_module.UpdateTask(
            self.job,
            self.task.id,
            new_state='failed',
            payload=self.task.payload)
        return

    logging.debug('buildbucket task response: %s', build)

    # Update the buildbucket result.
    self.task.payload.update({
        'buildbucket_job_status': build,
    })

    # Decide whether the build was successful or not.
    if build.get('status') != 'COMPLETED':
      # Skip this update.
      return

    result = build.get('result')
    if not result:
      logging.debug('Missing result field in response, bailing.')
      self.task.payload.update({
          'errors':
              self.task.payload.get('errors', []) + [{
                  'reason': 'InvalidResponse',
                  'message': 'Response is missing the "result" field.'
              }]
      })
      task_module.UpdateTask(
          self.job, self.task.id, new_state='failed', payload=self.task.payload)
      return

    self.task.payload.update({'build_url': build.get('url')})

    if result in FAILURE_MAPPING:
      self.task.payload.update({
          'errors':
              self.task.payload.get('errors', []) + [{
                  'reason':
                      'BuildFailed',
                  'message':
                      'Swarming task %s failed with status "%s"' %
                      (build.get('id'), result)
              }]
      })
      task_module.UpdateTask(
          self.job,
          self.task.id,
          new_state=FAILURE_MAPPING[result],
          payload=self.task.payload)
      return

    # Parse the result and mark this task completed.
    if 'result_details_json' not in build:
      self.task.payload.update({
          'errors': [{
              'reason':
                  'BuildIsolateNotFound',
              'message':
                  'Could not find isolate for build at %s' % (self.change,)
          }]
      })
      task_module.UpdateTask(
          self.job, self.task.id, new_state='failed', payload=self.task.payload)
      return

    try:
      result_details = json.loads(build['result_details_json'])
    except ValueError as e:
      self.task.payload.update({
          'errors': [{
              'reason': 'BuildIsolateNotFound',
              'message': 'Invalid JSON response: %s' % (e,)
          }]
      })
      task_module.UpdateTask(
          self.job, self.task.id, new_state='failed', payload=self.task.payload)
      return

    if 'properties' not in result_details:
      self.task.payload.update({
          'errors': [{
              'reason':
                  'BuildIsolateNotFound',
              'message':
                  'Could not find result details for build at %s' %
                  (self.change,)
          }]
      })
      task_module.UpdateTask(
          self.job, self.task.id, new_state='failed', payload=self.task.payload)
      return

    properties = result_details['properties']

    # Validate whether the properties in the result include required data.
    required_keys = {'isolate_server', 'got_revision_cp'}
    missing_keys = required_keys - set(properties)
    if missing_keys:
      logging.warning('Missing expected keys: %s. Property keys: %s',
                      missing_keys, set(properties))
      self.task.payload.update({
          'errors': [{
              'reason':
                  'BuildIsolateNotFound',
              'message':
                  'Properties in result missing required data: %s' %
                  (missing_keys,)
          }]
      })
      task_module.UpdateTask(
          self.job, self.task.id, new_state='failed', payload=self.task.payload)
      return

    commit_position = properties['got_revision_cp'].replace('@', '(at)')
    suffix = ('without_patch'
              if 'patch_storage' not in properties else 'with_patch')
    key = '_'.join(('swarm_hashes', commit_position, suffix))

    if self.task.payload.get('target') not in properties.get(key, {}):
      # TODO(dberris): Update the job state with an exception, or set of
      # failures.
      self.task.payload.update({
          'errors': [{
              'reason':
                  'BuildIsolateNotFound',
              'message':
                  'Could not find isolate for build at %s' % (self.change,)
          }]
      })
      task_module.UpdateTask(
          self.job, self.task.id, new_state='failed', payload=self.task.payload)
      return

    self.task.payload.update({
        'isolate_server': properties['isolate_server'],
        'isolate_hash': properties[key][self.task.payload.get('target')]
    })
    task_module.UpdateTask(
        self.job,
        self.task.id,
        new_state='completed',
        payload=self.task.payload)

  def __str__(self):
    return 'Update Build Action <job = %s, task = %s>' % (self.job.job_id,
                                                          self.task.id)


class InitiateEvaluator:

  def __init__(self, job):
    self.job = job

  @task_module.LogStateTransitionFailures
  def __call__(self, task, _, change):
    if task.status == 'ongoing':
      logging.warning(
          'Ignoring an initiate event on an ongoing task; task = %s', task.id)
      return None

    # Outline:
    #   - Check if we can find the isolate for this revision.
    #     - If found, update the payload of the task and update accumulator with
    #       result for this task.
    #     - If not found, schedule a build for this revision, update the task
    #       payload with the build details, wait for updates.
    try:
      change = change_module.Change(
          commits=[
              commit_module.Commit(c['repository'], c['git_hash'])
              for c in task.payload.get('change', {}).get('commits', [])
          ],
          patch=task.payload.get('patch'))
      logging.debug('Looking up isolate for change = %s', change)
      isolate_server, isolate_hash = isolate.Get(
          task.payload.get('builder'), change, task.payload.get('target'))
      task.payload.update({
          'isolate_server': isolate_server,
          'isolate_hash': isolate_hash,
      })

      # At this point we've found an isolate from a previous build, so we mark
      # the task 'completed' and allow tasks that depend on the isolate to see
      # this information.
      @task_module.LogStateTransitionFailures
      def CompleteWithCachedIsolate(_):
        task_module.UpdateTask(
            self.job, task.id, new_state='completed', payload=task.payload)

      return [CompleteWithCachedIsolate]
    except KeyError as e:
      logging.error('Failed to find isolate for task = %s;\nError: %s', task.id,
                    e)
      return [ScheduleBuildAction(self.job, task, change)]
    return None


class UpdateEvaluator:

  def __init__(self, job):
    self.job = job

  def __call__(self, task, event, _):
    # Outline:
    #   - Check build status payload.
    #     - If successful, update the task payload with status and relevant
    #       information, propagate information into the accumulator.
    #     - If unsuccessful:
    #       - Retry if the failure is a retryable error (update payload with
    #         retry information)
    #       - Fail if failure is non-retryable or we've exceeded retries.
    if event.type == 'update':
      change = change_module.ReconstituteChange(task.payload.get('change'))
      return [UpdateBuildStatusAction(self.job, task, change, event)]
    return None


class Evaluator(evaluators.SequenceEvaluator):

  def __init__(self, job):
    super().__init__(
        evaluators=(
            evaluators.TaskPayloadLiftingEvaluator(),
            evaluators.FilteringEvaluator(
                predicate=evaluators.All(
                    evaluators.TaskTypeEq('find_isolate'),
                    evaluators.TaskIsEventTarget(),
                    evaluators.Not(
                        evaluators.TaskStatusIn(
                            {'completed', 'failed', 'cancelled'})),
                ),
                delegate=evaluators.DispatchByEventTypeEvaluator({
                    'initiate': InitiateEvaluator(job),
                    'update': UpdateEvaluator(job),
                })),
        ))


def BuildSerializer(task, _, accumulator):
  results = accumulator.get(task.id, {})
  results.update({
      'completed':
          task.status in {'completed', 'failed', 'cancelled'},
      'exception':
          ','.join(e.get('reason') for e in task.payload.get('errors', [])) or
          None,
      'details': [{
          'key': 'builder',
          'value': task.payload.get('builder'),
          'url': task.payload.get('build_url'),
      }]
  })

  buildbucket_result = task.payload.get('buildbucket_result')
  if buildbucket_result:
    build = buildbucket_result.get('build')
    if build:
      results.get('details').append({
          'key': 'build',
          'value': build.get('id'),
          'url': build.get('url'),
      })

  if {'isolate_server', 'isolate_hash'} & set(task.payload):
    results.get('details').append({
        'key':
            'isolate',
        'value':
            task.payload.get('isolate_hash'),
        'url':
            '%s/browse?digest=%s' % (task.payload.get('isolate_server'),
                                     task.payload.get('isolate_hash'))
    })

  accumulator.update({task.id: results})


class Serializer(evaluators.FilteringEvaluator):

  def __init__(self):
    super().__init__(
        predicate=evaluators.TaskTypeEq('find_isolate'),
        delegate=BuildSerializer)


TaskOptions = collections.namedtuple('TaskOptions',
                                     ('builder', 'target', 'bucket', 'change'))


def ChangeId(change):
  return '_'.join(change.id_string.split(' '))


def CreateGraph(options):
  if not isinstance(options, TaskOptions):
    raise ValueError('options not an instance of find_isolate.TaskOptions')

  return task_module.TaskGraph(
      vertices=[
          task_module.TaskVertex(
              id='find_isolate_%s' % (ChangeId(options.change),),
              vertex_type='find_isolate',
              payload={
                  'builder': options.builder,
                  'target': options.target,
                  'bucket': options.bucket,
                  'change': options.change.AsDict()
              })
      ],
      edges=[])
