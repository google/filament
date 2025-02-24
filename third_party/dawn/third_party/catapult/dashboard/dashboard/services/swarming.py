# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Functions for interfacing with the Chromium Swarming Server.

The Swarming Server is a task distribution service. It can be used to kick off
a test run.

API explorer: https://goo.gl/uxPUZo
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
import random

from dashboard.services import request

_API_PATH = '_ah/api/swarming/v1'
_V2_PRPC_PREFIX = 'prpc/swarming.v2'


class Swarming:

  def __init__(self, server):
    self._server = server

  def Bot(self, bot_id):
    return Bot(self._server, bot_id)

  def Bots(self):
    return Bots(self._server)

  def Task(self, task_id):
    return Task(self._server, task_id)

  def Tasks(self):
    return Tasks(self._server)


class Bot:

  def __init__(self, server, bot_id):
    self._server = server
    self._bot_id = bot_id

  def GetV1(self):
    """Returns information about a known bot.

    This includes its state and dimensions, and if it is currently running a
    task."""
    return self._Request('get')

  def Get(self):
    """Returns information about a known bot.

    This includes its state and dimensions, and if it is currently running a
    task."""
    url = '%s/%s.Bots/GetBot' % (self._server, _V2_PRPC_PREFIX)
    request_body = {'botId': self._bot_id}
    return request.RequestJson(url, method='POST', body=request_body)

  # /prpc/swarming.v2.Bots/ListBotTasks
  def Tasks(self):
    """Lists a given bot's tasks within the specified date range."""
    return self._Request('tasks')

  def _Request(self, path, **kwargs):
    url = '%s/%s/bot/%s/%s' % (self._server, _API_PATH, self._bot_id, path)
    return request.RequestJson(url, **kwargs)


class Bots:

  def __init__(self, server):
    self._server = server

  def ListV1(self,
             cursor=None,
             dimensions=None,
             is_dead=None,
             limit=None,
             quarantined=None):
    """Provides list of known bots. Deleted bots will not be listed."""
    if dimensions:
      dimensions = tuple(
          ':'.join(dimension) for dimension in dimensions.items())

    url = '%s/%s/bots/list' % (self._server, _API_PATH)
    return request.RequestJson(
        url,
        cursor=cursor,
        dimensions=dimensions,
        is_dead=is_dead,
        limit=limit,
        quarantined=quarantined)

  def List(self,
           cursor=None,
           dimensions=None,
           is_dead=None,
           limit=None,
           quarantined=None):
    """Provides list of known bots. Deleted bots will not be listed."""
    if dimensions:
      dimensions = [{'key': k, 'value': v} for k, v in dimensions.items()]
    url = '%s/%s.Bots/ListBots' % (self._server, _V2_PRPC_PREFIX)
    request_body = {
        'cursor': cursor,
        'dimensions': dimensions,
        'isDead': is_dead,
        'limit': limit or 100,
        'quarantined': quarantined,
    }
    return request.RequestJson(url, method='POST', body=request_body)


class Task:

  def __init__(self, server, task_id):
    self._server = server
    self._task_id = task_id

  # /prpc/swarming.v2.Tasks/CancelTask
  def Cancel(self):
    """Cancels a task.

    If a bot was running the task, the bot will forcibly cancel the task."""
    return self._Request('cancel', method='POST')

  # /prpc/swarming.v2.Tasks/GetRequest
  def Request(self):
    """Returns the task request corresponding to a task ID."""
    return self._Request('request')

  # /prpc/swarming.v2.Tasks/GetResult
  def ResultV1(self, include_performance_stats=False):
    """Reports the result of the task corresponding to a task ID.

    It can be a 'run' ID specifying a specific retry or a 'summary' ID hiding
    the fact that a task may have been retried transparently, when a bot reports
    BOT_DIED. A summary ID ends with '0', a run ID ends with '1' or '2'."""
    if include_performance_stats:
      return self._Request('result', include_performance_stats=True)
    return self._Request('result')

  def Result(self, include_performance_stats=False):
    """Reports the result of the task corresponding to a task ID.

    It can be a 'run' ID specifying a specific retry or a 'summary' ID hiding
    the fact that a task may have been retried transparently, when a bot reports
    BOT_DIED. A summary ID ends with '0', a run ID ends with '1' or '2'."""
    url = '%s/%s.Tasks/GetResult' % (self._server, _V2_PRPC_PREFIX)
    request_body = {"taskId": self._task_id}
    if include_performance_stats:
      return request.RequestJson(
          url, method='POST', body=request_body, include_performance_stats=True)
    return request.RequestJson(url, method='POST', body=request_body)

  # /prpc/swarming.v2.Tasks/GetStdout
  def Stdout(self):
    """Returns the output of the task corresponding to a task ID."""
    return self._Request('stdout')

  def _Request(self, path, **kwargs):
    url = '%s/%s/task/%s/%s' % (self._server, _API_PATH, self._task_id, path)
    return request.RequestJson(url, **kwargs)


class Tasks:

  def __init__(self, server):
    self._server = server

  # /prpc/swarming.v2.Tasks/NewTask
  def NewV1(self, body):
    """Creates a new task.

    The task will be enqueued in the tasks list and will be executed at the
    earliest opportunity by a bot that has at least the dimensions as described
    in the task request.
    """
    url = '%s/%s/tasks/new' % (self._server, _API_PATH)
    return request.RequestJson(url, method='POST', body=body)

  def New(self, body):
    """Creates a new task.

    The task will be enqueued in the tasks list and will be executed at the
    earliest opportunity by a bot that has at least the dimensions as described
    in the task request.
    """
    url = '%s/%s.Tasks/NewTask' % (self._server, _V2_PRPC_PREFIX)
    return request.RequestJson(url, method='POST', body=body)

  # /prpc/swarming.v2.Tasks/CountTasks
  def CountV1(self, bot_id, state, pool):
    """Count the tasks queued on a bot

    Returns {'count': the number of tasks, 'now': the time of the query}
    """
    start_time = int(
        (datetime.datetime.now() - datetime.timedelta(days=7)).timestamp())
    query = 'start={}&state={}&tags=id%3A{}&tags=pool%3A{}'.format(
        start_time, state, bot_id, pool)
    url = '%s/%s/tasks/count?%s' % (self._server, _API_PATH, query)

    return request.RequestJson(url)

  def Count(self, bot_id, state, pool):
    """Count the tasks queued on a bot

    Returns {'count': the number of tasks, 'now': the time of the query}
    """
    start_time = (datetime.datetime.now() -
                  datetime.timedelta(days=7)).strftime("%Y-%m-%dT%H:%M:%SZ")
    request_body = {
        'start': start_time,
        'state': state,
        'tags': ['id:%s' % bot_id, 'pool:%s' % pool]
    }
    url = '%s/%s.Tasks/CountTasks' % (self._server, _V2_PRPC_PREFIX)

    return request.RequestJson(url, method='POST', body=request_body)


def _IsAliveV1(response):
  if response['is_dead'] or response['deleted']:
    return False
  if not response['quarantined']:
    return True
  return 'No available devices' not in str(response)


def _IsAlive(response):
  if response.get('isDead', False) or response.get('deleted', False):
    return False
  if not response.get('quarantined', False):
    return True
  return 'No available devices' not in str(response)


def GetAliveBotsByDimensionsV1(dimensions, swarming_server):
  # Queries Swarming for the set of bots we can use for this test.
  query_dimensions = {p['key']: p['value'] for p in dimensions}
  results = Swarming(swarming_server).Bots().ListV1(
      dimensions=query_dimensions, is_dead='FALSE', quarantined='FALSE')
  if 'items' in results:
    bots = [i['bot_id'] for i in results['items']]
    random.shuffle(bots)
    return bots
  return []


def GetAliveBotsByDimensions(dimensions, swarming_server):
  query_dimensions = {p['key']: p['value'] for p in dimensions}
  results = Swarming(swarming_server).Bots().List(
      dimensions=query_dimensions, is_dead='FALSE', quarantined='FALSE')
  if 'items' in results:
    bots = [i['botId'] for i in results['items']]
    random.shuffle(bots)
    return bots
  return []


def IsBotAliveV1(bot_id, swarming_server):
  result = Swarming(swarming_server).Bot(bot_id).GetV1()

  return _IsAliveV1(result)


def IsBotAlive(bot_id, swarming_server):
  result = Swarming(swarming_server).Bot(bot_id).Get()

  return _IsAlive(result)
