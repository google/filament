# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from unittest import mock

from dashboard.common import datastore_hooks
from dashboard.services import pinpoint_service


class PinpointServiceTest(unittest.TestCase):

  def setUp(self):
    super().setUp()

    patcher = mock.patch.object(datastore_hooks, 'IsUnalteredQueryPermitted')
    self.addCleanup(patcher.stop)
    self.mock_hooks = patcher.start()

    patcher = mock.patch('dashboard.services.request.RequestJson')
    self.mock_request = patcher.start()
    self.addCleanup(patcher.stop)

  def testNewJob(self):
    self.mock_request.request.return_value = {}
    self.mock_hooks.return_value = True

    pinpoint_service.NewJob({'foo': 'bar'})

    self.mock_request.assert_called_with(
        pinpoint_service._PINPOINT_URL + '/api/new',
        foo='bar',
        use_cache=False,
        use_auth=True,
        method='POST')

  def testRequest_Unprivileged_Asserts(self):
    self.mock_hooks.return_value = False

    with self.assertRaises(AssertionError):
      pinpoint_service.NewJob({})
