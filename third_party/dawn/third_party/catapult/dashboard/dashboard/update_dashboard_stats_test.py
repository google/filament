# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
from flask import Flask
from six.moves import http_client
from unittest import mock
import unittest
import webtest

from google.appengine.ext import ndb

from dashboard import update_dashboard_stats
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.services import gerrit_service
from dashboard.pinpoint.models.change import change as change_module
from dashboard.pinpoint.models.change import commit
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import job_state
from dashboard.pinpoint.models.quest import execution_test
from dashboard.pinpoint.models.quest import quest
from dashboard.pinpoint import test

_RESULTS_BY_CHANGE = {
    'chromium@aaaaaaa': [1, 1, 1, 1],
    'chromium@bbbbbbb': [5, 5, 5, 5]
}

flask_app = Flask(__name__)


@flask_app.route('/update_dashboard_stats')
def UpdateDashboardStatsGet():
  return update_dashboard_stats.UpdateDashboardStatsGet()


class _QuestStub(quest.Quest):

  def __str__(self):
    return 'Quest'

  def Start(self, change):
    return ExecutionResults(change)

  @classmethod
  def FromDict(cls, arguments):
    return cls


class ExecutionResults(execution_test._ExecutionStub):

  def __init__(self, c):
    super().__init__()
    self._result_for_test = _RESULTS_BY_CHANGE[str(c)]

  def _Poll(self):
    self._Complete(
        result_arguments={'arg key': 'arg value'},
        result_values=self._result_for_test)


def _StubFunc(*args, **kwargs):
  del args
  del kwargs


@ndb.tasklet
def _FakeTasklet(*args):
  del args


class UpdateDashboardStatsTest(test.TestCase):

  def setUp(self):
    super().setUp()
    self.testapp = webtest.TestApp(flask_app)

  def _CreateJob(self,
                 hash1,
                 hash2,
                 comparison_mode,
                 created,
                 bug_id,
                 exception=None,
                 arguments=None):
    old_commit = commit.Commit('chromium', hash1)
    change_a = change_module.Change((old_commit,))

    old_commit = commit.Commit('chromium', hash2)
    change_b = change_module.Change((old_commit,))

    with mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
                    mock.MagicMock(return_value=["a"])):
      job = job_module.Job.New((_QuestStub(),), (change_a, change_b),
                               comparison_mode=comparison_mode,
                               bug_id=bug_id,
                               arguments=arguments)
      job.created = created
      job.exception = exception
      job.state.ScheduleWork()
      job.state.Explore()
      job.put()
      return job

  @mock.patch.object(update_dashboard_stats, '_ProcessPinpointJobs',
                     mock.MagicMock(side_effect=_FakeTasklet))
  @mock.patch.object(update_dashboard_stats.deferred, 'defer')
  def testPost_ProcessAlerts_Success(self, mock_defer):
    created = datetime.datetime.now() - datetime.timedelta(hours=1)
    # sheriff = ndb.Key('Sheriff', 'Chromium Perf Sheriff')
    anomaly_entity = anomaly.Anomaly(
        test=utils.TestKey('M/B/suite'), timestamp=created)  #, sheriff=sheriff)
    anomaly_entity.put()

    self.testapp.get('/update_dashboard_stats')
    self.assertTrue(mock_defer.called)

  @mock.patch.object(update_dashboard_stats, '_ProcessPinpointJobs', _StubFunc)
  @mock.patch.object(update_dashboard_stats, '_ProcessPinpointStats', _StubFunc)
  def testPost_ProcessAlerts_NoAlerts(self):
    created = datetime.datetime.now() - datetime.timedelta(days=2)
    # sheriff = ndb.Key('Sheriff', 'Chromium Perf Sheriff')
    anomaly_entity = anomaly.Anomaly(
        test=utils.TestKey('M/B/suite'),
        timestamp=created)  # , sheriff=sheriff)
    anomaly_entity.put()

    self.testapp.get('/update_dashboard_stats')

    self.ExecuteDeferredTasks('default', recurse=False)

    patcher = mock.patch('update_dashboard_stats.deferred.defer')
    self.addCleanup(patcher.stop)
    mock_defer = patcher.start()
    self.assertFalse(mock_defer.called)

  @mock.patch.object(update_dashboard_stats, '_ProcessAlerts', _StubFunc)
  @mock.patch.object(change_module.Change, 'Midpoint',
                     mock.MagicMock(side_effect=commit.NonLinearError))
  @mock.patch.object(update_dashboard_stats, '_ProcessPinpointJobs', _StubFunc)
  def testPost_ProcessPinpointStats_Success(self):
    created = datetime.datetime.now() - datetime.timedelta(hours=12)
    j = self._CreateJob(
        'aaaaaaaa',
        'bbbbbbbb',
        job_state.PERFORMANCE,
        created,
        12345,
        arguments={
            'configuration': 'bot1',
            'benchmark': 'suite1'
        })
    j.updated = created + datetime.timedelta(hours=1)
    j.put()

    created = datetime.datetime.now() - datetime.timedelta(hours=12)
    j = self._CreateJob(
        'aaaaaaaa',
        'bbbbbbbb',
        job_state.PERFORMANCE,
        created,
        12345,
        arguments={
            'configuration': 'bot2',
            'benchmark': 'suite2'
        })
    j.updated = created + datetime.timedelta(hours=1)
    j.put()

    self.testapp.get('/update_dashboard_stats')

    patcher = mock.patch('update_dashboard_stats.deferred.defer')
    self.addCleanup(patcher.stop)
    mock_defer = patcher.start()

    self.ExecuteDeferredTasks('default', recurse=False)

    self.assertTrue(mock_defer.called)

  @mock.patch.object(update_dashboard_stats, '_ProcessAlerts',
                     mock.MagicMock(side_effect=_FakeTasklet))
  @mock.patch.object(update_dashboard_stats, '_ProcessPinpointStats',
                     mock.MagicMock(side_effect=_FakeTasklet))
  @mock.patch.object(change_module.Change, 'Midpoint',
                     mock.MagicMock(side_effect=commit.NonLinearError))
  @mock.patch.object(update_dashboard_stats.deferred, 'defer')
  def testPost_ProcessPinpoint_Success(self, mock_defer):
    created = datetime.datetime.now() - datetime.timedelta(days=1)
    self._CreateJob('aaaaaaaa', 'bbbbbbbb', job_state.PERFORMANCE, created,
                    12345)
    anomaly_entity = anomaly.Anomaly(
        test=utils.TestKey('M/B/S'), bug_id=12345, timestamp=created)
    anomaly_entity.put()

    self.testapp.get('/update_dashboard_stats')
    self.assertTrue(mock_defer.called)

  @mock.patch.object(gerrit_service, 'GetChange',
                     mock.MagicMock(side_effect=http_client.HTTPException))
  @mock.patch.object(update_dashboard_stats, '_ProcessAlerts', _StubFunc)
  @mock.patch.object(update_dashboard_stats, '_ProcessPinpointStats', _StubFunc)
  @mock.patch.object(change_module.Change, 'Midpoint',
                     mock.MagicMock(side_effect=commit.NonLinearError))
  def testPost_ProcessPinpoint_NoResults(self):
    created = datetime.datetime.now() - datetime.timedelta(days=1)

    anomaly_entity = anomaly.Anomaly(
        test=utils.TestKey('M/B/S'), bug_id=12345, timestamp=created)
    anomaly_entity.put()

    self._CreateJob('aaaaaaaa', 'bbbbbbbb', job_state.FUNCTIONAL, created,
                    12345)

    created = datetime.datetime.now() - datetime.timedelta(days=15)
    self._CreateJob('aaaaaaaa', 'bbbbbbbb', job_state.PERFORMANCE, created,
                    12345)

    created = datetime.datetime.now() - datetime.timedelta(days=1)
    self._CreateJob('aaaaaaaa', 'bbbbbbbb', job_state.PERFORMANCE, created,
                    None)

    created = datetime.datetime.now() - datetime.timedelta(days=1)
    self._CreateJob('aaaaaaaa', 'aaaaaaaa', job_state.PERFORMANCE, created,
                    12345)

    created = datetime.datetime.now() - datetime.timedelta(days=1)
    self._CreateJob('aaaaaaaa', 'bbbbbbbb', job_state.PERFORMANCE, created,
                    12345, 'foo')

    way_too_old = datetime.datetime(year=2000, month=1, day=1)
    anomaly_entity = anomaly.Anomaly(
        test=utils.TestKey('M/B/S'), bug_id=1, timestamp=way_too_old)
    anomaly_entity.put()

    created = datetime.datetime.now() - datetime.timedelta(days=1)
    self._CreateJob('aaaaaaaa', 'bbbbbbbb', job_state.PERFORMANCE, created, 1)

    self.testapp.get('/update_dashboard_stats')

    patcher = mock.patch('update_dashboard_stats.deferred.defer')
    self.addCleanup(patcher.stop)
    mock_defer = patcher.start()
    self.assertFalse(mock_defer.called)


if __name__ == '__main__':
  unittest.main()
