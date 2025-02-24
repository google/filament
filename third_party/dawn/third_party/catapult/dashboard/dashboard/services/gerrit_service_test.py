# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from unittest import mock

from dashboard.services import gerrit_service


class _SwarmingTest(unittest.TestCase):

  def setUp(self):
    patcher = mock.patch('dashboard.services.request.RequestJson')
    self._request_json = patcher.start()
    self.addCleanup(patcher.stop)
    patcher2 = mock.patch('dashboard.services.request.Request')
    self._request = patcher2.start()
    self.addCleanup(patcher2.stop)

    self._request_json.return_value = {'content': {}}

  def _AssertCorrectResponse(self, content):
    self.assertEqual(content, {'content': {}})

  def _AssertRequestMadeOnce(self, url, *args, **kwargs):
    self._request_json.assert_called_once_with(url, *args, **kwargs)

  def testGetChange(self):
    server = 'https://chromium-review.googlesource.com'
    response = gerrit_service.GetChange(server, 672011)
    self._AssertCorrectResponse(response)
    self._AssertRequestMadeOnce(
        server + '/changes/672011',
        use_auth=True,
        scope=gerrit_service.GERRIT_SCOPE,
        o=None)

  def testGetChangeWithFields(self):
    server = 'https://chromium-review.googlesource.com'
    response = gerrit_service.GetChange(server, 672011, fields=('FIELD_NAME',))
    self._AssertCorrectResponse(response)
    self._AssertRequestMadeOnce(
        server + '/changes/672011',
        use_auth=True,
        scope=gerrit_service.GERRIT_SCOPE,
        o=('FIELD_NAME',))

  def testPostChangeComment(self):
    server = 'https://chromium-review.googlesource.com'
    gerrit_service.PostChangeComment(server, 12334, 'hello!')
    self._request.assert_called_once_with(
        'https://chromium-review.googlesource.com/a/changes/12334'
        '/revisions/current/review',
        body='hello!',
        scope=gerrit_service.GERRIT_SCOPE,
        use_cache=False,
        method='POST',
        use_auth=True)
