# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json

from dashboard.common import namespaced_stored_object
from dashboard.common import testing_common
from dashboard.pinpoint import test


class ConfigTest(test.TestCase):

  def setUp(self):
    super().setUp()

    self.SetCurrentUser(testing_common.EXTERNAL_USER.email())
    self.SetCurrentUserOAuth(testing_common.EXTERNAL_USER)

    namespaced_stored_object.Set('bot_configurations', {
        'chromium-rel-mac11-pro': {},
    })

    self.SetCurrentUser(testing_common.INTERNAL_USER.email(), is_admin=True)
    testing_common.SetIsInternalUser(testing_common.INTERNAL_USER.email(), True)
    self.SetCurrentUserOAuth(testing_common.INTERNAL_USER)

    namespaced_stored_object.Set('bot_configurations', {
        'internal-only-bot': {},
    })

  def testGet_External(self):
    self.SetCurrentUser(testing_common.EXTERNAL_USER.email())
    self.SetCurrentUserOAuth(testing_common.EXTERNAL_USER)

    actual = json.loads(self.Post('/api/config').body)
    expected = {
        'configurations': ['chromium-rel-mac11-pro'],
    }
    self.assertEqual(actual, expected)

  def testGet_Internal(self):
    self.SetCurrentUser(testing_common.INTERNAL_USER.email())
    self.SetCurrentUserOAuth(testing_common.INTERNAL_USER)

    actual = json.loads(self.Post('/api/config').body)
    expected = {
        'configurations': ['internal-only-bot'],
    }
    self.assertEqual(actual, expected)
