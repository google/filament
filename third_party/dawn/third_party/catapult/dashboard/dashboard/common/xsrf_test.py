# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest
import webtest

from flask import Flask
from google.appengine.api import users

from dashboard.common import testing_common
from dashboard.common import xsrf

flask_app = Flask(__name__)


@flask_app.route('/example', methods=['POST'])
@xsrf.TokenRequired
def PostFlask():
  return '<p>HTML</p>'


class XsrfTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.flask_testapp = webtest.TestApp(flask_app)

  def testGenerateToken_CanBeValidatedWithSameUser(self):
    self.SetCurrentUser('foo@bar.com')
    token = xsrf.GenerateToken(users.get_current_user())
    self.assertTrue(xsrf._ValidateToken(token, users.get_current_user()))

  def testGenerateToken_CanNotBeValidatedWithDifferentUser(self):
    self.SetCurrentUser('foo@bar.com', user_id='x')
    token = xsrf.GenerateToken(users.get_current_user())
    self.SetCurrentUser('foo@other.com', user_id='y')
    self.assertFalse(xsrf._ValidateToken(token, users.get_current_user()))


  def testTokenRequired_NoToken_Returns403(self):
    self.flask_testapp.post('/example', {}, status=403)

  def testTokenRequired_BogusToken_Returns403(self):
    self.flask_testapp.post(
        '/example', {'xsrf_token': 'abcdefghijklmnopqrstuvwxyz0123456789'},
        status=403)

  def testTokenRequired_CorrectToken_Success(self):
    self.SetCurrentUser('foo@bar.com')
    token = xsrf.GenerateToken(users.get_current_user())
    self.flask_testapp.post('/example', {'xsrf_token': token})


if __name__ == '__main__':
  unittest.main()
