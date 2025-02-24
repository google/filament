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

"""Python wrappers for the Google Storage RESTful API."""





from __future__ import absolute_import
__all__ = ['ReadBuffer',
           'StreamingBuffer',
          ]

import collections
import os
import six
import six.moves.urllib.parse

from . import api_utils
from . import common
from . import errors
from . import rest_api

try:
  from google.appengine.api import urlfetch
  from google.appengine.ext import ndb
except ImportError:
  from google.appengine.api import urlfetch
  from google.appengine.ext import ndb



def _get_storage_api(retry_params, account_id=None):
  """Returns storage_api instance for API methods.

  Args:
    retry_params: An instance of api_utils.RetryParams. If none,
     thread's default will be used.
    account_id: Internal-use only.

  Returns:
    A storage_api instance to handle urlfetch work to GCS.
    On dev appserver, this instance by default will talk to a local stub
    unless common.ACCESS_TOKEN is set. That token will be used to talk
    to the real GCS.
  """


  api = _StorageApi(_StorageApi.full_control_scope,
                    service_account_id=account_id,
                    retry_params=retry_params)
  if common.local_run() and not common.get_access_token():
    api.api_url = common.local_api_url()
  if common.get_access_token():
    api.token = common.get_access_token()
  return api


class _StorageApi(rest_api._RestApi):
  """A simple wrapper for the Google Storage RESTful API.

  WARNING: Do NOT directly use this api. It's an implementation detail
  and is subject to change at any release.

  All async methods have similar args and returns.

  Args:
    path: The path to the Google Storage object or bucket, e.g.
      '/mybucket/myfile' or '/mybucket'.
    **kwd: Options for urlfetch. e.g.
      headers={'content-type': 'text/plain'}, payload='blah'.

  Returns:
    A ndb Future. When fulfilled, future.get_result() should return
    a tuple of (status, headers, content) that represents a HTTP response
    of Google Cloud Storage XML API.
  """

  api_url = 'https://storage.googleapis.com'
  read_only_scope = 'https://www.googleapis.com/auth/devstorage.read_only'
  read_write_scope = 'https://www.googleapis.com/auth/devstorage.read_write'
  full_control_scope = 'https://www.googleapis.com/auth/devstorage.full_control'

  def __getstate__(self):
    """Store state as part of serialization/pickling.

    Returns:
      A tuple (of dictionaries) with the state of this object
    """
    return (super(_StorageApi, self).__getstate__(), {'api_url': self.api_url})

  def __setstate__(self, state):
    """Restore state as part of deserialization/unpickling.

    Args:
      state: the tuple from a __getstate__ call
    """
    superstate, localstate = state
    super(_StorageApi, self).__setstate__(superstate)
    self.api_url = localstate['api_url']

  @api_utils._eager_tasklet
  @ndb.tasklet
  def do_request_async(self, url, method='GET', headers=None, payload=None,
                       deadline=None, callback=None):
    """Inherit docs.

    This method translates urlfetch exceptions to more service specific ones.
    """
    if headers is None:
      headers = {}
    if 'x-goog-api-version' not in headers:
      headers['x-goog-api-version'] = '2'
    headers['accept-encoding'] = 'gzip, *'
    try:
      resp_tuple = yield super(_StorageApi, self).do_request_async(
          url, method=method, headers=headers, payload=payload,
          deadline=deadline, callback=callback)
    except urlfetch.DownloadError as e:
      raise errors.TimeoutError(
          'Request to Google Cloud Storage timed out.', e)

    raise ndb.Return(resp_tuple)


  def post_object_async(self, path, **kwds):
    """POST to an object."""
    return self.do_request_async(self.api_url + path, 'POST', **kwds)

  def put_object_async(self, path, **kwds):
    """PUT an object."""
    return self.do_request_async(self.api_url + path, 'PUT', **kwds)

  def get_object_async(self, path, **kwds):
    """GET an object.

    Note: No payload argument is supported.
    """
    return self.do_request_async(self.api_url + path, 'GET', **kwds)

  def delete_object_async(self, path, **kwds):
    """DELETE an object.

    Note: No payload argument is supported.
    """
    return self.do_request_async(self.api_url + path, 'DELETE', **kwds)

  def head_object_async(self, path, **kwds):
    """HEAD an object.

    Depending on request headers, HEAD returns various object properties,
    e.g. Content-Length, Last-Modified, and ETag.

    Note: No payload argument is supported.
    """
    return self.do_request_async(self.api_url + path, 'HEAD', **kwds)

  def get_bucket_async(self, path, **kwds):
    """GET a bucket."""
    return self.do_request_async(self.api_url + path, 'GET', **kwds)

  def compose_object(self, file_list, destination_file, content_type):
    """COMPOSE multiple objects together.

    Using the given list of files, calls the put object with the compose flag.
    This call merges all the files into the destination file.

    Args:
      file_list: list of dicts with the file name.
      destination_file: Path to the destination file.
      content_type: Content type for the destination file.
    """

    xml_setting_list = ['<ComposeRequest>']

    for meta_data in file_list:
      xml_setting_list.append('<Component>')
      for key, val in meta_data.items():
        xml_setting_list.append('<%s>%s</%s>' % (key, val, key))
      xml_setting_list.append('</Component>')
    xml_setting_list.append('</ComposeRequest>')
    xml = ''.join(xml_setting_list)

    if content_type is not None:
      headers = {'Content-Type': content_type}
    else:
      headers = None
    status, resp_headers, content = self.put_object(
        api_utils._quote_filename(destination_file) + '?compose',
        payload=xml,
        headers=headers)
    errors.check_status(status, [200], destination_file, resp_headers,
                        body=content)


_StorageApi = rest_api.add_sync_methods(_StorageApi)


class ReadBuffer(object):
  """A class for reading Google storage files."""

  DEFAULT_BUFFER_SIZE = 1024 * 1024
  MAX_REQUEST_SIZE = 30 * DEFAULT_BUFFER_SIZE

  def __init__(self,
               api,
               path,
               buffer_size=DEFAULT_BUFFER_SIZE,
               max_request_size=MAX_REQUEST_SIZE,
               offset=0):
    """Constructor.

    Args:
      api: A StorageApi instance.
      path: Quoted/escaped path to the object, e.g. /mybucket/myfile
      buffer_size: buffer size. The ReadBuffer keeps
        one buffer. But there may be a pending future that contains
        a second buffer. This size must be less than max_request_size.
      max_request_size: Max bytes to request in one urlfetch.
      offset: Number of bytes to skip at the start of the file. If None, 0 is
        used.
    """
    self._api = api
    self._path = path
    self.name = api_utils._unquote_filename(path)
    self.closed = False

    assert buffer_size <= max_request_size
    self._buffer_size = buffer_size
    self._max_request_size = max_request_size
    self._offset = offset

    self._buffer = _Buffer()
    self._etag = None

    get_future = self._get_segment(offset, self._buffer_size, check_response=False)

    status, headers, content = self._api.head_object(path)
    errors.check_status(status, [200], path, resp_headers=headers, body=content)
    self._file_size = int(common.get_stored_content_length(headers))
    self._check_etag(headers.get('etag'))

    self._buffer_future = None

    if self._file_size != 0:
      content, check_response_closure = get_future.get_result()
      check_response_closure()
      self._buffer.reset(content)
      self._request_next_buffer()

  def __getstate__(self):
    """Store state as part of serialization/pickling.

    The contents of the read buffer are not stored, only the current offset for
    data read by the client. A new read buffer is established at unpickling.
    The head information for the object (file size and etag) are stored to
    reduce startup and ensure the file has not changed.

    Returns:
      A dictionary with the state of this object
    """
    return {'api': self._api,
            'path': self._path,
            'buffer_size': self._buffer_size,
            'request_size': self._max_request_size,
            'etag': self._etag,
            'size': self._file_size,
            'offset': self._offset,
            'closed': self.closed}

  def __setstate__(self, state):
    """Restore state as part of deserialization/unpickling.

    Args:
      state: the dictionary from a __getstate__ call

    Along with restoring the state, pre-fetch the next read buffer.
    """
    self._api = state['api']
    self._path = state['path']
    self.name = api_utils._unquote_filename(self._path)
    self._buffer_size = state['buffer_size']
    self._max_request_size = state['request_size']
    self._etag = state['etag']
    self._file_size = state['size']
    self._offset = state['offset']
    self._buffer = _Buffer()
    self.closed = state['closed']
    self._buffer_future = None
    if self._remaining() and not self.closed:
      self._request_next_buffer()

  def __iter__(self):
    """Iterator interface.

    Note the ReadBuffer container itself is the iterator. It's
    (quote PEP0234)
    'destructive: they consumes all the values and a second iterator
    cannot easily be created that iterates independently over the same values.
    You could open the file for the second time, or seek() to the beginning.'

    Returns:
      Self.
    """
    return self

  def next(self):
    line = self.readline()
    if not line:
      raise StopIteration()
    return line

  def readline(self, size=-1):
    """Read one line delimited by '\n' from the file.

    A trailing newline character is kept in the string. It may be absent when a
    file ends with an incomplete line. If the size argument is non-negative,
    it specifies the maximum string size (counting the newline) to return.
    A negative size is the same as unspecified. Empty string is returned
    only when EOF is encountered immediately.

    Args:
      size: Maximum number of bytes to read. If not specified, readline stops
        only on '\n' or EOF.

    Returns:
      The data read as a string.

    Raises:
      IOError: When this buffer is closed.
    """
    self._check_open()
    if size == 0 or not self._remaining():
      return ''

    data_list = []
    newline_offset = self._buffer.find_newline(size)
    while newline_offset < 0:
      data = self._buffer.read(size)
      size -= len(data)
      self._offset += len(data)
      data_list.append(data)
      if size == 0 or not self._remaining():
        return ''.join(data_list)
      self._buffer.reset(self._buffer_future.get_result())
      self._request_next_buffer()
      newline_offset = self._buffer.find_newline(size)

    data = self._buffer.read_to_offset(newline_offset + 1)
    self._offset += len(data)
    data_list.append(data)

    return ''.join(data_list)

  def read(self, size=-1):
    """Read data from RAW file.

    Args:
      size: Number of bytes to read as integer. Actual number of bytes
        read is always equal to size unless EOF is reached. If size is
        negative or unspecified, read the entire file.

    Returns:
      data read as str.

    Raises:
      IOError: When this buffer is closed.
    """
    self._check_open()
    if not self._remaining():
      return ''

    data_list = []
    while True:
      remaining = self._buffer.remaining()
      if size >= 0 and size < remaining:
        data_list.append(self._buffer.read(size))
        self._offset += size
        break
      else:
        size -= remaining
        self._offset += remaining
        data_list.append(self._buffer.read())

        if self._buffer_future is None:
          if size < 0 or size >= self._remaining():
            needs = self._remaining()
          else:
            needs = size
          data_list.extend(self._get_segments(self._offset, needs))
          self._offset += needs
          break

        if self._buffer_future:
          self._buffer.reset(self._buffer_future.get_result())
          self._buffer_future = None

    if self._buffer_future is None:
      self._request_next_buffer()
    return b''.join(data_list)

  def _remaining(self):
    return self._file_size - self._offset

  def _request_next_buffer(self):
    """Request next buffer.

    Requires self._offset and self._buffer are in consistent state.
    """
    self._buffer_future = None
    next_offset = self._offset + self._buffer.remaining()
    if next_offset != self._file_size:
      self._buffer_future = self._get_segment(next_offset,
                                              self._buffer_size)

  def _get_segments(self, start, request_size):
    """Get segments of the file from Google Storage as a list.

    A large request is broken into segments to avoid hitting urlfetch
    response size limit. Each segment is returned from a separate urlfetch.

    Args:
      start: start offset to request. Inclusive. Have to be within the
        range of the file.
      request_size: number of bytes to request.

    Returns:
      A list of file segments in order
    """
    if not request_size:
      return []

    end = start + request_size
    futures = []

    while request_size > self._max_request_size:
      futures.append(self._get_segment(start, self._max_request_size))
      request_size -= self._max_request_size
      start += self._max_request_size
    if start < end:
      futures.append(self._get_segment(start, end - start))
    return [fut.get_result() for fut in futures]

  @ndb.tasklet
  def _get_segment(self, start, request_size, check_response=True):
    """Get a segment of the file from Google Storage.

    Args:
      start: start offset of the segment. Inclusive. Have to be within the
        range of the file.
      request_size: number of bytes to request. Have to be small enough
        for a single urlfetch request. May go over the logical range of the
        file.
      check_response: True to check the validity of GCS response automatically
        before the future returns. False otherwise. See Yields section.

    Yields:
      If check_response is True, the segment [start, start + request_size)
      of the file.
      Otherwise, a tuple. The first element is the unverified file segment.
      The second element is a closure that checks response. Caller should
      first invoke the closure before consuing the file segment.

    Raises:
      ValueError: if the file has changed while reading.
    """
    end = start + request_size - 1
    content_range = '%d-%d' % (start, end)
    headers = {'Range': 'bytes=' + content_range}
    status, resp_headers, content = yield self._api.get_object_async(
        self._path, headers=headers)
    def _checker():
      errors.check_status(status, [200, 206], self._path, headers,
                          resp_headers, body=content)
      self._check_etag(resp_headers.get('etag'))
    if check_response:
      _checker()
      raise ndb.Return(content)
    raise ndb.Return(content, _checker)

  def _check_etag(self, etag):
    """Check if etag is the same across requests to GCS.

    If self._etag is None, set it. If etag is set, check that the new
    etag equals the old one.

    In the __init__ method, we fire one HEAD and one GET request using
    ndb tasklet. One of them would return first and set the first value.

    Args:
      etag: etag from a GCS HTTP response. None if etag is not part of the
        response header. It could be None for example in the case of GCS
        composite file.

    Raises:
      ValueError: if two etags are not equal.
    """
    if etag is None:
      return
    elif self._etag is None:
      self._etag = etag
    elif self._etag != etag:
      raise ValueError('File on GCS has changed while reading.')

  def close(self):
    self.closed = True
    self._buffer = None
    self._buffer_future = None

  def __enter__(self):
    return self

  def __exit__(self, atype, value, traceback):
    self.close()
    return False

  def seek(self, offset, whence=os.SEEK_SET):
    """Set the file's current offset.

    Note if the new offset is out of bound, it is adjusted to either 0 or EOF.

    Args:
      offset: seek offset as number.
      whence: seek mode. Supported modes are os.SEEK_SET (absolute seek),
        os.SEEK_CUR (seek relative to the current position), and os.SEEK_END
        (seek relative to the end, offset should be negative).

    Raises:
      IOError: When this buffer is closed.
      ValueError: When whence is invalid.
    """
    self._check_open()

    self._buffer.reset()
    self._buffer_future = None

    if whence == os.SEEK_SET:
      self._offset = offset
    elif whence == os.SEEK_CUR:
      self._offset += offset
    elif whence == os.SEEK_END:
      self._offset = self._file_size + offset
    else:
      raise ValueError('Whence mode %s is invalid.' % str(whence))

    self._offset = min(self._offset, self._file_size)
    self._offset = max(self._offset, 0)
    if self._remaining():
      self._request_next_buffer()

  def tell(self):
    """Tell the file's current offset.

    Returns:
      current offset in reading this file.

    Raises:
      IOError: When this buffer is closed.
    """
    self._check_open()
    return self._offset

  def _check_open(self):
    if self.closed:
      raise IOError('Buffer is closed.')

  def seekable(self):
    return True

  def readable(self):
    return True

  def writable(self):
    return False


class _Buffer(object):
  """In memory buffer."""

  def __init__(self):
    self.reset()

  def reset(self, content='', offset=0):
    self._buffer = content
    self._offset = offset

  def read(self, size=-1):
    """Returns bytes from self._buffer and update related offsets.

    Args:
      size: number of bytes to read starting from current offset.
        Read the entire buffer if negative.

    Returns:
      Requested bytes from buffer.
    """
    if size < 0:
      offset = len(self._buffer)
    else:
      offset = self._offset + size
    return self.read_to_offset(offset)

  def read_to_offset(self, offset):
    """Returns bytes from self._buffer and update related offsets.

    Args:
      offset: read from current offset to this offset, exclusive.

    Returns:
      Requested bytes from buffer.
    """
    assert offset >= self._offset
    result = self._buffer[self._offset: offset]
    self._offset += len(result)
    return result

  def remaining(self):
    return len(self._buffer) - self._offset

  def find_newline(self, size=-1):
    """Search for newline char in buffer starting from current offset.

    Args:
      size: number of bytes to search. -1 means all.

    Returns:
      offset of newline char in buffer. -1 if doesn't exist.
    """
    if size < 0:
      return self._buffer.find('\n', self._offset)
    return self._buffer.find('\n', self._offset, self._offset + size)


class StreamingBuffer(object):
  """A class for creating large objects using the 'resumable' API.

  The API is a subset of the Python writable stream API sufficient to
  support writing zip files using the zipfile module.

  The exact sequence of calls and use of headers is documented at
  https://developers.google.com/storage/docs/developer-guide#unknownresumables
  """

  _blocksize = 256 * 1024

  _flushsize = 8 * _blocksize

  _maxrequestsize = 9 * 4 * _blocksize

  def __init__(self,
               api,
               path,
               content_type=None,
               gcs_headers=None):
    """Constructor.

    Args:
      api: A StorageApi instance.
      path: Quoted/escaped path to the object, e.g. /mybucket/myfile
      content_type: Optional content-type; Default value is
        delegate to Google Cloud Storage.
      gcs_headers: additional gs headers as a str->str dict, e.g
        {'x-goog-acl': 'private', 'x-goog-meta-foo': 'foo'}.
    Raises:
      IOError: When this location can not be found.
    """
    assert self._maxrequestsize > self._blocksize
    assert self._maxrequestsize % self._blocksize == 0
    assert self._maxrequestsize >= self._flushsize

    self._api = api
    self._path = path

    self.name = api_utils._unquote_filename(path)
    self.closed = False

    self._buffer = collections.deque()
    self._buffered = 0
    self._written = 0
    self._offset = 0

    headers = {'x-goog-resumable': 'start'}
    if content_type:
      headers['content-type'] = content_type
    if gcs_headers:
      headers.update(gcs_headers)
    status, resp_headers, content = self._api.post_object(path, headers=headers)
    errors.check_status(status, [201], path, headers, resp_headers,
                        body=content)
    loc = resp_headers.get('location')
    if not loc:
      raise IOError('No location header found in 201 response')
    parsed = six.moves.urllib.parse.urlparse(loc)
    self._path_with_token = '%s?%s' % (self._path, parsed.query)

  def __getstate__(self):
    """Store state as part of serialization/pickling.

    The contents of the write buffer are stored. Writes to the underlying
    storage are required to be on block boundaries (_blocksize) except for the
    last write. In the worst case the pickled version of this object may be
    slightly larger than the blocksize.

    Returns:
      A dictionary with the state of this object

    """
    return {'api': self._api,
            'path': self._path,
            'path_token': self._path_with_token,
            'buffer': self._buffer,
            'buffered': self._buffered,
            'written': self._written,
            'offset': self._offset,
            'closed': self.closed}

  def __setstate__(self, state):
    """Restore state as part of deserialization/unpickling.

    Args:
      state: the dictionary from a __getstate__ call
    """
    self._api = state['api']
    self._path_with_token = state['path_token']
    self._buffer = state['buffer']
    self._buffered = state['buffered']
    self._written = state['written']
    self._offset = state['offset']
    self.closed = state['closed']
    self._path = state['path']
    self.name = api_utils._unquote_filename(self._path)

  def write(self, data):
    """Write some bytes.

    Args:
      data: data to write. str.

    Raises:
      TypeError: if data is not of type str.
    """
    self._check_open()
    if not isinstance(data, six.binary_type):
      raise TypeError('Expected binary but got %s.' % type(data))
    if not data:
      return
    self._buffer.append(data)
    self._buffered += len(data)
    self._offset += len(data)
    if self._buffered >= self._flushsize:
      self._flush()

  def flush(self):
    """Flush as much as possible to GCS.

    GCS *requires* that all writes except for the final one align on
    256KB boundaries. So the internal buffer may still have < 256KB bytes left
    after flush.
    """
    self._check_open()
    self._flush(finish=False)

  def tell(self):
    """Return the total number of bytes passed to write() so far.

    (There is no seek() method.)
    """
    return self._offset

  def close(self):
    """Flush the buffer and finalize the file.

    When this returns the new file is available for reading.
    """
    if not self.closed:
      self.closed = True
      self._flush(finish=True)
      self._buffer = None

  def __enter__(self):
    return self

  def __exit__(self, atype, value, traceback):
    self.close()
    return False

  def _flush(self, finish=False):
    """Internal API to flush.

    Buffer is flushed to GCS only when the total amount of buffered data is at
    least self._blocksize, or to flush the final (incomplete) block of
    the file with finish=True.
    """
    while ((finish and self._buffered >= 0) or
           (not finish and self._buffered >= self._blocksize)):
      tmp_buffer = []
      tmp_buffer_len = 0

      excess = 0
      while self._buffer:
        buf = self._buffer.popleft()
        size = len(buf)
        self._buffered -= size
        tmp_buffer.append(buf)
        tmp_buffer_len += size
        if tmp_buffer_len >= self._maxrequestsize:
          excess = tmp_buffer_len - self._maxrequestsize
          break
        if not finish and (
            tmp_buffer_len % self._blocksize + self._buffered <
            self._blocksize):
          excess = tmp_buffer_len % self._blocksize
          break

      if excess:
        over = tmp_buffer.pop()
        size = len(over)
        assert size >= excess
        tmp_buffer_len -= size
        head, tail = over[:-excess], over[-excess:]
        self._buffer.appendleft(tail)
        self._buffered += len(tail)
        if head:
          tmp_buffer.append(head)
          tmp_buffer_len += len(head)

      data = b''.join(tmp_buffer)
      file_len = '*'
      if finish and not self._buffered:
        file_len = self._written + len(data)
      self._send_data(data, self._written, file_len)
      self._written += len(data)
      if file_len != '*':
        break

  def _send_data(self, data, start_offset, file_len):
    """Send the block to the storage service.

    This is a utility method that does not modify self.

    Args:
      data: data to send in str.
      start_offset: start offset of the data in relation to the file.
      file_len: an int if this is the last data to append to the file.
        Otherwise '*'.
    """
    headers = {}
    end_offset = start_offset + len(data) - 1

    if data:
      headers['content-range'] = ('bytes %d-%d/%s' %
                                  (start_offset, end_offset, file_len))
    else:
      headers['content-range'] = ('bytes */%s' % file_len)

    status, response_headers, content = self._api.put_object(
        self._path_with_token, payload=data, headers=headers)
    if file_len == '*':
      expected = 308
    else:
      expected = 200
    errors.check_status(status, [expected], self._path, headers,
                        response_headers, content,
                        {'upload_path': self._path_with_token})

  def _get_offset_from_gcs(self):
    """Get the last offset that has been written to GCS.

    This is a utility method that does not modify self.

    Returns:
      an int of the last offset written to GCS by this upload, inclusive.
      -1 means nothing has been written.
    """
    headers = {'content-range': 'bytes */*'}
    status, response_headers, content = self._api.put_object(
        self._path_with_token, headers=headers)
    errors.check_status(status, [308], self._path, headers,
                        response_headers, content,
                        {'upload_path': self._path_with_token})
    val = response_headers.get('range')
    if val is None:
      return -1
    _, offset = val.rsplit('-', 1)
    return int(offset)

  def _force_close(self, file_length=None):
    """Close this buffer on file_length.

    Finalize this upload immediately on file_length.
    Contents that are still in memory will not be uploaded.

    This is a utility method that does not modify self.

    Args:
      file_length: file length. Must match what has been uploaded. If None,
        it will be queried from GCS.
    """
    if file_length is None:
      file_length = self._get_offset_from_gcs() + 1
    self._send_data('', 0, file_length)

  def _check_open(self):
    if self.closed:
      raise IOError('Buffer is closed.')

  def seekable(self):
    return False

  def readable(self):
    return False

  def writable(self):
    return True
