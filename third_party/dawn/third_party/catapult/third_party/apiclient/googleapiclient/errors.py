# Copyright 2014 Google Inc. All Rights Reserved.
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

"""Errors for the library.

All exceptions defined by the library
should be defined in this file.
"""
from __future__ import absolute_import

__author__ = 'jcgregorio@google.com (Joe Gregorio)'

import json

from oauth2client import util


class Error(Exception):
  """Base error for this module."""
  pass


class HttpError(Error):
  """HTTP data was invalid or unexpected."""

  @util.positional(3)
  def __init__(self, resp, content, uri=None):
    self.resp = resp
    self.content = content
    self.uri = uri

  def _get_reason(self):
    """Calculate the reason for the error from the response content."""
    reason = self.resp.reason
    try:
      data = json.loads(self.content)
      reason = data['error']['message']
    except (ValueError, KeyError):
      pass
    if reason is None:
      reason = ''
    return reason

  def __repr__(self):
    if self.uri:
      return '<HttpError %s when requesting %s returned "%s">' % (
          self.resp.status, self.uri, self._get_reason().strip())
    else:
      return '<HttpError %s "%s">' % (self.resp.status, self._get_reason())

  __str__ = __repr__


class InvalidJsonError(Error):
  """The JSON returned could not be parsed."""
  pass


class UnknownFileType(Error):
  """File type unknown or unexpected."""
  pass


class UnknownLinkType(Error):
  """Link type unknown or unexpected."""
  pass


class UnknownApiNameOrVersion(Error):
  """No API with that name and version exists."""
  pass


class UnacceptableMimeTypeError(Error):
  """That is an unacceptable mimetype for this operation."""
  pass


class MediaUploadSizeError(Error):
  """Media is larger than the method can accept."""
  pass


class ResumableUploadError(HttpError):
  """Error occured during resumable upload."""
  pass


class InvalidChunkSizeError(Error):
  """The given chunksize is not valid."""
  pass

class InvalidNotificationError(Error):
  """The channel Notification is invalid."""
  pass

class BatchError(HttpError):
  """Error occured during batch operations."""

  @util.positional(2)
  def __init__(self, reason, resp=None, content=None):
    self.resp = resp
    self.content = content
    self.reason = reason

  def __repr__(self):
      return '<BatchError %s "%s">' % (self.resp.status, self.reason)

  __str__ = __repr__


class UnexpectedMethodError(Error):
  """Exception raised by RequestMockBuilder on unexpected calls."""

  @util.positional(1)
  def __init__(self, methodId=None):
    """Constructor for an UnexpectedMethodError."""
    super(UnexpectedMethodError, self).__init__(
        'Received unexpected call %s' % methodId)


class UnexpectedBodyError(Error):
  """Exception raised by RequestMockBuilder on unexpected bodies."""

  def __init__(self, expected, provided):
    """Constructor for an UnexpectedMethodError."""
    super(UnexpectedBodyError, self).__init__(
        'Expected: [%s] - Provided: [%s]' % (expected, provided))
