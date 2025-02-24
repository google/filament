# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import Flask
import json
import unittest

from dashboard import update_test_suite_descriptors
from dashboard.api import api_auth
from dashboard.api import describe
from dashboard.common import namespaced_stored_object
from dashboard.common import testing_common
from dashboard.common import utils

TEST_SUITE_NAME = 'test_suite'

flask_app = Flask(__name__)


@flask_app.route(r'/api/describe', methods=['POST', 'OPTIONS'])
def DescribeHandlerPost():
  return describe.DescribePost()


class DescribeTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.SetUpFlaskApp(flask_app)
    self.SetCurrentClientIdOAuth(api_auth.OAUTH_CLIENT_ID_ALLOWLIST[0])

    external_key = update_test_suite_descriptors.CacheKey(
        'external', TEST_SUITE_NAME)
    namespaced_stored_object.SetExternal(
        external_key, {
            'measurements': ['measurement'],
            'bots': ['external:bot'],
            'cases': ['case'],
            'caseTags': {}
        })

    testing_common.AddTests(['external'], ['bot'],
                            {'test_suite': {
                                'measurement': {
                                    'case': {},
                                }
                            }})

    self.SetCurrentUserOAuth(testing_common.INTERNAL_USER)
    self.SetCurrentUser(testing_common.INTERNAL_USER.email())
    testing_common.AddTests(['internal'], ['bot'],
                            {'test_suite': {
                                'measurement': {
                                    'case': {},
                                }
                            }})
    t = utils.TestKey('internal/bot/test_suite').get()
    t.internal_only = True
    t.put()

    namespaced_stored_object.Set(
        external_key, {
            'measurements': ['measurement'],
            'bots': ['external:bot'],
            'cases': ['case'],
            'caseTags': {}
        })

    internal_key = update_test_suite_descriptors.CacheKey(
        'internal', TEST_SUITE_NAME)
    namespaced_stored_object.Set(
        internal_key, {
            'measurements': ['measurement'],
            'bots': ['internal:bot'],
            'cases': ['case'],
            'caseTags': {}
        })

  def _Post(self, suite):
    return json.loads(self.Post('/api/describe', {'test_suite': suite}).body)

  def testInternal(self):
    self.SetCurrentUserOAuth(testing_common.INTERNAL_USER)
    response = self._Post(TEST_SUITE_NAME)
    self.assertEqual(2, len(response['bots']))
    self.assertEqual('external:bot', response['bots'][0])
    self.assertEqual('internal:bot', response['bots'][1])

  def testAnonymous(self):
    self.SetCurrentUserOAuth(None)
    response = self._Post(TEST_SUITE_NAME)
    self.assertEqual(1, len(response['bots']))
    self.assertEqual('external:bot', response['bots'][0])



if __name__ == '__main__':
  unittest.main()
