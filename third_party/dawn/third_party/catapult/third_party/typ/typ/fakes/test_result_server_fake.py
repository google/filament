# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""A fake implementation of test-results.appspot.com."""

import io
import sys
import threading


if sys.version_info.major == 2:  # pragma: python2
    from SimpleHTTPServer import SimpleHTTPRequestHandler as HTTPRequestHandler
    from SocketServer import TCPServer
else:  # pragma: python3
    assert sys.version_info.major == 3
    # pylint: disable=invalid-name, redefined-builtin
    unicode = str
    from http.server import BaseHTTPRequestHandler  # pylint: disable=F0401
    HTTPRequestHandler = BaseHTTPRequestHandler
    from socketserver import TCPServer  # pylint: disable=F0401


def start(code=200):
    server = _Server(code=code)
    thread = threading.Thread(target=_run, args=(server,))
    server.main_thread = thread
    thread.daemon = True
    thread.start()
    return server


def _run(server):
    server.serve_forever(0.05)


class _Server(TCPServer):

    def __init__(self, code):
        self.allow_reuse_address = True
        TCPServer.__init__(self, ('localhost', 0), _RequestHandler)
        self.log = io.StringIO()
        self.requests = []
        self.main_thread = None
        self.code = code

    def stop(self):
        self.shutdown()
        self.main_thread.join()
        return self.requests


class _RequestHandler(HTTPRequestHandler):

    def __init__(self, *args, **kwargs):
        HTTPRequestHandler.__init__(self, *args, **kwargs)

    # 'Invalid Name' pylint: disable=C0103
    def do_POST(self):
        path = self.path
        length = int(self.headers['content-length'])
        payload = self.rfile.read(length)
        self.server.requests.append(('post', path, payload))
        self.send_response(self.server.code)
        self.end_headers()

    # 'Redefining built-in' pylint: disable=W0622
    def log_message(self, format, *args):
        self.server.log.write(unicode('%s\n' % (format % args)))
