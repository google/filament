# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from six.moves import http_client
import socket
import unittest

from unittest import mock

from google.appengine.api import urlfetch_errors
from google.appengine.ext import testbed

from dashboard.common import utils
from dashboard.services import request


class _RequestTest(unittest.TestCase):

  def setUp(self):
    self.testbed = testbed.Testbed()
    self.testbed.activate()
    self.testbed.init_memcache_stub()

    http = mock.MagicMock()
    self._request = http.request

    patcher = mock.patch('dashboard.common.utils.ServiceAccountHttp')
    self._service_account_http = patcher.start()
    self._service_account_http.return_value = http
    self.addCleanup(patcher.stop)


class SuccessTest(_RequestTest):

  def testRequest(self):
    self._request.return_value = ({'status': '200'}, 'response')
    response = request.Request('https://example.com')
    self._service_account_http.assert_called_once_with(
        scope=utils.EMAIL_SCOPE, timeout=60)
    self._request.assert_called_once_with('https://example.com', method='GET')
    self.assertEqual(response, 'response')

  def testRequestJson(self):
    self._request.return_value = ({'status': '200'}, '"response"')
    response = request.RequestJson('https://example.com')
    self._request.assert_called_once_with('https://example.com', method='GET')
    self.assertEqual(response, 'response')

  def testRequestJsonWithPrefix(self):
    self._request.return_value = ({'status': '200'}, ')]}\'\n"response"')
    response = request.RequestJson('https://example.com')
    self._request.assert_called_once_with('https://example.com', method='GET')
    self.assertEqual(response, 'response')

  def testRequestWithBodyAndParameters(self):
    self._request.return_value = ({'status': '200'}, 'response')
    response = request.Request(
        'https://example.com',
        'POST',
        body='a string',
        url_param_1='value_1',
        url_param_2='value_2')
    self._request.assert_called_once_with(
        'https://example.com?url_param_1=value_1&url_param_2=value_2',
        method='POST',
        body='"a string"',
        headers={
            'Content-Type': 'application/json',
            'Accept': 'application/json'
        })
    self.assertEqual(response, 'response')


class FailureAndRetryTest(_RequestTest):

  def _TestRetry(self):
    response = request.Request('https://example.com')

    self._request.assert_called_with('https://example.com', method='GET')
    self.assertEqual(self._request.call_count, 2)
    self.assertEqual(response, 'response')

  def testHttpErrorCode(self):
    self._request.return_value = ({'status': '500'}, '')
    with self.assertRaises(http_client.HTTPException):
      request.Request('https://example.com')
    self._request.assert_called_with('https://example.com', method='GET')
    self.assertEqual(self._request.call_count, 2)

  def testHttpException(self):
    self._request.side_effect = http_client.HTTPException
    with self.assertRaises(http_client.HTTPException):
      request.Request('https://example.com')
    self._request.assert_called_with('https://example.com', method='GET')
    self.assertEqual(self._request.call_count, 2)

  def testSocketError(self):
    self._request.side_effect = socket.error
    with self.assertRaises(socket.error):
      request.Request('https://example.com')
    self._request.assert_called_with('https://example.com', method='GET')
    self.assertEqual(self._request.call_count, 2)

  def testInternalTransientError(self):
    self._request.side_effect = urlfetch_errors.InternalTransientError
    with self.assertRaises(urlfetch_errors.InternalTransientError):
      request.Request('https://example.com')
    self._request.assert_called_with('https://example.com', method='GET')
    self.assertEqual(self._request.call_count, 2)

  def testNotFound(self):
    self._request.return_value = ({'status': '404'}, '')
    with self.assertRaises(request.NotFoundError):
      request.Request('https://example.com')
    self._request.assert_called_with('https://example.com', method='GET')
    self.assertEqual(self._request.call_count, 1)

  def testHttpNotAuthorized(self):
    self._request.return_value = ({'status': '403'}, b'\x00\xe2')
    with self.assertRaises(request.RequestError):
      request.Request('https://example.com')

  def testHttpErrorCodeSuccessOnRetry(self):
    failure_return_value = ({'status': '500'}, '')
    success_return_value = ({'status': '200'}, 'response')
    self._request.side_effect = failure_return_value, success_return_value
    self._TestRetry()

  def testHttpExceptionSuccessOnRetry(self):
    return_value = ({'status': '200'}, 'response')
    self._request.side_effect = http_client.HTTPException, return_value
    self._TestRetry()

  def testSocketErrorSuccessOnRetry(self):
    return_value = ({'status': '200'}, 'response')
    self._request.side_effect = socket.error, return_value
    self._TestRetry()


class CacheTest(_RequestTest):

  def testSetAndGet(self):
    self._request.return_value = ({'status': '200'}, 'response')

    response = request.Request('https://example.com', use_cache=True)
    self.assertEqual(response, 'response')
    self.assertEqual(self._request.call_count, 1)

    response = request.Request('https://example.com', use_cache=True)
    self.assertEqual(response, 'response')
    self.assertEqual(self._request.call_count, 1)

  def testRequestBody(self):
    self._request.return_value = ({'status': '200'}, 'response')
    with self.assertRaises(NotImplementedError):
      request.Request('https://example.com', body='body', use_cache=True)

  @mock.patch.object(request.memcache, 'add',
                     mock.MagicMock(side_effect=ValueError))
  def testResponseTooLarge(self):
    self._request.return_value = ({'status': '200'}, 'response')

    response = request.Request('https://example.com', use_cache=True)
    self.assertEqual(response, 'response')
    self.assertEqual(self._request.call_count, 1)

    response = request.Request('https://example.com', use_cache=True)
    self.assertEqual(response, 'response')
    self.assertEqual(self._request.call_count, 2)


class AuthTest(_RequestTest):

  def testNoAuth(self):
    http = mock.MagicMock()

    patcher = mock.patch('httplib2.Http')
    httplib2_http = patcher.start()
    httplib2_http.return_value = http
    self.addCleanup(patcher.stop)

    http.request.return_value = ({'status': '200'}, 'response')
    response = request.Request('https://example.com', use_auth=False)

    httplib2_http.assert_called_once_with(timeout=60)
    http.request.assert_called_once_with('https://example.com', method='GET')
    self.assertEqual(self._request.call_count, 0)
    self.assertEqual(response, 'response')
