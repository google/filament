#!/usr/bin/env vpython3
# Copyright (c) 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit Tests for auth.py"""

import calendar
import datetime
import json
import os
import unittest
import sys
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import auth
import subprocess2

NOW = datetime.datetime(2019, 10, 17, 12, 30, 59, 0)
VALID_EXPIRY = NOW + datetime.timedelta(seconds=31)


class AuthenticatorTest(unittest.TestCase):
    def setUp(self):
        mock.patch('subprocess2.check_call').start()
        mock.patch('subprocess2.check_call_out').start()
        mock.patch('auth.datetime_now', return_value=NOW).start()
        self.addCleanup(mock.patch.stopall)

    def testHasCachedCredentials_NotLoggedIn(self):
        subprocess2.check_call_out.side_effect = [
            subprocess2.CalledProcessError(1, ['cmd'], 'cwd', 'stdout',
                                           'stderr')
        ]
        self.assertFalse(auth.Authenticator().has_cached_credentials())

    def testHasCachedCredentials_LoggedIn(self):
        subprocess2.check_call_out.return_value = (json.dumps({
            'token': 'token',
            'expiry': 12345678
        }), '')
        self.assertTrue(auth.Authenticator().has_cached_credentials())

    def testGetAccessToken_NotLoggedIn(self):
        subprocess2.check_call_out.side_effect = [
            subprocess2.CalledProcessError(1, ['cmd'], 'cwd', 'stdout',
                                           'stderr')
        ]
        self.assertRaises(auth.LoginRequiredError,
                          auth.Authenticator().get_access_token)

    def testGetAccessToken_CachedToken(self):
        authenticator = auth.Authenticator()
        authenticator._access_token = auth.Token('token', None)
        self.assertEqual(auth.Token('token', None),
                         authenticator.get_access_token())
        subprocess2.check_call_out.assert_not_called()

    def testGetAccesstoken_LoggedIn(self):
        expiry = calendar.timegm(VALID_EXPIRY.timetuple())
        subprocess2.check_call_out.return_value = (json.dumps({
            'token': 'token',
            'expiry': expiry
        }), '')
        self.assertEqual(auth.Token('token', VALID_EXPIRY),
                         auth.Authenticator().get_access_token())
        subprocess2.check_call_out.assert_called_with([
            'luci-auth', 'token', '-scopes', auth.OAUTH_SCOPE_EMAIL,
            '-json-output', '-'
        ],
                                                      stdout=subprocess2.PIPE,
                                                      stderr=subprocess2.PIPE)

    def testGetAccessToken_DifferentScope(self):
        expiry = calendar.timegm(VALID_EXPIRY.timetuple())
        subprocess2.check_call_out.return_value = (json.dumps({
            'token': 'token',
            'expiry': expiry
        }), '')
        self.assertEqual(auth.Token('token', VALID_EXPIRY),
                         auth.Authenticator('custom scopes').get_access_token())
        subprocess2.check_call_out.assert_called_with([
            'luci-auth', 'token', '-scopes', 'custom scopes', '-json-output',
            '-'
        ],
                                                      stdout=subprocess2.PIPE,
                                                      stderr=subprocess2.PIPE)

    def testAuthorize_AccessToken(self):
        http = mock.Mock()
        http_request = http.request
        http_request.__name__ = '__name__'

        authenticator = auth.Authenticator()
        authenticator._access_token = auth.Token('access_token', None)
        authenticator._id_token = auth.Token('id_token', None)

        authorized = authenticator.authorize(http)
        authorized.request('https://example.com',
                           method='POST',
                           body='body',
                           headers={'header': 'value'})
        http_request.assert_called_once_with(
            'https://example.com', 'POST', 'body', {
                'header': 'value',
                'Authorization': 'Bearer access_token'
            }, mock.ANY, mock.ANY)

    def testGetIdToken_NotLoggedIn(self):
        subprocess2.check_call_out.side_effect = [
            subprocess2.CalledProcessError(1, ['cmd'], 'cwd', 'stdout',
                                           'stderr')
        ]
        self.assertRaises(auth.LoginRequiredError,
                          auth.Authenticator().get_id_token)

    def testGetIdToken_CachedToken(self):
        authenticator = auth.Authenticator()
        authenticator._id_token = auth.Token('token', None)
        self.assertEqual(auth.Token('token', None),
                         authenticator.get_id_token())
        subprocess2.check_call_out.assert_not_called()

    def testGetIdToken_LoggedIn(self):
        expiry = calendar.timegm(VALID_EXPIRY.timetuple())
        subprocess2.check_call_out.return_value = (json.dumps({
            'token': 'token',
            'expiry': expiry
        }), '')
        self.assertEqual(
            auth.Token('token', VALID_EXPIRY),
            auth.Authenticator(audience='https://test.com').get_id_token())
        subprocess2.check_call_out.assert_called_with([
            'luci-auth', 'token', '-use-id-token', '-audience',
            'https://test.com', '-json-output', '-'
        ],
                                                      stdout=subprocess2.PIPE,
                                                      stderr=subprocess2.PIPE)

    def testAuthorize_IdToken(self):
        http = mock.Mock()
        http_request = http.request
        http_request.__name__ = '__name__'

        authenticator = auth.Authenticator()
        authenticator._access_token = auth.Token('access_token', None)
        authenticator._id_token = auth.Token('id_token', None)

        authorized = authenticator.authorize(http, use_id_token=True)
        authorized.request('https://example.com',
                           method='POST',
                           body='body',
                           headers={'header': 'value'})
        http_request.assert_called_once_with(
            'https://example.com', 'POST', 'body', {
                'header': 'value',
                'Authorization': 'Bearer id_token'
            }, mock.ANY, mock.ANY)


class TokenTest(unittest.TestCase):
    def setUp(self):
        mock.patch('auth.datetime_now', return_value=NOW).start()
        self.addCleanup(mock.patch.stopall)

    def testNeedsRefresh_NoExpiry(self):
        self.assertFalse(auth.Token('token', None).needs_refresh())

    def testNeedsRefresh_Expired(self):
        expired = NOW + datetime.timedelta(seconds=30)
        self.assertTrue(auth.Token('token', expired).needs_refresh())

    def testNeedsRefresh_Valid(self):
        self.assertFalse(auth.Token('token', VALID_EXPIRY).needs_refresh())


class HasLuciContextLocalAuthTest(unittest.TestCase):
    def setUp(self):
        mock.patch('os.environ').start()
        mock.patch('builtins.open', mock.mock_open()).start()
        self.addCleanup(mock.patch.stopall)

    def testNoLuciContextEnvVar(self):
        os.environ = {}
        self.assertFalse(auth.has_luci_context_local_auth())

    def testNonexistentPath(self):
        os.environ = {'LUCI_CONTEXT': 'path'}
        open.side_effect = OSError
        self.assertFalse(auth.has_luci_context_local_auth())
        open.assert_called_with('path')

    def testInvalidJsonFile(self):
        os.environ = {'LUCI_CONTEXT': 'path'}
        open().read.return_value = 'not-a-json-file'
        self.assertFalse(auth.has_luci_context_local_auth())
        open.assert_called_with('path')

    def testNoLocalAuth(self):
        os.environ = {'LUCI_CONTEXT': 'path'}
        open().read.return_value = '{}'
        self.assertFalse(auth.has_luci_context_local_auth())
        open.assert_called_with('path')

    def testNoDefaultAccountId(self):
        os.environ = {'LUCI_CONTEXT': 'path'}
        open().read.return_value = json.dumps({
            'local_auth': {
                'secret':
                'secret',
                'accounts': [{
                    'email': 'bots@account.iam.gserviceaccount.com',
                    'id': 'system',
                }],
                'rpc_port':
                1234,
            }
        })
        self.assertFalse(auth.has_luci_context_local_auth())
        open.assert_called_with('path')

    def testHasLocalAuth(self):
        os.environ = {'LUCI_CONTEXT': 'path'}
        open().read.return_value = json.dumps({
            'local_auth': {
                'secret':
                'secret',
                'accounts': [
                    {
                        'email': 'bots@account.iam.gserviceaccount.com',
                        'id': 'system',
                    },
                    {
                        'email': 'builder@account.iam.gserviceaccount.com',
                        'id': 'task',
                    },
                ],
                'rpc_port':
                1234,
                'default_account_id':
                'task',
            },
        })
        self.assertTrue(auth.has_luci_context_local_auth())
        open.assert_called_with('path')


if __name__ == '__main__':
    if '-v' in sys.argv:
        logging.basicConfig(level=logging.DEBUG)
    unittest.main()
