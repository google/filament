# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import Flask
import json
import unittest

from dashboard.api import api_auth
from dashboard.api import config
from dashboard.common import datastore_hooks
from dashboard.common import namespaced_stored_object
from dashboard.common import stored_object
from dashboard.common import testing_common

flask_app = Flask(__name__)


@flask_app.route(r'/api/config', methods=['POST'])
def ConfigHandlerPost():
  return config.ConfigHandlerPost()


class ConfigTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.SetUpFlaskApp(flask_app)
    self.SetCurrentClientIdOAuth(api_auth.OAUTH_CLIENT_ID_ALLOWLIST[0])
    external_key = namespaced_stored_object.NamespaceKey(
        config.ALLOWLIST[0], datastore_hooks.EXTERNAL)
    stored_object.Set(external_key, datastore_hooks.EXTERNAL)
    internal_key = namespaced_stored_object.NamespaceKey(
        config.ALLOWLIST[0], datastore_hooks.INTERNAL)
    stored_object.Set(internal_key, datastore_hooks.INTERNAL)

  def _Post(self, suite):
    return json.loads(self.Post('/api/config', {'key': suite}).body)

  def testNotInAllowlist(self):
    self.SetCurrentUserOAuth(testing_common.INTERNAL_USER)
    response = self._Post('disallowed')
    self.assertEqual(None, response)

  def testInternal(self):
    self.SetCurrentUserOAuth(testing_common.INTERNAL_USER)
    response = self._Post(config.ALLOWLIST[0])
    self.assertEqual(datastore_hooks.INTERNAL, response)

  def testAnonymous(self):
    self.SetCurrentUserOAuth(None)
    response = self._Post(config.ALLOWLIST[0])
    self.assertEqual(datastore_hooks.EXTERNAL, response)


if __name__ == '__main__':
  unittest.main()
