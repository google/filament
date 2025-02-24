# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import absolute_import
from collections import namedtuple
import errno
import gzip
import logging
import mimetypes
import os
import socket
import sys
import traceback
from io import BytesIO

import six.moves.socketserver # pylint: disable=import-error
import six.moves.urllib.parse # pylint: disable=import-error
import six.moves.BaseHTTPServer # pylint: disable=import-error
import six.moves.SimpleHTTPServer # pylint: disable=import-error
from six.moves import map # pylint: disable=redefined-builtin

from telemetry.core import local_server

ByteRange = namedtuple('ByteRange', ['from_byte', 'to_byte'])
ResourceAndRange = namedtuple('ResourceAndRange', ['resource', 'byte_range'])

_MIME_TYPES_FILE = os.path.abspath(
    os.path.join(os.path.dirname(__file__), 'mime.types'))


class MemoryCacheHTTPRequestHandler(
    six.moves.SimpleHTTPServer.SimpleHTTPRequestHandler):

  protocol_version = 'HTTP/1.1'  # override BaseHTTPServer setting
  wbufsize = -1  # override StreamRequestHandler (a base class) setting

  def handle(self):
    try:
      six.moves.BaseHTTPServer.BaseHTTPRequestHandler.handle(self)
    except socket.error as e:
      # Connection reset errors happen all the time due to the browser closing
      # without terminating the connection properly.  They can be safely
      # ignored.
      if e.errno != errno.ECONNRESET:
        raise

  def do_GET(self):
    """Serve a GET request."""
    resource_range = self.SendHead()

    if not resource_range or not resource_range.resource:
      return
    response = resource_range.resource['response']
    if not isinstance(response, bytes):
      response = response.encode('utf-8')

    if not resource_range.byte_range:
      self.wfile.write(response)
      return

    start_index = resource_range.byte_range.from_byte
    end_index = resource_range.byte_range.to_byte
    self.wfile.write(response[start_index:end_index + 1])

  def do_HEAD(self):
    """Serve a HEAD request."""
    self.SendHead()

  def log_error(self, fmt, *args):
    pass

  def log_request(self, code='-', size='-'):
    # Don't spam the console unless it is important.
    pass

  def Response(self, path):
    """Get the response for the path."""
    if path not in self.server.resource_map:
      return None

    return self.server.resource_map[path]

  def SendHead(self):
    path = os.path.realpath(self.translate_path(self.path))
    resource = self.Response(path)
    if not resource:
      self.send_error(404, 'File not found')
      return None

    total_num_of_bytes = resource['content-length']
    byte_range = self.GetByteRange(total_num_of_bytes)
    if byte_range:
      # request specified a range, so set response code to 206.
      self.send_response(206)
      self.send_header('Content-Range', 'bytes %d-%d/%d' %
                       (byte_range.from_byte, byte_range.to_byte,
                        total_num_of_bytes))
      total_num_of_bytes = byte_range.to_byte - byte_range.from_byte + 1
    else:
      self.send_response(200)

    self.send_header('Content-Length', str(total_num_of_bytes))
    self.send_header('Content-Type', resource['content-type'])
    self.send_header('Last-Modified',
                     self.date_time_string(resource['last-modified']))
    if resource['zipped']:
      self.send_header('Content-Encoding', 'gzip')
    self.end_headers()
    return ResourceAndRange(resource, byte_range)

  def GetByteRange(self, total_num_of_bytes):
    """Parse the header and get the range values specified.

    Args:
      total_num_of_bytes: Total # of bytes in requested resource,
      used to calculate upper range limit.
    Returns:
      A ByteRange namedtuple object with the requested byte-range values.
      If no Range is explicitly requested or there is a failure parsing,
      return None.
      If range specified is in the format "N-", return N-END. Refer to
      http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html for details.
      If upper range limit is greater than total # of bytes, return upper index.
    """
    range_header = self.headers.get('Range')
    if range_header is None:
      return None
    if not range_header.startswith('bytes='):
      return None

    # The range header is expected to be a string in this format:
    # bytes=0-1
    # Get the upper and lower limits of the specified byte-range.
    # We've already confirmed that range_header starts with 'bytes='.
    byte_range_values = range_header[len('bytes='):].split('-')
    from_byte = 0
    to_byte = 0

    if len(byte_range_values) == 2:
      # If to_range is not defined return all bytes starting from from_byte.
      to_byte = (int(byte_range_values[1]) if byte_range_values[1] else
                 total_num_of_bytes - 1)
      # If from_range is not defined return last 'to_byte' bytes.
      from_byte = (int(byte_range_values[0]) if byte_range_values[0] else
                   total_num_of_bytes - to_byte)
    else:
      return None

    # Do some validation.
    if from_byte < 0:
      return None

    # Make to_byte the end byte by default in edge cases.
    if to_byte < from_byte or to_byte >= total_num_of_bytes:
      to_byte = total_num_of_bytes - 1

    return ByteRange(from_byte, to_byte)


class _MemoryCacheHTTPServerImpl(six.moves.socketserver.ThreadingMixIn,
                                 six.moves.BaseHTTPServer.HTTPServer):
  # Increase the request queue size. The default value, 5, is set in
  # SocketServer.TCPServer (the parent of BaseHTTPServer.HTTPServer).
  # Since we're intercepting many domains through this single server,
  # it is quite possible to get more than 5 concurrent requests.
  request_queue_size = 128

  # Don't prevent python from exiting when there is thread activity.
  daemon_threads = True

  def __init__(self, host_port, handler, paths):
    six.moves.BaseHTTPServer.HTTPServer.__init__(self, host_port, handler)
    self.resource_map = {}
    # Use Telemetry's 'mime.types' file instead of relying on system files to
    # ensure the mime type inference is deterministic
    # (also see crbug.com/894868).
    assert os.path.isfile(_MIME_TYPES_FILE)
    mimetypes.init([_MIME_TYPES_FILE])
    for path in paths:
      if os.path.isdir(path):
        self.AddDirectoryToResourceMap(path)
      else:
        self.AddFileToResourceMap(path)

  def AddDirectoryToResourceMap(self, directory_path):
    """Loads all files in directory_path into the in-memory resource map."""
    for root, dirs, files in os.walk(directory_path):
      # Skip hidden files and folders (like .svn and .git).
      files = [f for f in files if f[0] != '.']
      dirs[:] = [d for d in dirs if d[0] != '.']

      for f in files:
        file_path = os.path.join(root, f)
        if not os.path.exists(file_path):  # Allow for '.#' files
          continue
        self.AddFileToResourceMap(file_path)

  def AddFileToResourceMap(self, file_path):
    """Loads file_path into the in-memory resource map."""
    file_path = os.path.realpath(file_path)
    if file_path in self.resource_map:
      return

    with open(file_path, 'rb') as fd:
      response = fd.read()
      fs = os.fstat(fd.fileno())
    content_type = mimetypes.guess_type(file_path)[0]
    zipped = False
    if content_type in ['text/html', 'text/css', 'application/javascript']:
      zipped = True
      bio = BytesIO()
      gzf = gzip.GzipFile(fileobj=bio, compresslevel=9, mode='wb')
      gzf.write(response)
      gzf.close()
      response = bio.getvalue()
      bio.close()
    self.resource_map[file_path] = {
        'content-type': content_type,
        'content-length': len(response),
        'last-modified': fs.st_mtime,
        'response': response,
        'zipped': zipped
    }

    index = 'index.html'
    if os.path.basename(file_path) == index:
      dir_path = os.path.dirname(file_path)
      self.resource_map[dir_path] = self.resource_map[file_path]

  def handle_error(self, request, client_address):
    """Handle error in a thread-safe way

    We override handle_error method of our base TCPServer class. It does the
    same but uses thread-safe logging.error instead of print, because
    SocketServer.ThreadingMixIn runs network operations on multiple threads and
    there's a race condition on stdout.
    """
    del request  # unused
    logging.error('Exception happened during processing of request from '
                  '%s\n%s%s', client_address, traceback.format_exc(), '-'*80)


class MemoryCacheHTTPServerBackend(local_server.LocalServerBackend):

  def __init__(self):
    super().__init__()
    self._httpd = None

  def StartAndGetNamedPorts(self, args, handler_class=None):
    if handler_class:
      assert issubclass(handler_class, MemoryCacheHTTPRequestHandler)

    base_dir = args['base_dir']
    os.chdir(base_dir)

    paths = args['paths']
    for path in paths:
      if not os.path.realpath(path).startswith(os.path.realpath(os.getcwd())):
        print('"%s" is not under the cwd.' % path, file=sys.stderr)
        sys.exit(1)

    server_address = (args['host'], args['port'])
    MemoryCacheHTTPRequestHandler.protocol_version = 'HTTP/1.1'
    self._httpd = _MemoryCacheHTTPServerImpl(
        server_address,
        handler_class if handler_class else MemoryCacheHTTPRequestHandler,
        paths)
    return [local_server.NamedPort('http', self._httpd.server_address[1])]

  def ServeForever(self):
    return self._httpd.serve_forever()


class MemoryCacheHTTPServer(local_server.LocalServer):

  def __init__(self, paths):
    super().__init__(MemoryCacheHTTPServerBackend)
    self._base_dir = None

    for path in paths:
      assert os.path.exists(path), '%s does not exist.' % path

    paths = list(paths)
    self._paths = paths

    self._paths_as_set = set(map(os.path.realpath, paths))

    common_prefix = os.path.commonprefix(paths)
    if os.path.isdir(common_prefix):
      self._base_dir = common_prefix
    else:
      self._base_dir = os.path.dirname(common_prefix)

  def GetBackendStartupArgs(self):
    return {'base_dir': self._base_dir,
            'paths': self._paths,
            'host': self.host_ip,
            'port': 0}

  @property
  def paths(self):
    return self._paths_as_set

  @property
  def url(self):
    return 'http://127.0.0.1:%s' % self.port

  @property
  def localhost_url(self):
    return 'http://localhost:%s' % self.port

  def UrlOf(self, path):
    if os.path.isabs(path):
      relative_path = os.path.relpath(path, self._base_dir)
    else:
      relative_path = path
    # Preserve trailing slash or backslash.
    # It doesn't matter in a file path, but it does matter in a URL.
    if path.endswith(os.sep) or (os.altsep and path.endswith(os.altsep)):
      relative_path += '/'
    return six.moves.urllib.parse.urljoin(
        self.url, relative_path.replace(os.sep, '/'))

  def LocalhostUrlOf(self, path):
    if os.path.isabs(path):
      relative_path = os.path.relpath(path, self._base_dir)
    else:
      relative_path = path
    # Preserve trailing slash or backslash.
    # It doesn't matter in a file path, but it does matter in a URL.
    if path.endswith(os.sep) or (os.altsep and path.endswith(os.altsep)):
      relative_path += '/'
    return six.moves.urllib.parse.urljoin(
        self.localhost_url, relative_path.replace(os.sep, '/'))


class MemoryCacheDynamicHTTPRequestHandler(MemoryCacheHTTPRequestHandler):
  """This class extends MemoryCacheHTTPRequestHandler by adding support for
  dynamic responses. Inherite this class and register the sub-class to the
  story set (through StorySet.SetRequestHandlerClass() or the constructor).
  """

  def ResponseFromHandler(self, path):
    """Override this method to return dynamic response."""
    del path  # Unused.

  def Response(self, path):
    """Returns the dynamic response if exists, otherwise, use the resource
    map.
    """
    # pylint: disable=assignment-from-no-return
    response = self.ResponseFromHandler(path)
    # pylint: enable=assignment-from-no-return
    if response:
      return response

    if path not in self.server.resource_map:
      return None

    return self.server.resource_map[path]

  def MakeResponse(self, content, content_type, zipped):
    """Helper method to create a response object.
    """
    return {
        'content-type': content_type,
        'content-length': len(content),
        'last-modified': None,
        'response': content,
        'zipped': zipped
    }


class MemoryCacheDynamicHTTPServer(MemoryCacheHTTPServer):
  """This class extends MemoryCacheHTTPServer by adding support for returning
  dynamic responses.
  """

  def __init__(self, paths, dynamic_request_handler_class):
    # dynamic_request_handler_class must be a sub-class of
    # MemoryCacheDynamicHTTPRequestHandler
    assert issubclass(dynamic_request_handler_class,
                      MemoryCacheDynamicHTTPRequestHandler)
    super().__init__(paths)
    self._dynamic_request_handler_class = dynamic_request_handler_class

  @property
  def dynamic_request_handler_class(self):
    return self._dynamic_request_handler_class

  def GetBackendStartupArgs(self):
    args = super().GetBackendStartupArgs()
    args['dynamic_request_handler_module_name'] = \
        self._dynamic_request_handler_class.__module__
    args['dynamic_request_handler_class_name'] = \
        self._dynamic_request_handler_class.__name__
    return args
