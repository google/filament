# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from unittest import mock

from dashboard.services import swarming


class _SwarmingTest(unittest.TestCase):

  def setUp(self):
    patcher = mock.patch('dashboard.services.request.RequestJson')
    self._request_json = patcher.start()
    self.addCleanup(patcher.stop)

    self._request_json.return_value = {'content': {}}

  def _AssertCorrectResponse(self, content):
    self.assertEqual(content, {'content': {}})

  def _AssertRequestMadeOnce(self, path, *args, **kwargs):
    self._request_json.assert_called_once_with(
        'https://server/_ah/api/swarming/v1/' + path,
        *args,
        **kwargs)

  def _AssertV2RequestMadeOnce(self, path, *args, **kwargs):
    self._request_json.assert_called_once_with(
        'https://server/prpc/swarming.v2.' + path, *args, **kwargs)


class BotTest(_SwarmingTest):

  def testGet(self):
    response = swarming.Swarming('https://server').Bot('bot-123').Get()
    body = {'botId': 'bot-123'}
    self._AssertCorrectResponse(response)
    self._AssertV2RequestMadeOnce('Bots/GetBot', method='POST', body=body)

  def testTasks(self):
    response = swarming.Swarming('https://server').Bot('bot-123').Tasks()
    self._AssertCorrectResponse(response)
    self._AssertRequestMadeOnce('bot/bot-123/tasks')


class BotsTest(_SwarmingTest):

  def testList(self):
    response = swarming.Swarming('https://server').Bots().List(
        'CkMSPWoQ', {
            'a': 'b',
            'pool': 'Chrome-perf',
        }, False, 1, True)
    self._AssertCorrectResponse(response)

    path = ('Bots/ListBots')
    body = {
        'cursor': 'CkMSPWoQ',
        'dimensions': [
            {
                'key': 'a',
                'value': 'b'
            },
            {
                'key': 'pool',
                'value': 'Chrome-perf'
            },
        ],
        'isDead': False,
        'limit': 1,
        'quarantined': True
    }
    self._AssertV2RequestMadeOnce(path, method='POST', body=body)


@mock.patch('dashboard.services.swarming.Bots.List')
class QueryBotsTest(unittest.TestCase):

  @mock.patch('random.shuffle')
  def testSingleBotReturned(self, random_shuffle, swarming_bots_list):
    swarming_bots_list.return_value = {'items': [{'botId': 'a'}]}
    self.assertEqual(
        swarming.GetAliveBotsByDimensions([{
            'key': 'k',
            'value': 'val'
        }], 'server'), ['a'])
    random_shuffle.assert_called_with(['a'])
    swarming_bots_list.assert_called_with(
        dimensions={'k': 'val'}, is_dead='FALSE', quarantined='FALSE')

  def testNoBotsReturned(self, swarming_bots_list):
    swarming_bots_list.return_value = {"success": "false"}
    self.assertEqual(
        swarming.GetAliveBotsByDimensions([{
            'key': 'k',
            'value': 'val'
        }], 'server'), [])


class IsBotAliveTest(unittest.TestCase):

  @mock.patch(
      'dashboard.services.swarming.Bot.Get',
      mock.MagicMock(return_value={
          'isDead': False,
          'deleted': False,
          'quarantined': False
      }))
  def testAlive(self):
    self.assertTrue(swarming.IsBotAlive('a', 'server'))

  @mock.patch(
      'dashboard.services.swarming.Bot.Get',
      mock.MagicMock(return_value={
          'isDead': True,
          'deleted': False,
          'quarantined': False
      }))
  def testDead(self):
    self.assertFalse(swarming.IsBotAlive('a', 'server'))

  @mock.patch(
      'dashboard.services.swarming.Bot.Get',
      mock.MagicMock(return_value={
          'isDead': False,
          'deleted': True,
          'quarantined': False
      }))
  def testDeleted(self):
    self.assertFalse(swarming.IsBotAlive('a', 'server'))

  @mock.patch(
      'dashboard.services.swarming.Bot.Get',
      mock.MagicMock(
          return_value={
              'isDead': False,
              'deleted': False,
              'quarantined': True,
              'state': 'device hot'
          }))
  def testQuarantinedTemp(self):
    self.assertTrue(swarming.IsBotAlive('a', 'server'))

  @mock.patch(
      'dashboard.services.swarming.Bot.Get',
      mock.MagicMock(
          return_value={
              'isDead': False,
              'deleted': False,
              'quarantined': True,
              'state': '"quarantined":"No available devices."'
          }))
  def testQuarantinedNotAvailable(self):
    self.assertFalse(swarming.IsBotAlive('a', 'server'))


class TaskTest(_SwarmingTest):

  def testCancel(self):
    response = swarming.Swarming('https://server').Task('task_id').Cancel()
    self._AssertCorrectResponse(response)
    self._AssertRequestMadeOnce('task/task_id/cancel', method='POST')

  def testRequest(self):
    response = swarming.Swarming('https://server').Task('task_id').Request()
    self._AssertCorrectResponse(response)
    self._AssertRequestMadeOnce('task/task_id/request')

  def testResult(self):
    response = swarming.Swarming('https://server').Task('task_id').Result()
    body = {'taskId': 'task_id'}
    self._AssertCorrectResponse(response)
    self._AssertV2RequestMadeOnce('Tasks/GetResult', method='POST', body=body)

  def testResultWithPerformanceStats(self):
    response = swarming.Swarming('https://server').Task('task_id').Result(True)
    body = {'taskId': 'task_id'}
    self._AssertCorrectResponse(response)
    self._AssertV2RequestMadeOnce(
        'Tasks/GetResult',
        method='POST',
        body=body,
        include_performance_stats=True)

  def testStdout(self):
    response = swarming.Swarming('https://server').Task('task_id').Stdout()
    self._AssertCorrectResponse(response)
    self._AssertRequestMadeOnce('task/task_id/stdout')


class TasksTest(_SwarmingTest):

  def testNew(self):
    body = {
        'name': 'name',
        'user': 'user',
        'priority': '100',
        'expirationSecs': '600',
        'properties': {
            'inputsRef': {
                'isolated': 'isolated_hash',
            },
            'extraArgs': ['--output-format=histograms'],
            'dimensions': [
                {
                    'key': 'id',
                    'value': 'bot_id'
                },
                {
                    'key': 'pool',
                    'value': 'Chrome-perf'
                },
            ],
            'executionTimeoutSecs': '3600',
            'ioTimeoutSecs': '3600',
        },
        'tags': [
            'id:bot_id',
            'pool:Chrome-perf',
        ],
    }

    response = swarming.Swarming('https://server').Tasks().New(body)
    self._AssertCorrectResponse(response)
    self._AssertV2RequestMadeOnce('Tasks/NewTask', method='POST', body=body)
