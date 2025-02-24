# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from six.moves import http_client
import httplib2
import json
import logging
import socket
import six.moves.urllib.parse

from google.appengine.api import memcache
from google.appengine.api import urlfetch_errors

from dashboard.common import utils

_CACHE_DURATION = 60 * 60 * 24 * 7  # 1 week.
_VULNERABILITY_PREFIX = b")]}'\n"


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
  content = six.ensure_binary(Request(*args, **kwargs))
  if content.startswith(_VULNERABILITY_PREFIX):
    content = content[len(_VULNERABILITY_PREFIX):]
  return json.loads(content)


def Request(url,
            method='GET',
            body=None,
            use_cache=False,
            use_auth=True,
            scope=utils.EMAIL_SCOPE,
            **parameters):
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
    http_client.HTTPException: The request or response is malformed, or there is
        a network or server error, or the HTTP status code is not 2xx.
  """
  if use_cache and body:
    raise NotImplementedError('Caching not supported with request body.')

  if parameters:
    # URL-encode the parameters.
    for key, value in list(parameters.items()):
      if value is None:
        del parameters[key]
      if isinstance(value, bool):
        parameters[key] = str(value).lower()
    params = sorted(parameters.items())
    url += '?' + six.moves.urllib.parse.urlencode(params, True)

  kwargs = {'method': method}
  if body:
    # JSON-encode the body.
    kwargs['body'] = json.dumps(body)
    kwargs['headers'] = {
        'Accept': 'application/json',
        'Content-Type': 'application/json'
    }

  if use_cache:
    content = memcache.get(key=url)
    if content is not None:
      return content

  try:
    content = _RequestAndProcessHttpErrors(url, use_auth, scope, **kwargs)
  except NotFoundError:
    raise
  except (http_client.HTTPException, socket.error,
          urlfetch_errors.InternalTransientError):
    # Retry once.
    content = _RequestAndProcessHttpErrors(url, use_auth, scope, **kwargs)

  if use_cache:
    try:
      memcache.add(key=url, value=content, time=_CACHE_DURATION)
    except ValueError:
      # Max memcache size is 1000000 bytes.
      pass

  return content


def _RequestAndProcessHttpErrors(url, use_auth, scope, **kwargs):
  """Requests a URL, converting HTTP errors to Python exceptions."""
  if use_auth:
    http = utils.ServiceAccountHttp(timeout=60, scope=scope)
  else:
    http = httplib2.Http(timeout=60)
  logging.info('url: %s; use_auth: %s; kwargs: %s', url, use_auth, kwargs)

  response, content = http.request(url, **kwargs)

  if response['status'] == '404':
    logging.debug('Response headers: %s, body: %s', response, content)
    raise NotFoundError(
        'HTTP status code %s: %s' % (response['status'], repr(content[0:200])),
        response, content)
  if not response['status'].startswith('2'):
    logging.debug('Response headers: %s, body: %s', response, content)
    raise RequestError(
        'Failure in request for `%s`; HTTP status code %s: %s' %
        (url, response['status'], repr(content[0:200])), response, content)
  return content
