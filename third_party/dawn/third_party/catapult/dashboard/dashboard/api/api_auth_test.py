# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from unittest import mock
import unittest

from dashboard.api import api_auth
from dashboard.common import datastore_hooks
from dashboard.common import testing_common
from dashboard.common import utils


class ApiAuthTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()

    patcher = mock.patch.object(datastore_hooks, 'SetPrivilegedRequest')
    self.addCleanup(patcher.stop)
    self.mock_set_privileged_request = patcher.start()

  def testPost_NoUser(self):
    self.SetCurrentUserOAuth(None)
    with self.assertRaises(api_auth.NotLoggedInError):
      with self.PatchEnviron('/api/fake'):
        api_auth.Authorize()
    self.assertFalse(self.mock_set_privileged_request.called)

  def testPost_OAuthUser(self):
    self.SetCurrentUserOAuth(testing_common.INTERNAL_USER)
    self.SetCurrentClientIdOAuth(api_auth.OAUTH_CLIENT_ID_ALLOWLIST[0])
    with self.PatchEnviron('/api/fake'):
      api_auth.Authorize()
    self.assertTrue(self.mock_set_privileged_request.called)

  def testPost_OAuthUser_ServiceAccount(self):
    self.SetCurrentUserOAuth(testing_common.SERVICE_ACCOUNT_USER)
    testing_common.SetIsInternalUser(testing_common.SERVICE_ACCOUNT_USER, True)
    with self.PatchEnviron('/api/fake'):
      api_auth.Authorize()
    self.assertTrue(self.mock_set_privileged_request.called)

  def testPost_OAuthUser_ServiceAccount_NotInChromeperfAccess(self):
    self.SetCurrentUserOAuth(testing_common.SERVICE_ACCOUNT_USER)
    with self.PatchEnviron('/api/fake'):
      api_auth.Authorize()
    self.assertFalse(self.mock_set_privileged_request.called)

  def testPost_AuthorizedUser_NotInAllowlist(self):
    self.SetCurrentUserOAuth(testing_common.INTERNAL_USER)
    with self.assertRaises(api_auth.OAuthError):
      with self.PatchEnviron('/api/fake'):
        api_auth.Authorize()
    self.assertFalse(self.mock_set_privileged_request.called)

  def testPost_OAuthUser_User_NotInChromeperfAccess(self):
    self.SetCurrentUserOAuth(testing_common.EXTERNAL_USER)
    self.SetCurrentClientIdOAuth(api_auth.OAUTH_CLIENT_ID_ALLOWLIST[0])
    with self.PatchEnviron('/api/fake'):
      api_auth.Authorize()
    self.assertFalse(self.mock_set_privileged_request.called)

  def testPost_OAuthUser_User_InChromeperfAccess(self):
    self.SetCurrentUserOAuth(testing_common.INTERNAL_USER)
    self.SetCurrentClientIdOAuth(api_auth.OAUTH_CLIENT_ID_ALLOWLIST[0])
    with self.PatchEnviron('/api/fake'):
      api_auth.Authorize()
    self.assertTrue(self.mock_set_privileged_request.called)

  def testPost_OauthUser_Unauthorized(self):
    self.SetCurrentUserOAuth(testing_common.EXTERNAL_USER)
    with self.assertRaises(api_auth.OAuthError):
      with self.PatchEnviron('/api/fake'):
        api_auth.Authorize()
    self.assertFalse(self.mock_set_privileged_request.called)

  def testEmail(self):
    self.SetCurrentUserOAuth(testing_common.EXTERNAL_USER)
    with self.PatchEnviron('/api/fake'):
      self.assertEqual(utils.GetEmail(), testing_common.EXTERNAL_USER.email())

  def testEmail_NoUser(self):
    self.SetCurrentUserOAuth(None)
    with self.PatchEnviron('/api/fake'):
      self.assertIsNone(utils.GetEmail())


if __name__ == '__main__':
  unittest.main()
