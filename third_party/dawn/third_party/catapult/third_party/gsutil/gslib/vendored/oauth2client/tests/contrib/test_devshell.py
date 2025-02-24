# Copyright 2015 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Tests for oauth2client.contrib.devshell."""

import datetime
import json
import os
import socket
import threading
import unittest

import mock

from oauth2client import _helpers
from oauth2client import client
from oauth2client.contrib import devshell

# A dummy value to use for the expires_in field
# in CredentialInfoResponse.
EXPIRES_IN = 1000
DEFAULT_CREDENTIAL_JSON = json.dumps([
    'joe@example.com',
    'fooproj',
    'sometoken',
    EXPIRES_IN
])


class TestCredentialInfoResponse(unittest.TestCase):

    def test_constructor_with_non_list(self):
        json_non_list = '{}'
        with self.assertRaises(ValueError):
            devshell.CredentialInfoResponse(json_non_list)

    def test_constructor_with_bad_json(self):
        json_non_list = '{BADJSON'
        with self.assertRaises(ValueError):
            devshell.CredentialInfoResponse(json_non_list)

    def test_constructor_empty_list(self):
        info_response = devshell.CredentialInfoResponse('[]')
        self.assertEqual(info_response.user_email, None)
        self.assertEqual(info_response.project_id, None)
        self.assertEqual(info_response.access_token, None)
        self.assertEqual(info_response.expires_in, None)

    def test_constructor_full_list(self):
        user_email = 'user_email'
        project_id = 'project_id'
        access_token = 'access_token'
        expires_in = 1
        json_string = json.dumps(
            [user_email, project_id, access_token, expires_in])
        info_response = devshell.CredentialInfoResponse(json_string)
        self.assertEqual(info_response.user_email, user_email)
        self.assertEqual(info_response.project_id, project_id)
        self.assertEqual(info_response.access_token, access_token)
        self.assertEqual(info_response.expires_in, expires_in)


class Test_SendRecv(unittest.TestCase):

    def test_port_zero(self):
        with mock.patch('oauth2client.contrib.devshell.os') as os_mod:
            os_mod.getenv = mock.Mock(name='getenv', return_value=0)
            with self.assertRaises(devshell.NoDevshellServer):
                devshell._SendRecv()
            os_mod.getenv.assert_called_once_with(devshell.DEVSHELL_ENV, 0)

    def test_no_newline_in_received_header(self):
        non_zero_port = 1
        sock = mock.Mock()

        header_without_newline = ''
        sock.recv(6).decode = mock.Mock(
            name='decode', return_value=header_without_newline)

        with mock.patch('oauth2client.contrib.devshell.os') as os_mod:
            os_mod.getenv = mock.Mock(name='getenv',
                                      return_value=non_zero_port)
            with mock.patch('oauth2client.contrib.devshell.socket') as socket:
                socket.socket = mock.Mock(name='socket',
                                          return_value=sock)
                with self.assertRaises(devshell.CommunicationError):
                    devshell._SendRecv()
                os_mod.getenv.assert_called_once_with(devshell.DEVSHELL_ENV, 0)
                socket.socket.assert_called_once_with()
                sock.recv(6).decode.assert_called_once_with()

                data = devshell.CREDENTIAL_INFO_REQUEST_JSON
                msg = _helpers._to_bytes(
                    '{0}\n{1}'.format(len(data), data), encoding='utf-8')
                expected_sock_calls = [
                    mock.call.recv(6),  # From the set-up above
                    mock.call.connect(('localhost', non_zero_port)),
                    mock.call.sendall(msg),
                    mock.call.recv(6),
                    mock.call.recv(6),  # From the check above
                ]
                self.assertEqual(sock.method_calls, expected_sock_calls)


class _AuthReferenceServer(threading.Thread):

    def __init__(self, response=None):
        super(_AuthReferenceServer, self).__init__(None)
        self.response = response or DEFAULT_CREDENTIAL_JSON
        self.bad_request = False

    def __enter__(self):
        return self.start_server()

    def start_server(self):
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._socket.bind(('localhost', 0))
        port = self._socket.getsockname()[1]
        os.environ[devshell.DEVSHELL_ENV] = str(port)
        self._socket.listen(0)
        self.daemon = True
        self.start()
        return self

    def __exit__(self, e_type, value, traceback):
        self.stop_server()

    def stop_server(self):
        del os.environ[devshell.DEVSHELL_ENV]
        self._socket.close()

    def run(self):
        s = None
        try:
            # Do not set the timeout on the socket, leave it in the blocking
            # mode as setting the timeout seems to cause spurious EAGAIN
            # errors on OSX.
            self._socket.settimeout(None)

            s, unused_addr = self._socket.accept()
            resp_buffer = ''
            resp_1 = s.recv(6).decode()
            nstr, extra = resp_1.split('\n', 1)
            resp_buffer = extra
            n = int(nstr)
            to_read = n - len(extra)
            if to_read > 0:
                resp_buffer += _helpers._from_bytes(
                    s.recv(to_read, socket.MSG_WAITALL))
            if resp_buffer != devshell.CREDENTIAL_INFO_REQUEST_JSON:
                self.bad_request = True
            response_len = len(self.response)
            s.sendall('{0}\n{1}'.format(response_len, self.response).encode())
        finally:
            # Will fail if s is None, but these tests never encounter
            # that scenario.
            s.close()


class DevshellCredentialsTests(unittest.TestCase):

    def test_signals_no_server(self):
        with self.assertRaises(devshell.NoDevshellServer):
            devshell.DevshellCredentials()

    def test_bad_message_to_mock_server(self):
        request_content = devshell.CREDENTIAL_INFO_REQUEST_JSON + 'extrastuff'
        request_message = _helpers._to_bytes(
            '{0}\n{1}'.format(len(request_content), request_content))
        response_message = 'foobar'
        with _AuthReferenceServer(response_message) as auth_server:
            self.assertFalse(auth_server.bad_request)
            sock = socket.socket()
            port = int(os.getenv(devshell.DEVSHELL_ENV, 0))
            sock.connect(('localhost', port))
            sock.sendall(request_message)

            # Mimic the receive part of _SendRecv
            header = sock.recv(6).decode()
            len_str, result = header.split('\n', 1)
            to_read = int(len_str) - len(result)
            result += sock.recv(to_read, socket.MSG_WAITALL).decode()

        self.assertTrue(auth_server.bad_request)
        self.assertEqual(result, response_message)

    def test_request_response(self):
        with _AuthReferenceServer():
            response = devshell._SendRecv()
            self.assertEqual(response.user_email, 'joe@example.com')
            self.assertEqual(response.project_id, 'fooproj')
            self.assertEqual(response.access_token, 'sometoken')

    def test_no_refresh_token(self):
        with _AuthReferenceServer():
            creds = devshell.DevshellCredentials()
            self.assertEquals(None, creds.refresh_token)

    @mock.patch('oauth2client.client._UTCNOW')
    def test_reads_credentials(self, utcnow):
        NOW = datetime.datetime(1992, 12, 31)
        utcnow.return_value = NOW
        with _AuthReferenceServer():
            creds = devshell.DevshellCredentials()
            self.assertEqual('joe@example.com', creds.user_email)
            self.assertEqual('fooproj', creds.project_id)
            self.assertEqual('sometoken', creds.access_token)
            self.assertEqual(
                NOW + datetime.timedelta(seconds=EXPIRES_IN),
                creds.token_expiry)
            utcnow.assert_called_once_with()

    def test_handles_skipped_fields(self):
        with _AuthReferenceServer('["joe@example.com"]'):
            creds = devshell.DevshellCredentials()
            self.assertEqual('joe@example.com', creds.user_email)
            self.assertEqual(None, creds.project_id)
            self.assertEqual(None, creds.access_token)
            self.assertEqual(None, creds.token_expiry)

    def test_handles_tiny_response(self):
        with _AuthReferenceServer('[]'):
            creds = devshell.DevshellCredentials()
            self.assertEqual(None, creds.user_email)
            self.assertEqual(None, creds.project_id)
            self.assertEqual(None, creds.access_token)

    def test_handles_ignores_extra_fields(self):
        with _AuthReferenceServer(
                '["joe@example.com", "fooproj", "sometoken", 1, "extra"]'):
            creds = devshell.DevshellCredentials()
            self.assertEqual('joe@example.com', creds.user_email)
            self.assertEqual('fooproj', creds.project_id)
            self.assertEqual('sometoken', creds.access_token)

    def test_refuses_to_save_to_well_known_file(self):
        ORIGINAL_ISDIR = os.path.isdir
        try:
            os.path.isdir = lambda path: True
            with _AuthReferenceServer():
                creds = devshell.DevshellCredentials()
                with self.assertRaises(NotImplementedError):
                    client.save_to_well_known_file(creds)
        finally:
            os.path.isdir = ORIGINAL_ISDIR

    def test_from_json(self):
        with self.assertRaises(NotImplementedError):
            devshell.DevshellCredentials.from_json(None)

    def test_serialization_data(self):
        with _AuthReferenceServer('[]'):
            credentials = devshell.DevshellCredentials()
            with self.assertRaises(NotImplementedError):
                getattr(credentials, 'serialization_data')
