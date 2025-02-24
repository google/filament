# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import Flask
import json
import unittest

from dashboard import update_test_suites
from dashboard.api import api_auth
from dashboard.api import test_suites
from dashboard.common import datastore_hooks
from dashboard.common import namespaced_stored_object
from dashboard.common import stored_object
from dashboard.common import testing_common

flask_app = Flask(__name__)


@flask_app.route('/api/test_suites', methods=['POST', 'OPTIONS'])
def TestSuitesPost():
  return test_suites.TestSuitesPost()


class TestSuitesTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.SetUpFlaskApp(flask_app)
    self.SetCurrentClientIdOAuth(api_auth.OAUTH_CLIENT_ID_ALLOWLIST[0])
    external_key = namespaced_stored_object.NamespaceKey(
        update_test_suites.TEST_SUITES_2_CACHE_KEY, datastore_hooks.EXTERNAL)
    stored_object.Set(external_key, ['external'])
    internal_key = namespaced_stored_object.NamespaceKey(
        update_test_suites.TEST_SUITES_2_CACHE_KEY, datastore_hooks.INTERNAL)
    stored_object.Set(internal_key, ['external', 'internal'])

  def testInternal(self):
    self.SetCurrentUserOAuth(testing_common.INTERNAL_USER)
    response = json.loads(self.Post('/api/test_suites').body)
    self.assertEqual(2, len(response))
    self.assertEqual('external', response[0])
    self.assertEqual('internal', response[1])

  def testExternal(self):
    self.SetCurrentUserOAuth(testing_common.EXTERNAL_USER)
    response = json.loads(self.Post('/api/test_suites').body)
    self.assertEqual(1, len(response))
    self.assertEqual('external', response[0])

  def testAnonymous(self):
    self.SetCurrentUserOAuth(None)
    response = json.loads(self.Post('/api/test_suites').body)
    self.assertEqual(1, len(response))
    self.assertEqual('external', response[0])


if __name__ == '__main__':
  unittest.main()
