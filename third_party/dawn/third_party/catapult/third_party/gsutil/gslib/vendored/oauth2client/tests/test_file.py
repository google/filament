# Copyright 2014 Google Inc. All rights reserved.
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

"""Unit tests for oauth2client.file."""

import copy
import datetime
import json
import os
import pickle
import stat
import tempfile
import unittest
import warnings

import mock
import six
from six.moves import http_client
from six.moves import urllib_parse

from oauth2client import _helpers
from oauth2client import client
from oauth2client import file as file_module
from oauth2client import transport
from oauth2client.contrib import multiprocess_file_storage
from tests import http_mock

try:
    # Python2
    from future_builtins import oct
except ImportError:  # pragma: NO COVER
    pass

_filehandle, FILENAME = tempfile.mkstemp('oauth2client_test.data')
os.close(_filehandle)


class OAuth2ClientFileTests(unittest.TestCase):

    def tearDown(self):
        try:
            os.unlink(FILENAME)
        except OSError:
            pass

    def setUp(self):
        warnings.simplefilter("ignore")
        try:
            os.unlink(FILENAME)
        except OSError:
            pass

    def _create_test_credentials(self, client_id='some_client_id',
                                 expiration=None):
        access_token = 'foo'
        client_secret = 'cOuDdkfjxxnv+'
        refresh_token = '1/0/a.df219fjls0'
        token_expiry = expiration or datetime.datetime.utcnow()
        token_uri = 'https://www.google.com/accounts/o8/oauth2/token'
        user_agent = 'refresh_checker/1.0'

        credentials = client.OAuth2Credentials(
            access_token, client_id, client_secret,
            refresh_token, token_expiry, token_uri,
            user_agent)
        return credentials

    @mock.patch('warnings.warn')
    def test_non_existent_file_storage(self, warn_mock):
        storage = file_module.Storage(FILENAME)
        credentials = storage.get()
        warn_mock.assert_called_with(
            _helpers._MISSING_FILE_MESSAGE.format(FILENAME))
        self.assertIsNone(credentials)

    def test_directory_file_storage(self):
        storage = file_module.Storage(FILENAME)
        os.mkdir(FILENAME)
        try:
            with self.assertRaises(IOError):
                storage.get()
        finally:
            os.rmdir(FILENAME)

    @unittest.skipIf(not hasattr(os, 'symlink'), 'No symlink available')
    def test_no_sym_link_credentials(self):
        SYMFILENAME = FILENAME + '.sym'
        os.symlink(FILENAME, SYMFILENAME)
        storage = file_module.Storage(SYMFILENAME)
        try:
            with self.assertRaises(IOError):
                storage.get()
        finally:
            os.unlink(SYMFILENAME)

    def test_pickle_and_json_interop(self):
        # Write a file with a pickled OAuth2Credentials.
        credentials = self._create_test_credentials()

        credentials_file = open(FILENAME, 'wb')
        pickle.dump(credentials, credentials_file)
        credentials_file.close()

        # Storage should be not be able to read that object, as the capability
        # to read and write credentials as pickled objects has been removed.
        storage = file_module.Storage(FILENAME)
        read_credentials = storage.get()
        self.assertIsNone(read_credentials)

        # Now write it back out and confirm it has been rewritten as JSON
        storage.put(credentials)
        with open(FILENAME) as credentials_file:
            data = json.load(credentials_file)

        self.assertEquals(data['access_token'], 'foo')
        self.assertEquals(data['_class'], 'OAuth2Credentials')
        self.assertEquals(data['_module'], client.OAuth2Credentials.__module__)

    def test_token_refresh_store_expired(self):
        expiration = (datetime.datetime.utcnow() -
                      datetime.timedelta(minutes=15))
        credentials = self._create_test_credentials(expiration=expiration)

        storage = file_module.Storage(FILENAME)
        storage.put(credentials)
        credentials = storage.get()
        new_cred = copy.copy(credentials)
        new_cred.access_token = 'bar'
        storage.put(new_cred)

        access_token = '1/3w'
        token_response = {'access_token': access_token, 'expires_in': 3600}
        response_content = json.dumps(token_response).encode('utf-8')
        http = http_mock.HttpMock(data=response_content)

        credentials._refresh(http)
        self.assertEquals(credentials.access_token, access_token)

        # Verify mocks.
        self.assertEqual(http.requests, 1)
        self.assertEqual(http.uri, credentials.token_uri)
        self.assertEqual(http.method, 'POST')
        expected_body = {
            'grant_type': ['refresh_token'],
            'client_id': [credentials.client_id],
            'client_secret': [credentials.client_secret],
            'refresh_token': [credentials.refresh_token],
        }
        self.assertEqual(urllib_parse.parse_qs(http.body), expected_body)
        expected_headers = {
            'content-type': 'application/x-www-form-urlencoded',
            'user-agent': credentials.user_agent,
        }
        self.assertEqual(http.headers, expected_headers)

    def test_token_refresh_store_expires_soon(self):
        # Tests the case where an access token that is valid when it is read
        # from the store expires before the original request succeeds.
        expiration = (datetime.datetime.utcnow() +
                      datetime.timedelta(minutes=15))
        credentials = self._create_test_credentials(expiration=expiration)

        storage = file_module.Storage(FILENAME)
        storage.put(credentials)
        credentials = storage.get()
        new_cred = copy.copy(credentials)
        new_cred.access_token = 'bar'
        storage.put(new_cred)

        access_token = '1/3w'
        token_response = {'access_token': access_token, 'expires_in': 3600}
        http = http_mock.HttpMockSequence([
            ({'status': http_client.UNAUTHORIZED},
             b'Initial token expired'),
            ({'status': http_client.UNAUTHORIZED},
             b'Store token expired'),
            ({'status': http_client.OK},
             json.dumps(token_response).encode('utf-8')),
            ({'status': http_client.OK},
             b'Valid response to original request')
        ])

        credentials.authorize(http)
        transport.request(http, 'https://example.com')
        self.assertEqual(credentials.access_token, access_token)

    def test_token_refresh_good_store(self):
        expiration = (datetime.datetime.utcnow() +
                      datetime.timedelta(minutes=15))
        credentials = self._create_test_credentials(expiration=expiration)

        storage = file_module.Storage(FILENAME)
        storage.put(credentials)
        credentials = storage.get()
        new_cred = copy.copy(credentials)
        new_cred.access_token = 'bar'
        storage.put(new_cred)

        credentials._refresh(None)
        self.assertEquals(credentials.access_token, 'bar')

    def test_token_refresh_stream_body(self):
        expiration = (datetime.datetime.utcnow() +
                      datetime.timedelta(minutes=15))
        credentials = self._create_test_credentials(expiration=expiration)

        storage = file_module.Storage(FILENAME)
        storage.put(credentials)
        credentials = storage.get()
        new_cred = copy.copy(credentials)
        new_cred.access_token = 'bar'
        storage.put(new_cred)

        valid_access_token = '1/3w'
        token_response = {'access_token': valid_access_token,
                          'expires_in': 3600}
        http = http_mock.HttpMockSequence([
            ({'status': http_client.UNAUTHORIZED},
             b'Initial token expired'),
            ({'status': http_client.UNAUTHORIZED},
             b'Store token expired'),
            ({'status': http_client.OK},
             json.dumps(token_response).encode('utf-8')),
            ({'status': http_client.OK}, 'echo_request_body')
        ])

        body = six.StringIO('streaming body')

        credentials.authorize(http)
        _, content = transport.request(http, 'https://example.com', body=body)
        self.assertEqual(content, 'streaming body')
        self.assertEqual(credentials.access_token, valid_access_token)

    def test_credentials_delete(self):
        credentials = self._create_test_credentials()

        storage = file_module.Storage(FILENAME)
        storage.put(credentials)
        credentials = storage.get()
        self.assertIsNotNone(credentials)
        storage.delete()
        credentials = storage.get()
        self.assertIsNone(credentials)

    def test_access_token_credentials(self):
        access_token = 'foo'
        user_agent = 'refresh_checker/1.0'

        credentials = client.AccessTokenCredentials(access_token, user_agent)

        storage = file_module.Storage(FILENAME)
        credentials = storage.put(credentials)
        credentials = storage.get()

        self.assertIsNotNone(credentials)
        self.assertEquals('foo', credentials.access_token)

        self.assertTrue(os.path.exists(FILENAME))

        if os.name == 'posix':  # pragma: NO COVER
            mode = os.stat(FILENAME).st_mode
            self.assertEquals('0o600', oct(stat.S_IMODE(mode)))

    def test_multiprocess_file_storage_credestore_permissions(self):
        credentials = 'dummy-string'

        storage = multiprocess_file_storage.MultiprocessFileStorage(
            FILENAME,
            credentials
        )
        storage.acquire_lock()
        storage.release_lock()

        self.assertTrue(os.path.exists(FILENAME))

        if os.name == 'posix':  # pragma: NO COVER
          mode = os.stat(FILENAME).st_mode
          self.assertEquals('0o600', oct(stat.S_IMODE(mode)))
