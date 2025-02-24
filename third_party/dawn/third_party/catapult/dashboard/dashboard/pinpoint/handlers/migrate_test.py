# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json

from unittest import mock

from dashboard.api import api_auth
from dashboard.common import stored_object
from dashboard.common import testing_common
from dashboard.pinpoint.handlers import migrate
from dashboard.pinpoint.models import job
from dashboard.pinpoint.models import job_state
from dashboard.pinpoint import test


class MigrateAuthTest(test.TestCase):

  def setUp(self):
    super().setUp()

    patcher = mock.patch.object(migrate, 'datetime', _DatetimeStub())
    self.addCleanup(patcher.stop)
    patcher.start()

    with mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
                    mock.MagicMock(return_value=["a"])):
      for _ in range(100):
        job.Job.New((), ())

  def _SetupCredentials(self, user, client_id, is_internal, is_admin):
    email = user.email()
    testing_common.SetIsInternalUser(email, is_internal)
    testing_common.SetIsAdministrator(email, is_admin)
    self.SetCurrentUserOAuth(user)
    self.SetCurrentClientIdOAuth(client_id)
    self.SetCurrentUser(email)

  def testGet_ExternalUser_Fails(self):
    self._SetupCredentials(testing_common.EXTERNAL_USER, None, False, False)

    self.Get('/api/migrate', status=403)

  def testGet_InternalUser_NotAdmin_Fails(self):
    self._SetupCredentials(testing_common.INTERNAL_USER,
                           api_auth.OAUTH_CLIENT_ID_ALLOWLIST[0], True, False)

    self.Get('/api/migrate', status=403)


class MigrateTest(MigrateAuthTest):

  def setUp(self):
    super().setUp()

    print('MigrateTest')
    self._SetupCredentials(testing_common.INTERNAL_USER,
                           api_auth.OAUTH_CLIENT_ID_ALLOWLIST[0], True, True)

  def testGet_NoMigration(self):
    response = self.Get('/api/migrate', status=200)
    self.assertEqual(response.normal_body, b'{}')

  def testGet_MigrationInProgress(self):
    expected = {
        'count': 0,
        'started': 'Date Time',
        'total': 100,
        'errors': 0,
    }

    response = self.Post('/api/migrate', status=200)
    self.assertEqual(response.normal_body, json.dumps(expected).encode('utf-8'))

    response = self.Get('/api/migrate', status=200)
    self.assertEqual(response.normal_body, json.dumps(expected).encode('utf-8'))

  def testPost_EndToEnd(self):
    expected = {
        'count': 0,
        'started': 'Date Time',
        'total': 100,
        'errors': 0,
    }

    job_state.JobState.__setstate__ = _JobStateSetState

    response = self.Post('/api/migrate', status=200)
    self.assertEqual(response.normal_body, json.dumps(expected).encode('utf-8'))

    expected = {
        'count': 50,
        'started': 'Date Time',
        'total': 100,
        'errors': 0,
    }

    self.ExecuteDeferredTasks('default', recurse=False)
    status = stored_object.Get(migrate._STATUS_KEY)
    self.assertEqual(status, expected)

    self.ExecuteDeferredTasks('default', recurse=False)
    status = stored_object.Get(migrate._STATUS_KEY)
    self.assertEqual(status, None)

    del job_state.JobState.__setstate__

    jobs = job.Job.query().fetch()
    for j in jobs:
      self.assertEqual(j.state._new_field, 'new value')


def _JobStateSetState(self, state):
  self.__dict__ = state
  self._new_field = 'new value'


class _DatetimeStub:

  # pylint: disable=invalid-name
  class datetime:

    def isoformat(self):
      return 'Date Time'

    @classmethod
    def now(cls):
      return cls()
