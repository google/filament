# Copyright 2016 Google Inc. All rights reserved.
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

import datetime
import json
import unittest

import mock
from six.moves import http_client

from oauth2client.contrib import _metadata
from tests import http_mock


PATH = 'instance/service-accounts/default'
DATA = {'foo': 'bar'}
EXPECTED_URL = (
    'http://metadata.google.internal/computeMetadata/v1/instance'
    '/service-accounts/default')


def request_mock(status, content_type, content):
    headers = {'status': status, 'content-type': content_type}
    http = http_mock.HttpMock(headers=headers,
                              data=content.encode('utf-8'))
    return http


class TestMetadata(unittest.TestCase):

    def test_get_success_json(self):
        http = request_mock(
            http_client.OK, 'application/json', json.dumps(DATA))
        self.assertEqual(
            _metadata.get(http, PATH),
            DATA
        )

        # Verify mocks.
        self.assertEqual(http.requests, 1)
        self.assertEqual(http.uri, EXPECTED_URL)
        self.assertEqual(http.method, 'GET')
        self.assertIsNone(http.body)
        self.assertEqual(http.headers, _metadata.METADATA_HEADERS)

    def test_get_success_string(self):
        http = request_mock(
            http_client.OK, 'text/html', '<p>Hello World!</p>')
        self.assertEqual(
            _metadata.get(http, PATH),
            '<p>Hello World!</p>'
        )

        # Verify mocks.
        self.assertEqual(http.requests, 1)
        self.assertEqual(http.uri, EXPECTED_URL)
        self.assertEqual(http.method, 'GET')
        self.assertIsNone(http.body)
        self.assertEqual(http.headers, _metadata.METADATA_HEADERS)

    def test_get_failure(self):
        http = request_mock(
            http_client.NOT_FOUND, 'text/html', '<p>Error</p>')
        with self.assertRaises(http_client.HTTPException):
            _metadata.get(http, PATH)

        # Verify mocks.
        self.assertEqual(http.requests, 1)
        self.assertEqual(http.uri, EXPECTED_URL)
        self.assertEqual(http.method, 'GET')
        self.assertIsNone(http.body)
        self.assertEqual(http.headers, _metadata.METADATA_HEADERS)

    @mock.patch(
        'oauth2client.client._UTCNOW',
        return_value=datetime.datetime.min)
    def test_get_token_success(self, now):
        http = request_mock(
            http_client.OK,
            'application/json',
            json.dumps({'access_token': 'a', 'expires_in': 100})
        )
        token, expiry = _metadata.get_token(http=http)
        self.assertEqual(token, 'a')
        self.assertEqual(
            expiry, datetime.datetime.min + datetime.timedelta(seconds=100))
        # Verify mocks.
        now.assert_called_once_with()
        self.assertEqual(http.requests, 1)
        self.assertEqual(http.uri, EXPECTED_URL + '/token')
        self.assertEqual(http.method, 'GET')
        self.assertIsNone(http.body)
        self.assertEqual(http.headers, _metadata.METADATA_HEADERS)

    def test_service_account_info(self):
        http = request_mock(
            http_client.OK, 'application/json', json.dumps(DATA))
        info = _metadata.get_service_account_info(http)
        self.assertEqual(info, DATA)
        # Verify mock.
        self.assertEqual(http.requests, 1)
        self.assertEqual(http.uri, EXPECTED_URL + '/?recursive=True')
        self.assertEqual(http.method, 'GET')
        self.assertIsNone(http.body)
        self.assertEqual(http.headers, _metadata.METADATA_HEADERS)
