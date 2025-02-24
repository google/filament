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

"""Helpers shared by cloudstorage_stub and cloudstorage_api."""





from __future__ import absolute_import
import six
__all__ = ['CS_XML_NS',
           'CSFileStat',
           'dt_str_to_posix',
           'local_api_url',
           'LOCAL_GCS_ENDPOINT',
           'local_run',
           'get_access_token',
           'get_stored_content_length',
           'get_metadata',
           'GCSFileStat',
           'http_time_to_posix',
           'memory_usage',
           'posix_time_to_http',
           'posix_to_dt_str',
           'set_access_token',
           'validate_options',
           'validate_bucket_name',
           'validate_bucket_path',
           'validate_file_path',
          ]


import calendar
import datetime
from email import utils as email_utils
import logging
import os
import re

try:
  from google.appengine.api import runtime
except ImportError:
  from google.appengine.api import runtime


_GCS_BUCKET_REGEX_BASE = r'[a-z0-9\.\-_]{3,63}'
_GCS_BUCKET_REGEX = re.compile(_GCS_BUCKET_REGEX_BASE + r'$')
_GCS_BUCKET_PATH_REGEX = re.compile(r'/' + _GCS_BUCKET_REGEX_BASE + r'$')
_GCS_PATH_PREFIX_REGEX = re.compile(r'/' + _GCS_BUCKET_REGEX_BASE + r'.*')
_GCS_FULLPATH_REGEX = re.compile(r'/' + _GCS_BUCKET_REGEX_BASE + r'/.*')
_GCS_METADATA = ['x-goog-meta-',
                 'content-disposition',
                 'cache-control',
                 'content-encoding']
_GCS_OPTIONS = _GCS_METADATA + ['x-goog-acl']
CS_XML_NS = 'http://doc.s3.amazonaws.com/2006-03-01'
LOCAL_GCS_ENDPOINT = '/_ah/gcs'
_access_token = ''


_MAX_GET_BUCKET_RESULT = 1000


def set_access_token(access_token):
  """Set the shared access token to authenticate with Google Cloud Storage.

  When set, the library will always attempt to communicate with the
  real Google Cloud Storage with this token even when running on dev appserver.
  Note the token could expire so it's up to you to renew it.

  When absent, the library will automatically request and refresh a token
  on appserver, or when on dev appserver, talk to a Google Cloud Storage
  stub.

  Args:
    access_token: you can get one by run 'gsutil -d ls' and copy the
      str after 'Bearer'.
  """
  global _access_token
  _access_token = access_token


def get_access_token():
  """Returns the shared access token."""
  return _access_token


class GCSFileStat(object):
  """Container for GCS file stat."""

  def __init__(self,
               filename,
               st_size,
               etag,
               st_ctime,
               content_type=None,
               metadata=None,
               is_dir=False):
    """Initialize.

    For files, the non optional arguments are always set.
    For directories, only filename and is_dir is set.

    Args:
      filename: a Google Cloud Storage filename of form '/bucket/filename'.
      st_size: file size in bytes. long compatible.
      etag: hex digest of the md5 hash of the file's content. str.
      st_ctime: posix file creation time. float compatible.
      content_type: content type. str.
      metadata: a str->str dict of user specified options when creating
        the file. Possible keys are x-goog-meta-, content-disposition,
        content-encoding, and cache-control.
      is_dir: True if this represents a directory. False if this is a real file.
    """
    self.filename = filename
    self.is_dir = is_dir
    self.st_size = None
    self.st_ctime = None
    self.etag = None
    self.content_type = content_type
    self.metadata = metadata

    if not is_dir:
      self.st_size = int(st_size)
      self.st_ctime = float(st_ctime)
      if etag[0] == '"' and etag[-1] == '"':
        etag = etag[1:-1]
      self.etag = etag

  def __repr__(self):
    if self.is_dir:
      return '(directory: %s)' % self.filename

    return (
        '(filename: %(filename)s, st_size: %(st_size)s, '
        'st_ctime: %(st_ctime)s, etag: %(etag)s, '
        'content_type: %(content_type)s, '
        'metadata: %(metadata)s)' %
        dict(filename=self.filename,
             st_size=self.st_size,
             st_ctime=self.st_ctime,
             etag=self.etag,
             content_type=self.content_type,
             metadata=self.metadata))

  def __cmp__(self, other):
    if not isinstance(other, self.__class__):
      raise ValueError('Argument to cmp must have the same type. '
                       'Expect %s, got %s', self.__class__.__name__,
                       other.__class__.__name__)
    if self.filename > other.filename:
      return 1
    elif self.filename < other.filename:
      return -1
    return 0

  def __hash__(self):
    if self.etag:
      return hash(self.etag)
    return hash(self.filename)


CSFileStat = GCSFileStat


def get_stored_content_length(headers):
  """Return the content length (in bytes) of the object as stored in GCS.

  x-goog-stored-content-length should always be present except when called via
  the local dev_appserver. Therefore if it is not present we default to the
  standard content-length header.

  Args:
    headers: a dict of headers from the http response.

  Returns:
    the stored content length.
  """
  length = headers.get('x-goog-stored-content-length')
  if length is None:
    length = headers.get('content-length')
  return length


def get_metadata(headers):
  """Get user defined options from HTTP response headers."""
  return dict((k, v) for k, v in headers.items()
              if any(k.lower().startswith(valid) for valid in _GCS_METADATA))


def validate_bucket_name(name):
  """Validate a Google Storage bucket name.

  Args:
    name: a Google Storage bucket name with no prefix or suffix.

  Raises:
    ValueError: if name is invalid.
  """
  _validate_path(name)
  if not _GCS_BUCKET_REGEX.match(name):
    raise ValueError('Bucket should be 3-63 characters long using only a-z,'
                     '0-9, underscore, dash or dot but got %s' % name)


def validate_bucket_path(path):
  """Validate a Google Cloud Storage bucket path.

  Args:
    path: a Google Storage bucket path. It should have form '/bucket'.

  Raises:
    ValueError: if path is invalid.
  """
  _validate_path(path)
  if not _GCS_BUCKET_PATH_REGEX.match(path):
    raise ValueError('Bucket should have format /bucket '
                     'but got %s' % path)


def validate_file_path(path):
  """Validate a Google Cloud Storage file path.

  Args:
    path: a Google Storage file path. It should have form '/bucket/filename'.

  Raises:
    ValueError: if path is invalid.
  """
  _validate_path(path)
  if not _GCS_FULLPATH_REGEX.match(path):
    raise ValueError('Path should have format /bucket/filename '
                     'but got %s' % path)


def _process_path_prefix(path_prefix):
  """Validate and process a Google Cloud Stoarge path prefix.

  Args:
    path_prefix: a Google Cloud Storage path prefix of format '/bucket/prefix'
      or '/bucket/' or '/bucket'.

  Raises:
    ValueError: if path is invalid.

  Returns:
    a tuple of /bucket and prefix. prefix can be None.
  """
  _validate_path(path_prefix)
  if not _GCS_PATH_PREFIX_REGEX.match(path_prefix):
    raise ValueError('Path prefix should have format /bucket, /bucket/, '
                     'or /bucket/prefix but got %s.' % path_prefix)
  bucket_name_end = path_prefix.find('/', 1)
  bucket = path_prefix
  prefix = None
  if bucket_name_end != -1:
    bucket = path_prefix[:bucket_name_end]
    prefix = path_prefix[bucket_name_end + 1:] or None
  return bucket, prefix


def _validate_path(path):
  """Basic validation of Google Storage paths.

  Args:
    path: a Google Storage path. It should have form '/bucket/filename'
      or '/bucket'.

  Raises:
    ValueError: if path is invalid.
    TypeError: if path is not of type basestring.
  """
  if not path:
    raise ValueError('Path is empty')
  if not isinstance(path, six.string_types):
    raise TypeError('Path should be a string but is %s (%s).' %
                    (path.__class__, path))


def validate_options(options):
  """Validate Google Cloud Storage options.

  Args:
    options: a str->basestring dict of options to pass to Google Cloud Storage.

  Raises:
    ValueError: if option is not supported.
    TypeError: if option is not of type str or value of an option
      is not of type basestring.
  """
  if not options:
    return

  for k, v in options.items():
    if not isinstance(k, str):
      raise TypeError('option %r should be a str.' % k)
    if not any(k.lower().startswith(valid) for valid in _GCS_OPTIONS):
      raise ValueError('option %s is not supported.' % k)
    if not isinstance(v, six.string_types):
      raise TypeError('value %r for option %s should be of type basestring.' %
                      (v, k))


def http_time_to_posix(http_time):
  """Convert HTTP time format to posix time.

  See http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.1
  for http time format.

  Args:
    http_time: time in RFC 2616 format. e.g.
      "Mon, 20 Nov 1995 19:12:08 GMT".

  Returns:
    A float of secs from unix epoch.
  """
  if http_time is not None:
    return email_utils.mktime_tz(email_utils.parsedate_tz(http_time))


def posix_time_to_http(posix_time):
  """Convert posix time to HTML header time format.

  Args:
    posix_time: unix time.

  Returns:
    A datatime str in RFC 2616 format.
  """
  if posix_time:
    return email_utils.formatdate(posix_time, usegmt=True)


_DT_FORMAT = '%Y-%m-%dT%H:%M:%S'


def dt_str_to_posix(dt_str):
  """format str to posix.

  datetime str is of format %Y-%m-%dT%H:%M:%S.%fZ,
  e.g. 2013-04-12T00:22:27.978Z. According to ISO 8601, T is a separator
  between date and time when they are on the same line.
  Z indicates UTC (zero meridian).

  A pointer: http://www.cl.cam.ac.uk/~mgk25/iso-time.html

  This is used to parse LastModified node from GCS's GET bucket XML response.

  Args:
    dt_str: A datetime str.

  Returns:
    A float of secs from unix epoch. By posix definition, epoch is midnight
    1970/1/1 UTC.
  """
  parsable, _ = dt_str.split('.')
  dt = datetime.datetime.strptime(parsable, _DT_FORMAT)
  return calendar.timegm(dt.utctimetuple())


def posix_to_dt_str(posix):
  """Reverse of str_to_datetime.

  This is used by GCS stub to generate GET bucket XML response.

  Args:
    posix: A float of secs from unix epoch.

  Returns:
    A datetime str.
  """
  dt = datetime.datetime.utcfromtimestamp(posix)
  dt_str = dt.strftime(_DT_FORMAT)
  return dt_str + '.000Z'


def local_run():
  """Whether we should hit GCS dev appserver stub."""
  server_software = os.environ.get('SERVER_SOFTWARE')
  if server_software is None:
    return True
  if 'remote_api' in server_software:
    return False
  if server_software.startswith(('Development', 'testutil')):
    return True
  return False


def local_api_url():
  """Return URL for GCS emulation on dev appserver."""
  return 'http://%s%s' % (os.environ.get('HTTP_HOST'), LOCAL_GCS_ENDPOINT)


def memory_usage(method):
  """Log memory usage before and after a method."""
  def wrapper(*args, **kwargs):
    logging.info('Memory before method %s is %s.',
                 method.__name__, runtime.memory_usage().current())
    result = method(*args, **kwargs)
    logging.info('Memory after method %s is %s',
                 method.__name__, runtime.memory_usage().current())
    return result
  return wrapper


def _add_ns(tagname):
  return '{%(ns)s}%(tag)s' % {'ns': CS_XML_NS,
                              'tag': tagname}


_T_CONTENTS = _add_ns('Contents')
_T_LAST_MODIFIED = _add_ns('LastModified')
_T_ETAG = _add_ns('ETag')
_T_KEY = _add_ns('Key')
_T_SIZE = _add_ns('Size')
_T_PREFIX = _add_ns('Prefix')
_T_COMMON_PREFIXES = _add_ns('CommonPrefixes')
_T_NEXT_MARKER = _add_ns('NextMarker')
_T_IS_TRUNCATED = _add_ns('IsTruncated')
