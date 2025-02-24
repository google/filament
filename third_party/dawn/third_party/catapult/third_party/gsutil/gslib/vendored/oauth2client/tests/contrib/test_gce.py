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

"""Unit tests for oauth2client.contrib.gce."""

import datetime
import json
import os
import unittest

import mock
from six.moves import http_client
from six.moves import reload_module

from oauth2client import client
from oauth2client.contrib import _metadata
from oauth2client.contrib import gce
from tests import http_mock


SERVICE_ACCOUNT_INFO = {
    'scopes': ['a', 'b'],
    'email': 'a@example.com',
    'aliases': ['default']
}
METADATA_PATH = 'instance/service-accounts/a@example.com/token'


class AppAssertionCredentialsTests(unittest.TestCase):

    def test_constructor(self):
        credentials = gce.AppAssertionCredentials()
        self.assertIsNone(credentials.assertion_type, None)
        self.assertIsNone(credentials.service_account_email)
        self.assertIsNone(credentials.scopes)
        self.assertTrue(credentials.invalid)

    @mock.patch('warnings.warn')
    def test_constructor_with_scopes(self, warn_mock):
        scope = 'http://example.com/a http://example.com/b'
        scopes = scope.split()
        credentials = gce.AppAssertionCredentials(scopes=scopes)
        self.assertEqual(credentials.scopes, None)
        self.assertEqual(credentials.assertion_type, None)
        warn_mock.assert_called_once_with(gce._SCOPES_WARNING)

    def test_to_json(self):
        credentials = gce.AppAssertionCredentials()
        with self.assertRaises(NotImplementedError):
            credentials.to_json()

    def test_from_json(self):
        with self.assertRaises(NotImplementedError):
            gce.AppAssertionCredentials.from_json({})

    @mock.patch('oauth2client.contrib._metadata.get_token',
                side_effect=[('A', datetime.datetime.min),
                             ('B', datetime.datetime.max)])
    @mock.patch('oauth2client.contrib._metadata.get_service_account_info',
                return_value=SERVICE_ACCOUNT_INFO)
    def test_refresh_token(self, get_info, get_token):
        http_mock = object()
        credentials = gce.AppAssertionCredentials()
        credentials.invalid = False
        credentials.service_account_email = 'a@example.com'
        self.assertIsNone(credentials.access_token)
        credentials.get_access_token(http=http_mock)
        self.assertEqual(credentials.access_token, 'A')
        self.assertTrue(credentials.access_token_expired)
        get_token.assert_called_with(http_mock,
                                     service_account='a@example.com')
        credentials.get_access_token(http=http_mock)
        self.assertEqual(credentials.access_token, 'B')
        self.assertFalse(credentials.access_token_expired)
        get_token.assert_called_with(http_mock,
                                     service_account='a@example.com')
        get_info.assert_not_called()

    def test_refresh_token_failed_fetch(self):
        headers = {
            'status': http_client.NOT_FOUND,
            'content-type': 'application/json',
        }
        response = json.dumps({'access_token': 'a', 'expires_in': 100})
        http = http_mock.HttpMock(headers=headers, data=response)
        credentials = gce.AppAssertionCredentials()
        credentials.invalid = False
        credentials.service_account_email = 'a@example.com'
        with self.assertRaises(client.HttpAccessTokenRefreshError):
            credentials._refresh(http)
        # Verify mock.
        self.assertEqual(http.requests, 1)
        expected_uri = _metadata.METADATA_ROOT + METADATA_PATH
        self.assertEqual(http.uri, expected_uri)
        self.assertEqual(http.method, 'GET')
        self.assertIsNone(http.body)
        self.assertEqual(http.headers, _metadata.METADATA_HEADERS)

    def test_serialization_data(self):
        credentials = gce.AppAssertionCredentials()
        with self.assertRaises(NotImplementedError):
            getattr(credentials, 'serialization_data')

    def test_create_scoped_required(self):
        credentials = gce.AppAssertionCredentials()
        self.assertFalse(credentials.create_scoped_required())

    def test_sign_blob_not_implemented(self):
        credentials = gce.AppAssertionCredentials([])
        with self.assertRaises(NotImplementedError):
            credentials.sign_blob(b'blob')

    @mock.patch('oauth2client.contrib._metadata.get_service_account_info',
                return_value=SERVICE_ACCOUNT_INFO)
    def test_retrieve_scopes(self, metadata):
        http_mock = object()
        credentials = gce.AppAssertionCredentials()
        self.assertTrue(credentials.invalid)
        self.assertIsNone(credentials.scopes)
        scopes = credentials.retrieve_scopes(http_mock)
        self.assertEqual(scopes, SERVICE_ACCOUNT_INFO['scopes'])
        self.assertFalse(credentials.invalid)
        credentials.retrieve_scopes(http_mock)
        # Assert scopes weren't refetched
        metadata.assert_called_once_with(http_mock,
                                         service_account='default')

    @mock.patch('oauth2client.contrib._metadata.get_service_account_info',
                side_effect=http_client.HTTPException('No Such Email'))
    def test_retrieve_scopes_bad_email(self, metadata):
        http_mock = object()
        credentials = gce.AppAssertionCredentials(email='b@example.com')
        with self.assertRaises(http_client.HTTPException):
            credentials.retrieve_scopes(http_mock)

        metadata.assert_called_once_with(http_mock,
                                         service_account='b@example.com')

    def test_save_to_well_known_file(self):
        import os
        ORIGINAL_ISDIR = os.path.isdir
        try:
            os.path.isdir = lambda path: True
            credentials = gce.AppAssertionCredentials()
            with self.assertRaises(NotImplementedError):
                client.save_to_well_known_file(credentials)
        finally:
            os.path.isdir = ORIGINAL_ISDIR

    def test_custom_metadata_root_from_env(self):
        headers = {'content-type': 'application/json'}
        http = http_mock.HttpMock(headers=headers, data='{}')
        fake_metadata_root = 'another.metadata.service'
        os.environ['GCE_METADATA_ROOT'] = fake_metadata_root
        reload_module(_metadata)
        try:
            _metadata.get(http, '')
        finally:
            del os.environ['GCE_METADATA_ROOT']
            reload_module(_metadata)
        # Verify mock.
        self.assertEqual(http.requests, 1)
        expected_uri = 'http://{}/computeMetadata/v1/'.format(fake_metadata_root)
        self.assertEqual(http.uri, expected_uri)

    def test_new_custom_metadata_host_from_env(self):
        headers = {'content-type': 'application/json'}
        http = http_mock.HttpMock(headers=headers, data='{}')
        fake_metadata_root = 'another.metadata.service'
        os.environ['GCE_METADATA_HOST'] = fake_metadata_root
        reload_module(_metadata)
        try:
            _metadata.get(http, '')
        finally:
            del os.environ['GCE_METADATA_HOST']
            reload_module(_metadata)
        # Verify mock.
        self.assertEqual(http.requests, 1)
        expected_uri = 'http://{}/computeMetadata/v1/'.format(fake_metadata_root)
        self.assertEqual(http.uri, expected_uri)
