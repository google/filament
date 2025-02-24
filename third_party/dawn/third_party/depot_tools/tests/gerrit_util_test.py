#!/usr/bin/env vpython3
# coding=utf-8
# Copyright (c) 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import os
import socket
import subprocess
import sys
import textwrap
import unittest

from io import StringIO
from pathlib import Path
from typing import Optional
from unittest import mock

import httplib2

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import scm_mock

import gerrit_util
import metrics
import scm
import subprocess2

RUN_SUBPROC_TESTS = 'RUN_SUBPROC_TESTS' in os.environ


def makeConn(host: str) -> gerrit_util.HttpConn:
    """Makes an empty gerrit_util.HttpConn for the given host."""
    return gerrit_util.HttpConn(
        req_uri='???',
        req_method='GET',
        req_host=host,
        req_headers={},
        req_body=None,
    )


class CookiesAuthenticatorTest(unittest.TestCase):
    _GITCOOKIES = '\n'.join([
        '\t'.join([
            'chromium.googlesource.com',
            'FALSE',
            '/',
            'TRUE',
            '2147483647',
            'o',
            'git-user.chromium.org=1/chromium-secret',
        ]),
        '\t'.join([
            'chromium-review.googlesource.com',
            'FALSE',
            '/',
            'TRUE',
            '2147483647',
            'o',
            'git-user.chromium.org=1/chromium-secret',
        ]),
        '\t'.join([
            '.example.com',
            'FALSE',
            '/',
            'TRUE',
            '2147483647',
            'o',
            'example-bearer-token',
        ]),
        '\t'.join([
            'another-path.example.com',
            'FALSE',
            '/foo',
            'TRUE',
            '2147483647',
            'o',
            'git-example.com=1/another-path-secret',
        ]),
        '\t'.join([
            'another-key.example.com',
            'FALSE',
            '/',
            'TRUE',
            '2147483647',
            'not-o',
            'git-example.com=1/another-key-secret',
        ]),
        '#' + '\t'.join([
            'chromium-review.googlesource.com',
            'FALSE',
            '/',
            'TRUE',
            '2147483647',
            'o',
            'git-invalid-user.chromium.org=1/invalid-chromium-secret',
        ]),
        'Some unrelated line\t that should not be here',
    ])

    def setUp(self):
        mock.patch('gclient_utils.FileRead',
                   return_value=self._GITCOOKIES).start()
        mock.patch('os.getenv', return_value={}).start()
        mock.patch('os.environ', {'HOME': '$HOME'}).start()
        mock.patch('os.getcwd', return_value='/fame/cwd').start()
        mock.patch('os.path.exists', return_value=True).start()
        mock.patch(
            'git_common.run',
            side_effect=[
                subprocess2.CalledProcessError(1, ['cmd'], 'cwd', 'out', 'err')
            ],
        ).start()
        scm_mock.GIT(self)

        self.addCleanup(mock.patch.stopall)
        self.maxDiff = None

    def assertAuthenticatedConnAuth(self,
                                    auth: gerrit_util.CookiesAuthenticator,
                                    host: str, expected: str):
        conn = makeConn(host)
        auth.authenticate(conn)
        self.assertEqual(conn.req_headers['Authorization'], expected)

    def testGetNewPasswordUrl(self):
        auth = gerrit_util.CookiesAuthenticator()
        self.assertEqual('https://chromium.googlesource.com/new-password',
                         auth.get_new_password_url('chromium.googlesource.com'))
        self.assertEqual(
            'https://chrome-internal.googlesource.com/new-password',
            auth.get_new_password_url(
                'chrome-internal-review.googlesource.com'))

    def testGetNewPasswordMessage(self):
        auth = gerrit_util.CookiesAuthenticator()
        self.assertIn(
            'https://chromium.googlesource.com/new-password',
            auth._get_new_password_message('chromium-review.googlesource.com'))
        self.assertIn(
            'https://chrome-internal.googlesource.com/new-password',
            auth._get_new_password_message('chrome-internal.googlesource.com'))

    def testGetGitcookiesPath(self):
        self.assertEqual(
            os.path.expanduser(os.path.join('~', '.gitcookies')),
            gerrit_util.CookiesAuthenticator().get_gitcookies_path())

        scm.GIT.SetConfig(os.getcwd(), 'http.cookiefile', '/some/path')
        self.assertEqual(
            '/some/path',
            gerrit_util.CookiesAuthenticator().get_gitcookies_path())

        os.getenv.return_value = 'git-cookies-path'
        self.assertEqual(
            'git-cookies-path',
            gerrit_util.CookiesAuthenticator().get_gitcookies_path())
        os.getenv.assert_called_with('GIT_COOKIES_PATH')

    def testGitcookies(self):
        auth = gerrit_util.CookiesAuthenticator()
        self.assertEqual(
            auth.gitcookies, {
                'chromium.googlesource.com':
                ('git-user.chromium.org', '1/chromium-secret'),
                'chromium-review.googlesource.com':
                ('git-user.chromium.org', '1/chromium-secret'),
                '.example.com': ('', 'example-bearer-token'),
            })

    def testGetAuthHeader(self):
        expected_chromium_header = (
            'Basic Z2l0LXVzZXIuY2hyb21pdW0ub3JnOjEvY2hyb21pdW0tc2VjcmV0')

        auth = gerrit_util.CookiesAuthenticator()
        self.assertAuthenticatedConnAuth(auth, 'chromium.googlesource.com',
                                         expected_chromium_header)
        self.assertAuthenticatedConnAuth(auth,
                                         'chromium-review.googlesource.com',
                                         expected_chromium_header)
        self.assertAuthenticatedConnAuth(auth, 'some-review.example.com',
                                         'Bearer example-bearer-token')

    def testGetAuthEmail(self):
        auth = gerrit_util.CookiesAuthenticator()
        self.assertEqual('user@chromium.org',
                         auth.get_auth_email('chromium.googlesource.com'))
        self.assertEqual(
            'user@chromium.org',
            auth.get_auth_email('chromium-review.googlesource.com'))
        self.assertIsNone(auth.get_auth_email('some-review.example.com'))


class GceAuthenticatorTest(unittest.TestCase):
    def setUp(self):
        super(GceAuthenticatorTest, self).setUp()
        mock.patch('httplib2.Http').start()
        mock.patch('os.getenv', return_value=None).start()
        mock.patch('gerrit_util.time_sleep').start()
        mock.patch('gerrit_util.time_time').start()
        self.addCleanup(mock.patch.stopall)

        # GceAuthenticator has class variables that cache the results. Build a
        # new class for every test to avoid inter-test dependencies.
        class GceAuthenticator(gerrit_util.GceAuthenticator):
            pass

        self.GceAuthenticator = GceAuthenticator

    def assertAuthenticatedToken(self, token: Optional[str]):
        conn = makeConn('some.example.com')
        self.GceAuthenticator().authenticate(conn)
        if token is None:
            self.assertNotIn('Authorization', conn.req_headers)
        else:
            self.assertEqual(conn.req_headers['Authorization'], token)

    def testIsGce_EnvVarSkip(self, *_mocks):
        os.getenv.return_value = '1'
        self.assertFalse(self.GceAuthenticator.is_applicable())
        os.getenv.assert_called_once_with('SKIP_GCE_AUTH_FOR_GIT')

    def testIsGce_Error(self):
        httplib2.Http().request.side_effect = httplib2.HttpLib2Error
        self.assertFalse(self.GceAuthenticator.is_applicable())

    def testIsGce_500(self):
        httplib2.Http().request.return_value = (mock.Mock(status=500), None)
        self.assertFalse(self.GceAuthenticator.is_applicable())
        last_call = gerrit_util.time_sleep.mock_calls[-1]
        self.assertLessEqual(last_call, mock.call(43.0))

    def testIsGce_FailsThenSucceeds(self):
        response = mock.Mock(status=200)
        response.get.return_value = 'Google'
        httplib2.Http().request.side_effect = [
            (mock.Mock(status=500), None),
            (response, 'who cares'),
        ]
        self.assertTrue(self.GceAuthenticator.is_applicable())

    def testIsGce_MetadataFlavorIsNotGoogle(self):
        response = mock.Mock(status=200)
        response.get.return_value = None
        httplib2.Http().request.return_value = (response, 'who cares')
        self.assertFalse(self.GceAuthenticator.is_applicable())
        response.get.assert_called_once_with('metadata-flavor')

    def testIsGce_ResultIsCached(self):
        response = mock.Mock(status=200)
        response.get.return_value = 'Google'
        httplib2.Http().request.side_effect = [(response, 'who cares')]
        self.assertTrue(self.GceAuthenticator.is_applicable())
        self.assertTrue(self.GceAuthenticator.is_applicable())
        httplib2.Http().request.assert_called_once()

    def testGetAuthHeader_Error(self):
        httplib2.Http().request.side_effect = httplib2.HttpLib2Error
        self.assertAuthenticatedToken(None)

    def testGetAuthHeader_500(self):
        httplib2.Http().request.return_value = (mock.Mock(status=500), None)
        self.assertAuthenticatedToken(None)

    def testGetAuthHeader_Non200(self):
        httplib2.Http().request.return_value = (mock.Mock(status=403), None)
        self.assertAuthenticatedToken(None)

    def testGetAuthHeader_OK(self):
        httplib2.Http().request.return_value = (
            mock.Mock(status=200),
            '{"expires_in": 125, "token_type": "TYPE", "access_token": "TOKEN"}'
        )
        gerrit_util.time_time.return_value = 0
        self.assertAuthenticatedToken('TYPE TOKEN')

    def testGetAuthHeader_Cache(self):
        httplib2.Http().request.return_value = (
            mock.Mock(status=200),
            '{"expires_in": 125, "token_type": "TYPE", "access_token": "TOKEN"}'
        )
        gerrit_util.time_time.return_value = 0
        self.assertAuthenticatedToken('TYPE TOKEN')
        self.assertAuthenticatedToken('TYPE TOKEN')
        httplib2.Http().request.assert_called_once()

    def testGetAuthHeader_CacheOld(self):
        httplib2.Http().request.return_value = (
            mock.Mock(status=200),
            '{"expires_in": 125, "token_type": "TYPE", "access_token": "TOKEN"}'
        )
        gerrit_util.time_time.side_effect = [0, 100, 200]
        self.assertAuthenticatedToken('TYPE TOKEN')
        self.assertAuthenticatedToken('TYPE TOKEN')
        self.assertEqual(2, len(httplib2.Http().request.mock_calls))


class GerritUtilTest(unittest.TestCase):
    def setUp(self):
        super(GerritUtilTest, self).setUp()
        mock.patch('gerrit_util.LOGGER').start()
        mock.patch('gerrit_util.time_sleep').start()
        mock.patch('metrics.collector').start()
        mock.patch('metrics_utils.extract_http_metrics',
                   return_value='http_metrics').start()
        self.addCleanup(mock.patch.stopall)

    def testQueryString(self):
        self.assertEqual('', gerrit_util._QueryString([]))
        self.assertEqual('first%20param%2B',
                         gerrit_util._QueryString([], 'first param+'))
        self.assertEqual(
            'key:val+foo:bar',
            gerrit_util._QueryString([('key', 'val'), ('foo', 'bar')]))
        self.assertEqual(
            'first%20param%2B+key:val+foo:bar',
            gerrit_util._QueryString([('key', 'val'), ('foo', 'bar')],
                                     'first param+'))

    @mock.patch('gerrit_util.CookiesAuthenticator._get_auth_for_host')
    @mock.patch('gerrit_util._Authenticator.get')
    def testCreateHttpConn_Basic(self, mockAuth, cookieAuth):
        mockAuth.return_value = gerrit_util.CookiesAuthenticator()
        cookieAuth.return_value = None

        conn = gerrit_util.CreateHttpConn('host.example.com', 'foo/bar')
        self.assertEqual('host.example.com', conn.req_host)
        self.assertEqual(
            {
                'uri': 'https://host.example.com/a/foo/bar',
                'method': 'GET',
                'headers': {},
                'body': None,
            }, conn.req_params)

    @mock.patch('gerrit_util.CookiesAuthenticator._get_auth_for_host')
    @mock.patch('gerrit_util._Authenticator.get')
    def testCreateHttpConn_Authenticated(self, mockAuth, cookieAuth):
        mockAuth.return_value = gerrit_util.CookiesAuthenticator()
        cookieAuth.return_value = (None, 'token')

        conn = gerrit_util.CreateHttpConn('host.example.com',
                                          'foo/bar',
                                          headers={'header': 'value'})
        self.assertEqual('host.example.com', conn.req_host)
        self.assertEqual(
            {
                'uri': 'https://host.example.com/a/foo/bar',
                'method': 'GET',
                'headers': {
                    'Authorization': 'Bearer token',
                    'header': 'value'
                },
                'body': None,
            }, conn.req_params)

    @mock.patch('gerrit_util.CookiesAuthenticator._get_auth_for_host')
    @mock.patch('gerrit_util._Authenticator')
    def testCreateHttpConn_Body(self, mockAuth, cookieAuth):
        mockAuth.return_value = gerrit_util.CookiesAuthenticator()
        cookieAuth.return_value = None

        conn = gerrit_util.CreateHttpConn('host.example.com',
                                          'foo/bar',
                                          body={
                                              'l': [1, 2, 3],
                                              'd': {
                                                  'k': 'v'
                                              }
                                          })
        self.assertEqual('host.example.com', conn.req_host)
        self.assertEqual(
            {
                'uri': 'https://host.example.com/a/foo/bar',
                'method': 'GET',
                'headers': {
                    'Content-Type': 'application/json'
                },
                'body': '{"d": {"k": "v"}, "l": [1, 2, 3]}',
            }, conn.req_params)

    def testReadHttpResponse_200(self):
        conn = mock.Mock()
        conn.req_params = {'uri': 'uri', 'method': 'method'}
        conn.request.return_value = (mock.Mock(status=200),
                                     b'content\xe2\x9c\x94')

        content = gerrit_util.ReadHttpResponse(conn)
        self.assertEqual('content✔', content.getvalue())
        metrics.collector.add_repeated.assert_called_once_with(
            'http_requests', 'http_metrics')

    def testReadHttpResponse_AuthenticationIssue(self):
        for status in (302, 401, 403):
            response = mock.Mock(status=status)
            response.get.return_value = None
            conn = mock.Mock(req_params={'uri': 'uri', 'method': 'method'})
            conn.request.return_value = (response, b'')

            with mock.patch('sys.stdout', StringIO()):
                with self.assertRaises(gerrit_util.GerritError) as cm:
                    gerrit_util.ReadHttpResponse(conn)

                self.assertEqual(status, cm.exception.http_status)
                self.assertIn('Your Gerrit credentials might be misconfigured',
                              sys.stdout.getvalue())

    def testReadHttpResponse_ClientError(self):
        conn = mock.Mock(req_params={'uri': 'uri', 'method': 'method'})
        conn.request.return_value = (mock.Mock(status=404), b'')

        with self.assertRaises(gerrit_util.GerritError) as cm:
            gerrit_util.ReadHttpResponse(conn)

        self.assertEqual(404, cm.exception.http_status)

    def readHttpResponse_ServerErrorHelper(self, status):
        conn = mock.Mock(req_params={'uri': 'uri', 'method': 'method'})
        conn.request.return_value = (mock.Mock(status=status), b'')

        with self.assertRaises(gerrit_util.GerritError) as cm:
            gerrit_util.ReadHttpResponse(conn)

        self.assertEqual(status, cm.exception.http_status)
        self.assertEqual(gerrit_util.TRY_LIMIT, len(conn.request.mock_calls))
        last_call = gerrit_util.time_sleep.mock_calls[-1]
        self.assertLessEqual(last_call, mock.call(422.0))

    def testReadHttpResponse_ServerError(self):
        self.readHttpResponse_ServerErrorHelper(status=404)
        self.readHttpResponse_ServerErrorHelper(status=409)
        self.readHttpResponse_ServerErrorHelper(status=429)
        self.readHttpResponse_ServerErrorHelper(status=500)

    def testReadHttpResponse_ServerErrorAndSuccess(self):
        conn = mock.Mock(req_params={'uri': 'uri', 'method': 'method'})
        conn.request.side_effect = [
            (mock.Mock(status=500), b''),
            (mock.Mock(status=200), b'content\xe2\x9c\x94'),
        ]

        self.assertEqual('content✔',
                         gerrit_util.ReadHttpResponse(conn).getvalue())
        self.assertEqual(2, len(conn.request.mock_calls))
        gerrit_util.time_sleep.assert_called_once_with(12.0)

    def testReadHttpResponse_TimeoutAndSuccess(self):
        conn = mock.Mock(req_params={'uri': 'uri', 'method': 'method'})
        conn.request.side_effect = [
            socket.timeout('timeout'),
            (mock.Mock(status=200), b'content\xe2\x9c\x94'),
        ]

        self.assertEqual('content✔',
                         gerrit_util.ReadHttpResponse(conn).getvalue())
        self.assertEqual(2, len(conn.request.mock_calls))
        gerrit_util.time_sleep.assert_called_once_with(12.0)

    def testReadHttpResponse_SetMaxTries(self):
        conn = mock.Mock(req_params={'uri': 'uri', 'method': 'method'})
        conn.request.side_effect = [
            (mock.Mock(status=409), b'error!'),
            (mock.Mock(status=409), b'error!'),
            (mock.Mock(status=409), b'error!'),
        ]

        self.assertRaises(gerrit_util.GerritError,
                          gerrit_util.ReadHttpResponse,
                          conn,
                          max_tries=2)
        self.assertEqual(2, len(conn.request.mock_calls))
        gerrit_util.time_sleep.assert_called_once_with(12.0)

    def testReadHttpResponse_Expected404(self):
        conn = mock.Mock()
        conn.req_params = {'uri': 'uri', 'method': 'method'}
        conn.request.return_value = (mock.Mock(status=404),
                                     b'content\xe2\x9c\x94')

        content = gerrit_util.ReadHttpResponse(conn, (404, ))
        self.assertEqual('', content.getvalue())

    @mock.patch('gerrit_util.ReadHttpResponse')
    def testReadHttpJsonResponse_NotJSON(self, mockReadHttpResponse):
        mockReadHttpResponse.return_value = StringIO('not json')
        with self.assertRaises(gerrit_util.GerritError) as cm:
            gerrit_util.ReadHttpJsonResponse(None)
        self.assertEqual(cm.exception.http_status, 200)
        self.assertEqual(cm.exception.message,
                         '(200) Unexpected json output: not json')

    @mock.patch('gerrit_util.ReadHttpResponse')
    def testReadHttpJsonResponse_EmptyValue(self, mockReadHttpResponse):
        mockReadHttpResponse.return_value = StringIO(')]}\'')
        self.assertEqual(gerrit_util.ReadHttpJsonResponse(None), {})

    @mock.patch('gerrit_util.ReadHttpResponse')
    def testReadHttpJsonResponse_JSON(self, mockReadHttpResponse):
        expected_value = {'foo': 'bar', 'baz': [1, '2', 3]}
        mockReadHttpResponse.return_value = StringIO(')]}\'\n' +
                                                     json.dumps(expected_value))
        self.assertEqual(expected_value, gerrit_util.ReadHttpJsonResponse(None))

    @mock.patch('gerrit_util.CreateHttpConn')
    @mock.patch('gerrit_util.ReadHttpJsonResponse')
    def testQueryChanges(self, mockJsonResponse, mockCreateHttpConn):
        gerrit_util.QueryChanges('host', [('key', 'val'), ('foo', 'bar baz')],
                                 'first param',
                                 limit=500,
                                 o_params=['PARAM_A', 'PARAM_B'],
                                 start='start')
        mockCreateHttpConn.assert_called_once_with(
            'host', ('changes/?q=first%20param+key:val+foo:bar+baz'
                     '&start=start'
                     '&n=500'
                     '&o=PARAM_A'
                     '&o=PARAM_B'),
            timeout=30.0)

    def testQueryChanges_NoParams(self):
        self.assertRaises(RuntimeError, gerrit_util.QueryChanges, 'host', [])

    @mock.patch('gerrit_util.QueryChanges')
    def testGenerateAllChanges(self, mockQueryChanges):
        mockQueryChanges.side_effect = [
            # First results page
            [
                {
                    '_number': '4'
                },
                {
                    '_number': '3'
                },
                {
                    '_number': '2',
                    '_more_changes': True
                },
            ],
            # Second results page, there are new changes, so second page
            # includes some results from the first page.
            [
                {
                    '_number': '2'
                },
                {
                    '_number': '1'
                },
            ],
            # GenerateAllChanges queries again from the start to get any new
            # changes (5 in this case).
            [
                {
                    '_number': '5'
                },
                {
                    '_number': '4'
                },
                {
                    '_number': '3',
                    '_more_changes': True
                },
            ],
        ]

        changes = list(gerrit_util.GenerateAllChanges('host', 'params'))
        self.assertEqual([
            {
                '_number': '4'
            },
            {
                '_number': '3'
            },
            {
                '_number': '2',
                '_more_changes': True
            },
            {
                '_number': '1'
            },
            {
                '_number': '5'
            },
        ], changes)
        self.assertEqual([
            mock.call('host', 'params', None, 500, None, 0),
            mock.call('host', 'params', None, 500, None, 3),
            mock.call('host', 'params', None, 500, None, 0),
        ], mockQueryChanges.mock_calls)

    @mock.patch('gerrit_util.CreateHttpConn')
    @mock.patch('gerrit_util.ReadHttpJsonResponse')
    def testIsCodeOwnersEnabledOnRepo_Disabled(self, mockJsonResponse,
                                               mockCreateHttpConn):
        mockJsonResponse.return_value = {'status': {'disabled': True}}
        self.assertFalse(gerrit_util.IsCodeOwnersEnabledOnRepo('host', 'repo'))

    @mock.patch('gerrit_util.CreateHttpConn')
    @mock.patch('gerrit_util.ReadHttpJsonResponse')
    def testIsCodeOwnersEnabledOnRepo_Enabled(self, mockJsonResponse,
                                              mockCreateHttpConn):
        mockJsonResponse.return_value = {'status': {}}
        self.assertTrue(gerrit_util.IsCodeOwnersEnabledOnRepo('host', 'repo'))


class SSOAuthenticatorTest(unittest.TestCase):

    @classmethod
    def setUpClass(cls) -> None:
        cls._original_timeout_secs = gerrit_util.SSOAuthenticator._timeout_secs
        return super().setUpClass()

    def setUp(self) -> None:
        gerrit_util.SSOAuthenticator._sso_info = None
        gerrit_util.SSOAuthenticator._testing_load_expired_cookies = True
        gerrit_util.SSOAuthenticator._timeout_secs = self._original_timeout_secs
        self.sso = gerrit_util.SSOAuthenticator()
        return super().setUp()

    def tearDown(self) -> None:
        gerrit_util.SSOAuthenticator._sso_info = None
        gerrit_util.SSOAuthenticator._testing_load_expired_cookies = False
        gerrit_util.SSOAuthenticator._timeout_secs = self._original_timeout_secs
        return super().tearDown()

    @property
    def _input_dir(self) -> Path:
        base = Path(__file__).absolute().with_suffix('.inputs')
        # Here _testMethodName would be a string like "testCmdAssemblyFound"
        return base / self._testMethodName

    @mock.patch('gerrit_util.ssoHelper.find_cmd',
                return_value='/fake/git-remote-sso')
    def testCmdAssemblyFound(self, _):
        self.assertEqual(self.sso._resolve_sso_cmd(),
                         ('/fake/git-remote-sso', '-print_config',
                          'sso://*.git.corp.google.com'))
        with mock.patch('scm.GIT.GetConfig') as p:
            p.side_effect = ['firefly@google.com']
            self.assertTrue(self.sso.is_applicable())

    @mock.patch('gerrit_util.ssoHelper.find_cmd', return_value=None)
    def testCmdAssemblyNotFound(self, _):
        self.assertEqual(self.sso._resolve_sso_cmd(), ())
        self.assertFalse(self.sso.is_applicable())

    def testParseConfigOK(self):
        test_config = {
            'somekey': 'a value with = in it',
            'novalue': '',
            'http.proxy': 'localhost:12345',
            'http.cookiefile': str(self._input_dir / 'cookiefile.txt'),
            'include.path': str(self._input_dir / 'gitconfig'),
        }
        parsed = self.sso._parse_config(test_config)
        self.assertDictEqual(parsed.headers, {
            'Authorization': 'Basic REALLY_COOL_TOKEN',
        })
        self.assertEqual(parsed.proxy.proxy_host, b'localhost')
        self.assertEqual(parsed.proxy.proxy_port, 12345)

        c = parsed.cookies._cookies
        self.assertEqual(c['login.example.com']['/']['SSO'].value,
                         'TUVFUE1PUlAK')
        self.assertEqual(c['.example.com']['/']['__CoolProxy'].value,
                         'QkxFRVBCTE9SUAo=')

    @unittest.skipUnless(RUN_SUBPROC_TESTS, 'subprocess tests are flakey')
    def testLaunchHelperOK(self):
        gerrit_util.SSOAuthenticator._sso_cmd = ('python3',
                                                 str(self._input_dir /
                                                     'git-remote-sso.py'))

        info = self.sso._get_sso_info()
        self.assertDictEqual(info.headers, {
            'Authorization': 'Basic REALLY_COOL_TOKEN',
        })
        self.assertEqual(info.proxy.proxy_host, b'localhost')
        self.assertEqual(info.proxy.proxy_port, 12345)
        c = info.cookies._cookies
        self.assertEqual(c['login.example.com']['/']['SSO'].value,
                         'TUVFUE1PUlAK')
        self.assertEqual(c['.example.com']['/']['__CoolProxy'].value,
                         'QkxFRVBCTE9SUAo=')

    @unittest.skipUnless(RUN_SUBPROC_TESTS, 'subprocess tests are flakey')
    def testLaunchHelperFailQuick(self):
        gerrit_util.SSOAuthenticator._sso_cmd = ('python3',
                                                 str(self._input_dir /
                                                     'git-remote-sso.py'))

        with self.assertRaisesRegex(SystemExit, "SSO Failure Message!!!"):
            self.sso._get_sso_info()

    @unittest.skipUnless(RUN_SUBPROC_TESTS, 'subprocess tests are flakey')
    def testLaunchHelperFailSlow(self):
        gerrit_util.SSOAuthenticator._timeout_secs = 0.2
        gerrit_util.SSOAuthenticator._sso_cmd = ('python3',
                                                 str(self._input_dir /
                                                     'git-remote-sso.py'))

        with self.assertRaises(subprocess.TimeoutExpired):
            self.sso._get_sso_info()


class SSOHelperTest(unittest.TestCase):

    def setUp(self) -> None:
        self.sso = gerrit_util.SSOHelper()
        return super().setUp()

    @mock.patch('shutil.which', return_value='/fake/git-remote-sso')
    def testFindCmd(self, _):
        self.assertEqual(self.sso.find_cmd(), '/fake/git-remote-sso')

    @mock.patch('shutil.which', return_value=None)
    def testFindCmdMissing(self, _):
        self.assertEqual(self.sso.find_cmd(), '')

    @mock.patch('shutil.which', return_value='/fake/git-remote-sso')
    def testFindCmdCached(self, which):
        self.sso.find_cmd()
        self.sso.find_cmd()
        self.assertEqual(which.called, 1)


class ShouldUseSSOTest(unittest.TestCase):

    def setUp(self) -> None:
        self.newauth = mock.patch('newauth.Enabled', return_value=True)
        self.newauth.start()
        self.cwd = mock.patch('os.getcwd', return_value='/fake/cwd')
        self.cwd.start()
        self.sso = mock.patch('gerrit_util.ssoHelper.find_cmd',
                              return_value='/fake/git-remote-sso')
        self.sso.start()
        scm_mock.GIT(self)
        self.addCleanup(mock.patch.stopall)

        gerrit_util.ShouldUseSSO.cache_clear()
        return super().setUp()

    def tearDown(self) -> None:
        super().tearDown()
        self.sso.stop()
        self.newauth.stop()

    @mock.patch('newauth.Enabled', return_value=False)
    def testDisabled(self, _):
        self.assertFalse(
            gerrit_util.ShouldUseSSO('fake-host.googlesource.com',
                                     'firefly@google.com'))

    @mock.patch('gerrit_util.ssoHelper.find_cmd', return_value='')
    def testMissingCommand(self, _):
        self.assertFalse(
            gerrit_util.ShouldUseSSO('fake-host.googlesource.com',
                                     'firefly@google.com'))

    def testBadHost(self):
        self.assertFalse(
            gerrit_util.ShouldUseSSO('fake-host.coreboot.org',
                                     'firefly@google.com'))

    def testEmptyEmail(self):
        self.assertTrue(
            gerrit_util.ShouldUseSSO('fake-host.googlesource.com', ''))

    def testGoogleEmail(self):
        self.assertTrue(
            gerrit_util.ShouldUseSSO('fake-host.googlesource.com',
                                     'firefly@google.com'))

    def testGmail(self):
        self.assertFalse(
            gerrit_util.ShouldUseSSO('fake-host.googlesource.com',
                                     'firefly@gmail.com'))

    @mock.patch('gerrit_util.GetAccountEmails',
                return_value=[{
                    'email': 'firefly@chromium.org'
                }])
    def testLinkedChromium(self, email):
        self.assertTrue(
            gerrit_util.ShouldUseSSO('fake-host.googlesource.com',
                                     'firefly@chromium.org'))
        email.assert_called_with('fake-host.googlesource.com',
                                 'self',
                                 authenticator=mock.ANY)

    @mock.patch('gerrit_util.GetAccountEmails',
                return_value=[{
                    'email': 'firefly@google.com'
                }])
    def testUnlinkedChromium(self, email):
        self.assertFalse(
            gerrit_util.ShouldUseSSO('fake-host.googlesource.com',
                                     'firefly@chromium.org'))
        email.assert_called_with('fake-host.googlesource.com',
                                 'self',
                                 authenticator=mock.ANY)


if __name__ == '__main__':
    unittest.main()
