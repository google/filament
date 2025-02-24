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

import unittest

import httplib2
import mock

from oauth2client import client
from oauth2client import transport
from tests import http_mock


class TestMemoryCache(unittest.TestCase):

    def test_get_set_delete(self):
        cache = transport.MemoryCache()
        self.assertIsNone(cache.get('foo'))
        self.assertIsNone(cache.delete('foo'))
        cache.set('foo', 'bar')
        self.assertEqual('bar', cache.get('foo'))
        cache.delete('foo')
        self.assertIsNone(cache.get('foo'))


class Test_get_cached_http(unittest.TestCase):

    def test_global(self):
        cached_http = transport.get_cached_http()
        self.assertIsInstance(cached_http, httplib2.Http)
        self.assertIsInstance(cached_http.cache, transport.MemoryCache)

    def test_value(self):
        cache = object()
        with mock.patch('oauth2client.transport._CACHED_HTTP', new=cache):
            result = transport.get_cached_http()
        self.assertIs(result, cache)


class Test_get_http_object(unittest.TestCase):

    @mock.patch.object(httplib2, 'Http', return_value=object())
    def test_it(self, http_klass):
        result = transport.get_http_object()
        self.assertEqual(result, http_klass.return_value)
        http_klass.assert_called_once_with()

    @mock.patch.object(httplib2, 'Http', return_value=object())
    def test_with_args(self, http_klass):
        result = transport.get_http_object(1, 2, foo='bar')
        self.assertEqual(result, http_klass.return_value)
        http_klass.assert_called_once_with(1, 2, foo='bar')


class Test__initialize_headers(unittest.TestCase):

    def test_null(self):
        result = transport._initialize_headers(None)
        self.assertEqual(result, {})

    def test_copy(self):
        headers = {'a': 1, 'b': 2}
        result = transport._initialize_headers(headers)
        self.assertEqual(result, headers)
        self.assertIsNot(result, headers)


class Test__apply_user_agent(unittest.TestCase):

    def test_null(self):
        headers = object()
        result = transport._apply_user_agent(headers, None)
        self.assertIs(result, headers)

    def test_new_agent(self):
        headers = {}
        user_agent = 'foo'
        result = transport._apply_user_agent(headers, user_agent)
        self.assertIs(result, headers)
        self.assertEqual(result, {'user-agent': user_agent})

    def test_append(self):
        orig_agent = 'bar'
        headers = {'user-agent': orig_agent}
        user_agent = 'baz'
        result = transport._apply_user_agent(headers, user_agent)
        self.assertIs(result, headers)
        final_agent = user_agent + ' ' + orig_agent
        self.assertEqual(result, {'user-agent': final_agent})


class Test_clean_headers(unittest.TestCase):

    def test_no_modify(self):
        headers = {b'key': b'val'}
        result = transport.clean_headers(headers)
        self.assertIsNot(result, headers)
        self.assertEqual(result, headers)

    def test_cast_unicode(self):
        headers = {u'key': u'val'}
        header_bytes = {b'key': b'val'}
        result = transport.clean_headers(headers)
        self.assertIsNot(result, headers)
        self.assertEqual(result, header_bytes)

    def test_unicode_failure(self):
        headers = {u'key': u'\u2603'}
        with self.assertRaises(client.NonAsciiHeaderError):
            transport.clean_headers(headers)

    def test_cast_object(self):
        headers = {b'key': True}
        header_str = {b'key': b'True'}
        result = transport.clean_headers(headers)
        self.assertIsNot(result, headers)
        self.assertEqual(result, header_str)


class Test_wrap_http_for_auth(unittest.TestCase):

    def test_wrap(self):
        credentials = object()
        http = mock.Mock()
        http.request = orig_req_method = object()
        result = transport.wrap_http_for_auth(credentials, http)
        self.assertIsNone(result)
        self.assertNotEqual(http.request, orig_req_method)
        self.assertIs(http.request.credentials, credentials)


class Test_request(unittest.TestCase):

    uri = 'http://localhost'
    method = 'POST'
    body = 'abc'
    redirections = 3

    def test_with_request_attr(self):
        mock_result = object()
        headers = {'foo': 'bar'}
        http = http_mock.HttpMock(headers=headers, data=mock_result)

        response, content = transport.request(
            http, self.uri, method=self.method, body=self.body,
            redirections=self.redirections)
        self.assertEqual(response, headers)
        self.assertIs(content, mock_result)
        # Verify mocks.
        self.assertEqual(http.requests, 1)
        self.assertEqual(http.uri, self.uri)
        self.assertEqual(http.method, self.method)
        self.assertEqual(http.body, self.body)
        self.assertIsNone(http.headers)

    def test_with_callable_http(self):
        headers = {}
        mock_result = object()
        http = http_mock.HttpMock(headers=headers, data=mock_result)

        result = transport.request(http, self.uri, method=self.method,
                                   body=self.body,
                                   redirections=self.redirections)
        self.assertEqual(result, (headers, mock_result))
        # Verify mock.
        self.assertEqual(http.requests, 1)
        self.assertEqual(http.uri, self.uri)
        self.assertEqual(http.method, self.method)
        self.assertEqual(http.body, self.body)
        self.assertIsNone(http.headers)
        self.assertEqual(http.redirections, self.redirections)
