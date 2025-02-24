# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import google_auth_httplib2
import google.auth
import http.client as http_client
import json
import socket

from google.appengine.api import urlfetch_errors
from urllib.parse import urlencode

EMAIL_SCOPE = 'https://www.googleapis.com/auth/userinfo.email'


class RequestError(http_client.HTTPException):

  def __init__(self, msg, headers, content):
    super().__init__(msg)
    self.headers = headers
    self.content = content


class NotFoundError(RequestError):
  """Raised when a request gives a HTTP 404 error."""


def RequestJson(*args, **kwargs):
  """Fetch a URL and JSON-decode the response.

  See the documentation for Request() for details
  about the arguments and exceptions.
  """
  content = Request(*args, **kwargs)
  return json.loads(content)


def ServiceAccountHttp(scope=EMAIL_SCOPE, timeout=None):

  credentials, _ = google.auth.default()
  credentials = credentials.with_scopes([scope])
  http = google_auth_httplib2.AuthorizedHttp(credentials)
  if timeout:
    http.timeout = timeout
  return http


def Request(url, method='GET', body=None, **parameters):
  """Fetch a URL while authenticated as the service account.

  Args:
    method: The HTTP request method. E.g. 'GET', 'POST', 'PUT'.
    body: The request body as a Python object. It will be JSON-encoded.
    use_cache: If True, use memcache to cache the response.
    parameters: Parameters to be encoded in the URL query string.

  Returns:
    The reponse body.

  Raises:
    NotFoundError: The HTTP status code is 404.
    http.client.HTTPException: The request or response is malformed, or there is
        a network or server error, or the HTTP status code is not 2xx.
  """

  if parameters:
    # URL-encode the parameters.
    for key, value in list(parameters.items()):
      if value is None:
        del parameters[key]
      if isinstance(value, bool):
        parameters[key] = str(value).lower()
    params = sorted(parameters.items())
    url += '?' + urlencode(params, True)

  kwargs = {'method': method}
  if body:
    # JSON-encode the body.
    kwargs['body'] = json.dumps(body)
    kwargs['headers'] = {
        'Accept': 'application/json',
        'Content-Type': 'application/json'
    }

  try:
    content = _RequestAndProcessHttpErrors(url, **kwargs)
  except NotFoundError:
    raise
  except (http_client.HTTPException, socket.error,
          urlfetch_errors.InternalTransientError):
    # Retry once.
    content = _RequestAndProcessHttpErrors(url, **kwargs)

  return content


def _RequestAndProcessHttpErrors(url, **kwargs):
  """Requests a URL, converting HTTP errors to Python exceptions."""
  http = ServiceAccountHttp(timeout=60)

  response, content = http.request(url, **kwargs)

  if response['status'] == '404':
    raise NotFoundError(
        'HTTP status code %s: %s' % (response['status'], repr(content[0:200])),
        response, content)
  if not response['status'].startswith('2'):
    raise RequestError(
        'Failure in request for `%s`; HTTP status code %s: %s' %
        (url, response['status'], repr(content[0:200])), response, content)
  return content
