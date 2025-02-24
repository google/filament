# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Quest and Execution for running a test in Swarming.

This is the only Quest/Execution where the Execution has a reference back to
modify the Quest.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from datetime import datetime
import json
import logging
import random
import shlex

from dashboard.common import cloud_metric
from dashboard.pinpoint.models import errors
from dashboard.pinpoint.models.quest import execution as execution_module
from dashboard.pinpoint.models.quest import quest
from dashboard.services import swarming

_TESTER_SERVICE_ACCOUNT = (
    'chrome-tester@chops-service-accounts.iam.gserviceaccount.com')
_CAS_DEFAULT_INSTANCE = (
    'projects/chromium-swarm/instances/default_instance'
)


def SwarmingTagsFromJob(job):
  ret = {
      'pinpoint_job_id': job.job_id,
      'url': job.url,
      'comparison_mode': job.comparison_mode,
      'pinpoint_task_kind': 'test',
      'pinpoint_user': job.user,
  }

  if job.batch_id is not None:
    ret['pinpoint_batch_id'] = job.batch_id

  return ret


class RunTest(quest.Quest):

  def __init__(self, swarming_server, dimensions, extra_args, swarming_tags,
               command, relative_cwd):
    """RunTest Quest

    Args:
      swarming_server: a string indicating the swarming server.
      dimensions: a list of dimensions.
      extra_args: a list of strings treated as additional arguments to
          provide to the task in Swarming.
      swarming_tags: a dict of swarming tags.
      command: a list of strings to be provided to the Swarming task command.
      relative_cwd: a string indicating the working directory in the isolate.
    """
    self._swarming_server = swarming_server
    self._dimensions = dimensions
    self._extra_args = extra_args
    self._swarming_tags = {} if not swarming_tags else swarming_tags
    self._command = command
    self._relative_cwd = relative_cwd

    # We want subsequent executions use the same bot as the first one.
    self._canonical_executions = []

    self._bots = None
    self._comparison_mode = None
    self._attempt_count = None

    self._started_executions = {}

  def __eq__(self, other):
    return (isinstance(other, type(self))
            and self._swarming_server == other._swarming_server
            and self._dimensions == other._dimensions
            and self._extra_args == other._extra_args
            and self._canonical_executions == other._canonical_executions
            and self._command == other._command
            and self._relative_cwd == other._relative_cwd
            and self._bots == other._bots
            and self._comparison_mode == other._comparison_mode
            and self._attempt_count == other._attempt_count
            and self._started_executions == other._started_executions)

  def __hash__(self):
    return hash(self.__str__())

  def __str__(self):
    return 'Test'

  def __setstate__(self, state):
    self.__dict__ = state  # pylint: disable=attribute-defined-outside-init
    if not hasattr(self, '_swarming_tags'):
      self._swarming_tags = {}
    if not hasattr(self, '_command'):
      self._command = None
    if not hasattr(self, '_relative_cwd'):
      self._relative_cwd = None

  @property
  def command(self):
    return getattr(self, '_command')

  @property
  def relative_cwd(self):
    return getattr(self, '_relative_cwd', 'out/Release')

  def PropagateJob(self, job):
    if not hasattr(self, '_swarming_tags'):
      self._swarming_tags = {}
    self._swarming_tags.update(SwarmingTagsFromJob(job))
    self._comparison_mode = job.comparison_mode
    self._attempt_count = job.state.attempt_count
    self._bots = [str(b) for b in job.bots]

  def Start(self, change, isolate_server, isolate_hash):
    return self._Start(change, isolate_server, isolate_hash, self._extra_args,
                       {}, None)

  def _Start(self, change, isolate_server, isolate_hash, extra_args,
             swarming_tags, execution_timeout_secs):
    if not hasattr(self, '_started_executions'):
      self._started_executions = {}
    if change not in self._started_executions:
      self._started_executions[change] = []
    index = len(self._started_executions[change])

    if self._swarming_tags:
      swarming_tags.update(self._swarming_tags)

    dimensions = self._GetDimensions(index)

    test_execution = _RunTestExecution(
        self,
        self._swarming_server,
        dimensions,
        extra_args,
        isolate_server,
        isolate_hash,
        swarming_tags,
        index,
        change,
        command=self.command,
        relative_cwd=self.relative_cwd,
        execution_timeout_secs=execution_timeout_secs)
    self._started_executions[change].append(test_execution)
    return test_execution

  def _StartAllTasks(self):
    # We need to wait for all of the builds to be complete.
    if len(self._started_executions) != 2:
      return
    logging.debug('bots in quest: %s', [str(b) for b in self._bots])
    a_list, b_list = self._started_executions.values()
    if len(a_list) != self._attempt_count or len(b_list) != self._attempt_count:
      return

    orderings = self._GetABOrderings(self._attempt_count)
    for i in range(self._attempt_count):
      if orderings[i]:
        a_list[i]._StartTask()
        b_list[i]._StartTask()
      else:
        b_list[i]._StartTask()
        a_list[i]._StartTask()

  def _GetABOrderings(self, attempt_count):
    # Get a list of if a/0 or b/1 should go first, such that
    # - half of As and half of Bs go first
    # - which half is random
    half = attempt_count // 2
    orderings = [0] * half + [1] * half
    if attempt_count % 2:
      orderings.append(0)
    random.shuffle(orderings)
    return orderings

  def _GetDimensions(self, index):
    # Adds a bot_id to dimensions
    if not hasattr(self, '_bots') or self._bots is None:
      return self._dimensions
    dimensions = list(self._dimensions)

    bot_id = self._bots[index % len(self._bots)]
    if bot_id:
      logging.debug('add bot_id for index %s and bots %s', str(index), bot_id)
      dimensions.append({'key': 'id', 'value': bot_id})
    return dimensions

  @classmethod
  def _ComputeCommand(cls, arguments):
    """Computes the relative_cwd and command properties for Swarming tasks.

    This can be overridden in the derived classes to allow custom computation
    of the relative working directory and the command to be provided to the
    Swarming task.

    Args:
      arguments: a dict of arguments provided to a Pinpoint job.

    Returns a tuple of (relative current working dir, command)."""
    return arguments.get('relative_cwd'), arguments.get('command')

  @classmethod
  def FromDict(cls, arguments):
    swarming_server = arguments.get('swarming_server')
    if not swarming_server:
      raise TypeError('Missing a "swarming_server" argument.')

    dimensions = arguments.get('dimensions')
    if not dimensions:
      raise TypeError('Missing a "dimensions" argument.')
    if isinstance(dimensions, str):
      dimensions = json.loads(dimensions)
    if not any(dimension['key'] == 'pool' for dimension in dimensions):
      raise ValueError('Missing a "pool" dimension.')
    relative_cwd, command = cls._ComputeCommand(arguments)
    extra_test_args = cls._ExtraTestArgs(arguments)
    swarming_tags = cls._GetSwarmingTags(arguments)
    return cls(swarming_server, dimensions, extra_test_args, swarming_tags,
               command, relative_cwd)

  @classmethod
  def _ExtraTestArgs(cls, arguments):
    extra_test_args = arguments.get('extra_test_args')
    if not extra_test_args:
      return []

    # We accept a json list or a string. If it can't be loaded as json, we
    # fall back to assuming it's a string argument.
    try:
      extra_test_args = json.loads(extra_test_args)
    except ValueError:
      extra_test_args = shlex.split(extra_test_args)
    if not isinstance(extra_test_args, list):
      raise TypeError('extra_test_args must be a list: %s' % extra_test_args)
    return extra_test_args

  @classmethod
  def _GetSwarmingTags(cls, arguments):
    pass


class _RunTestExecution(execution_module.Execution):

  def __init__(self,
               containing_quest,
               swarming_server,
               dimensions,
               extra_args,
               isolate_server,
               isolate_hash,
               swarming_tags,
               index,
               change,
               command=None,
               relative_cwd='out/Release',
               execution_timeout_secs=None):
    super().__init__()
    self._quest = containing_quest
    self._bot_id = None
    self._command = command
    self._dimensions = dimensions
    self._extra_args = extra_args
    self._isolate_hash = isolate_hash
    self._isolate_server = isolate_server
    self._relative_cwd = relative_cwd
    self._swarming_server = swarming_server
    self._swarming_tags = swarming_tags
    self._index = index
    self._change = change
    self._execution_timeout_secs = execution_timeout_secs
    self._task_id = None

  def __setstate__(self, state):
    self.__dict__ = state  # pylint: disable=attribute-defined-outside-init
    if not hasattr(self, '_swarming_tags'):
      self._swarming_tags = {}
    if not hasattr(self, '_command'):
      self._command = None
    if not hasattr(self, '_relative_cwd'):
      self._relative_cwd = 'out/Release'
    if not hasattr(self, '_execution_timeout_secs'):
      self._execution_timeout_secs = None

  @property
  def bot_id(self):
    return self._bot_id

  @property
  def command(self):
    return getattr(self, '_command')

  @property
  def relative_cwd(self):
    return getattr(self, '_relative_cwd', 'out/Release')

  @property
  def execution_timeout_secs(self):
    return getattr(self, '_execution_timeout_secs')

  def _AsDict(self):
    details = []
    if self._bot_id:
      details.append({
          'key': 'bot',
          'value': self._bot_id,
          'url': self._swarming_server + '/bot?id=' + self._bot_id,
      })
    if self._task_id:
      details.append({
          'key': 'task',
          'value': self._task_id,
          'url': self._swarming_server + '/task?id=' + self._task_id,
      })
    if self._result_arguments:
      cas_root_ref = self._result_arguments.get('cas_root_ref')
      if cas_root_ref is not None:
        digest = cas_root_ref['digest']
        # Backward compatibility for loading the job finished under the
        # Swarming API V1. The field names was in snake_case for V1 and
        # in camelCase in V2.
        cas_instance = cas_root_ref.get('casInstance') or cas_root_ref.get(
            'cas_instance')
        digest_byte_size = digest.get('sizeBytes') or digest.get('size_bytes')
        url = 'https://cas-viewer.appspot.com/{}/blobs/{}/{}/tree'.format(
            cas_instance, digest['hash'], digest_byte_size)
        value = '{}/{}'.format(digest['hash'], digest_byte_size)
      else:
        url = (self._result_arguments['isolate_server'] + '/browse?digest=' +
               self._result_arguments['isolate_hash'])
        value = self._result_arguments['isolate_hash']
      details.append({
          'key': 'isolate',
          'value': value,
          'url': url,
      })
    return details

  def _Poll(self):
    if not self._task_id:
      self._StartTasksIfMasterOrNotTry()
      return

    logging.debug('_RunTestExecution Polling swarming: %s', self._task_id)
    swarming_task = swarming.Swarming(self._swarming_server).Task(self._task_id)

    result = swarming_task.Result()
    logging.debug('swarming response: %s', result)

    if 'botId' in result:
      # For bisects, this will be set after the task is allocated to a bot.
      # A/Bs will set this elsewhere.
      self._bot_id = result['botId']

    if result['state'] == 'PENDING' or result['state'] == 'RUNNING':
      if self._bot_id:
        if not swarming.IsBotAlive(self._bot_id, self._swarming_server):
          raise errors.SwarmingTaskError('Bot is dead.')
      return

    if result['state'] == 'EXPIRED':
      raise errors.SwarmingExpired()

    if result['state'] != 'COMPLETED':
      raise errors.SwarmingTaskError(result['state'])

    # The swarming task is completed. Report the pending time
    self._ReportSwarmingJobsMetric(result)

    if result.get('failure', False):
      if 'outputsRef' not in result:
        task_url = '%s/task?id=%s' % (self._swarming_server, self._task_id)
        raise errors.SwarmingTaskFailed('%s' % (task_url,))
      isolate_output_url = '%s/browse?digest=%s' % (
          result['outputsRef']['isolatedserver'],
          result['outputsRef']['isolated'])
      raise errors.SwarmingTaskFailed('%s' % (isolate_output_url,))

    if 'casOutputRoot' in result:
      result_arguments = {
          'cas_root_ref': result['casOutputRoot'],  #CASReference
      }
    else:
      result_arguments = {
          'isolate_server': result['outputsRef']['isolatedserver'],
          'isolate_hash': result['outputsRef']['isolated'],
      }

    self._Complete(result_arguments=result_arguments)

  @staticmethod
  def _ReportSwarmingJobsMetric(result):
    """Report metrics on the completed swarming task.

    Including:
      pending_time: how long the task has waited, such as for a busy bot
      running_time: how long the task runs.
    Dimensions are:
      task_id: id of the swarming task
      bot_os: the detailed os version of the bot.
      bot_id: bot id.
      pinpoint_job_type: try or bisect. Expecting longer wait on try jobs
        as one of the two branches will wait for the same bot from the
        other branch.
      pinpont_job_id: id of the pinpoint job which launches the swarming task
    """
    created_ts = result.get('created_ts')
    started_ts = result.get('started_ts')
    completed_ts = result.get('completed_ts')
    if created_ts and started_ts and completed_ts:
      pending_time = datetime.fromisoformat(
          started_ts) - datetime.fromisoformat(created_ts)
      running_time = datetime.fromisoformat(
          completed_ts) - datetime.fromisoformat(started_ts)

      bot_os = pinpoint_job_type = pinpoint_job_id = None
      for tag in result.get('tags', ''):
        key, value = tag.split(':', 1)
        if key == 'os':
          bot_os = value
        if key == 'comparison_mode':
          pinpoint_job_type = value
        if key == 'pinpoint_job_id':
          pinpoint_job_id = value

      # debug infor for crbug/1422306
      if None in (bot_os, pinpoint_job_type, pinpoint_job_id):
        logging.debug('Missing value in swarming result: %s', result)

      cloud_metric.PublishPinpointSwarmingPendingMetric(
          task_id=result.get('task_id'),
          pinpoint_job_type=pinpoint_job_type,
          pinpoint_job_id=pinpoint_job_id,
          bot_id=result.get('bot_id'),
          bot_os=bot_os,
          pending_time=pending_time.total_seconds())

      cloud_metric.PublishPinpointSwarmingRuntimeMetric(
          task_id=result.get('task_id'),
          pinpoint_job_type=pinpoint_job_type,
          pinpoint_job_id=pinpoint_job_id,
          bot_id=result.get('bot_id'),
          bot_os=bot_os,
          running_time=running_time.total_seconds())

  @staticmethod
  def _IsCasDigest(d):
    return '/' in d

  def _StartTasksIfMasterOrNotTry(self):
    if not hasattr(self, '_quest') or self._quest._comparison_mode != 'try':
      self._StartTask()
      return

    if self._change.variant != 0 or self._index != 0:
      # This task is not responsible for kicking off all other tasks
      return

    # This will call _StartTask on all tasks, including this one
    self._quest._StartAllTasks()

  def _StartTask(self):
    """Kick off a Swarming task to run a test."""
    # TODO(fancl): Seperate cas input from isolate (including endpoint and
    # datastore module)
    if self._IsCasDigest(self._isolate_hash):
      cas_hash, cas_size = self._isolate_hash.split('/', 1)
      instance = self._isolate_server
      # This is a workaround for build cached uploaded before crrev/c/2964515
      # landed. We can delete it after all caches expired.
      if instance.startswith('https://'):
        instance = _CAS_DEFAULT_INSTANCE
      input_ref = {
          'casInputRoot': {
              'casInstance': instance,
              'digest': {
                  'hash': cas_hash,
                  'sizeBytes': int(cas_size),
              }
          }
      }
    else:
      input_ref = {
          'inputsRef': {
              'isolatedserver': self._isolate_server,
              'isolated': self._isolate_hash,
          }
      }

    properties = {
        'extraArgs': self._extra_args,
        'dimensions': self._dimensions,
        'executionTimeoutSecs': str(self.execution_timeout_secs or 2700),
        'ioTimeoutSecs': str(self.execution_timeout_secs or 2700),
    }
    properties.update(**input_ref)

    for d in self._dimensions:
      if d['key'] == 'id':
        self._bot_id = d['value']

    if self.command:
      properties.update({
          # Set the relative current working directory to be the root of the
          # isolate.
          'relativeCwd': self.relative_cwd,

          # Use the command provided in the creation of the execution.
          'command': self.command + self._extra_args,
      })

      # Swarming requires that if 'command' is present in the request, that we
      # not provide 'extraArgs'.
      del properties['extraArgs']

    body = {
        'realm':
            'chrome:pinpoint',
        'name':
            'Pinpoint job',
        'user':
            'Pinpoint',
        'priority':
            '100',
        'serviceAccount':
            _TESTER_SERVICE_ACCOUNT,
        'taskSlices': [{
            'properties': properties,
            'expirationSecs': '86400',  # 1 day.
        }],
    }

    if self._swarming_tags:
      # This means we have additional information available about the Pinpoint
      # tags, and we should add those to the Swarming Pub/Sub updates.
      body.update(
          {'tags': ['%s:%s' % (k, v) for k, v in self._swarming_tags.items()]})

    logging.debug('Requesting swarming task with parameters: %s', body)
    response = swarming.Swarming(self._swarming_server).Tasks().New(body)
    logging.debug('Response: %s', response)
    self._task_id = response['taskId']
