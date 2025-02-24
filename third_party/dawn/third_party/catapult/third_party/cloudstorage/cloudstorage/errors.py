# Copyright 2012 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
# either express or implied. See the License for the specific
# language governing permissions and limitations under the License.

"""Google Cloud Storage specific Files API calls."""





from __future__ import absolute_import
__all__ = ['AuthorizationError',
           'check_status',
           'Error',
           'FatalError',
           'FileClosedError',
           'ForbiddenError',
           'InvalidRange',
           'NotFoundError',
           'ServerError',
           'TimeoutError',
           'TransientError',
          ]

import six.moves.http_client


class Error(Exception):
  """Base error for all gcs operations.

  Error can happen on GAE side or GCS server side.
  For details on a particular GCS HTTP response code, see
  https://developers.google.com/storage/docs/reference-status#standardcodes
  """


class TransientError(Error):
  """TransientError could be retried."""


class TimeoutError(TransientError):
  """HTTP 408 timeout."""


class FatalError(Error):
  """FatalError shouldn't be retried."""


class FileClosedError(FatalError):
  """File is already closed.

  This can happen when the upload has finished but 'write' is called on
  a stale upload handle.
  """


class NotFoundError(FatalError):
  """HTTP 404 resource not found."""


class ForbiddenError(FatalError):
  """HTTP 403 Forbidden.

  While GCS replies with a 403 error for many reasons, the most common one
  is due to bucket permission not correctly setup for your app to access.
  """


class AuthorizationError(FatalError):
  """HTTP 401 authentication required.

  Unauthorized request has been received by GCS.

  This error is mostly handled by GCS client. GCS client will request
  a new access token and retry the request.
  """


class InvalidRange(FatalError):
  """HTTP 416 RequestRangeNotSatifiable."""


class ServerError(TransientError):
  """HTTP >= 500 server side error."""


def check_status(status, expected, path, headers=None,
                 resp_headers=None, body=None, extras=None):
  """Check HTTP response status is expected.

  Args:
    status: HTTP response status. int.
    expected: a list of expected statuses. A list of ints.
    path: filename or a path prefix.
    headers: HTTP request headers.
    resp_headers: HTTP response headers.
    body: HTTP response body.
    extras: extra info to be logged verbatim if error occurs.

  Raises:
    AuthorizationError: if authorization failed.
    NotFoundError: if an object that's expected to exist doesn't.
    TimeoutError: if HTTP request timed out.
    ServerError: if server experienced some errors.
    FatalError: if any other unexpected errors occurred.
  """
  if status in expected:
    return

  msg = ('Expect status %r from Google Storage. But got status %d.\n'
         'Path: %r.\n'
         'Request headers: %r.\n'
         'Response headers: %r.\n'
         'Body: %r.\n'
         'Extra info: %r.\n' %
         (expected, status, path, headers, resp_headers, body, extras))

  if status == six.moves.http_client.UNAUTHORIZED:
    raise AuthorizationError(msg)
  elif status == six.moves.http_client.FORBIDDEN:
    raise ForbiddenError(msg)
  elif status == six.moves.http_client.NOT_FOUND:
    raise NotFoundError(msg)
  elif status == six.moves.http_client.REQUEST_TIMEOUT:
    raise TimeoutError(msg)
  elif status == six.moves.http_client.REQUESTED_RANGE_NOT_SATISFIABLE:
    raise InvalidRange(msg)
  elif (status == six.moves.http_client.OK and 308 in expected and
        six.moves.http_client.OK not in expected):
    raise FileClosedError(msg)
  elif status >= 500:
    raise ServerError(msg)
  else:
    raise FatalError(msg)
